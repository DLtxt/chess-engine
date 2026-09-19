#include "position.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>

namespace eng {

// Values used by static exchange evaluation. These are deliberately plain
// material numbers -- SEE is about who wins a capture sequence, not about
// positional compensation.
const int kSeeValue[kPieceTypeNb] = { 100, 320, 330, 500, 900, 20000 };

namespace {

const char* kPieceChars = "PNBRQKpnbrqk";

// Board squares that matter for castling rights.
enum { kA1 = 0, kC1 = 2, kD1 = 3, kE1 = 4, kF1 = 5, kG1 = 6, kH1 = 7,
       kA8 = 56, kC8 = 58, kD8 = 59, kE8 = 60, kF8 = 61, kG8 = 62, kH8 = 63 };

// Moving from or to one of these squares removes the corresponding rights.
int castling_mask_table[64];
bool castling_mask_ready = false;

void init_castling_mask() {
	for (int sq = 0; sq < 64; ++sq) castling_mask_table[sq] = kAnyCastling;
	castling_mask_table[kE1] = kAnyCastling & ~(kWhiteOO | kWhiteOOO);
	castling_mask_table[kH1] = kAnyCastling & ~kWhiteOO;
	castling_mask_table[kA1] = kAnyCastling & ~kWhiteOOO;
	castling_mask_table[kE8] = kAnyCastling & ~(kBlackOO | kBlackOOO);
	castling_mask_table[kH8] = kAnyCastling & ~kBlackOO;
	castling_mask_table[kA8] = kAnyCastling & ~kBlackOOO;
	castling_mask_ready = true;
}

} // namespace

std::string square_to_string(int sq) {
	if (sq == kNoSquare) return "-";
	std::string s;
	s += char('a' + file_of(sq));
	s += char('1' + rank_of(sq));
	return s;
}

int square_from_string(const std::string& s) {
	if (s.size() < 2 || s[0] < 'a' || s[0] > 'h' || s[1] < '1' || s[1] > '8')
		return kNoSquare;
	return make_square(s[0] - 'a', s[1] - '1');
}

std::string move_to_uci(Move m) {
	if (m == kMoveNone) return "0000";
	std::string s = square_to_string(from_sq(m)) + square_to_string(to_sq(m));
	if (move_type(m) == kPromotion) s += "nbrq"[promotion_type(m) - kKnight];
	return s;
}

// ------------------------------------------------------- piece placement ---

void Position::put_piece(Piece pc, int sq) {
	board_[sq] = pc;
	const Bitboard b = square_bb(sq);
	by_type_[type_of(pc)] |= b;
	by_colour_[colour_of(pc)] |= b;
	key_ ^= zobrist::psq[pc][sq];
	if (type_of(pc) == kPawn) pawn_key_ ^= zobrist::psq[pc][sq];
}

void Position::remove_piece(int sq) {
	const Piece pc = board_[sq];
	const Bitboard b = square_bb(sq);
	by_type_[type_of(pc)] ^= b;
	by_colour_[colour_of(pc)] ^= b;
	board_[sq] = kNoPiece;
	key_ ^= zobrist::psq[pc][sq];
	if (type_of(pc) == kPawn) pawn_key_ ^= zobrist::psq[pc][sq];
}

void Position::move_piece(int from, int to) {
	const Piece pc = board_[from];
	const Bitboard b = square_bb(from) | square_bb(to);
	by_type_[type_of(pc)] ^= b;
	by_colour_[colour_of(pc)] ^= b;
	board_[from] = kNoPiece;
	board_[to] = pc;
	key_ ^= zobrist::psq[pc][from] ^ zobrist::psq[pc][to];
	if (type_of(pc) == kPawn)
		pawn_key_ ^= zobrist::psq[pc][from] ^ zobrist::psq[pc][to];
}

void Position::clear() {
	for (int i = 0; i < kPieceTypeNb; ++i) by_type_[i] = 0;
	by_colour_[kWhite] = by_colour_[kBlack] = 0;
	for (int i = 0; i < 64; ++i) board_[i] = kNoPiece;
	stm_ = kWhite;
	castling_ = kNoCastling;
	ep_ = kNoSquare;
	halfmove_ = 0;
	ply_ = 0;
	key_ = 0;
	pawn_key_ = 0;
	checkers_ = 0;
	st_.clear();
	st_.reserve(512);
}

void Position::set_check_info() {
	checkers_ = attackers_to(king_sq(stm_), pieces()) & by_colour_[~stm_];
}

void Position::set_start() {
	set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

// ------------------------------------------------------------------ FEN ----

bool Position::set_fen(const std::string& fen) {
	if (!castling_mask_ready) init_castling_mask();
	clear();

	std::istringstream ss(fen);
	std::string placement, side, castle, ep;
	int halfmove = 0, fullmove = 1;

	if (!(ss >> placement >> side)) return false;
	if (!(ss >> castle)) castle = "-";
	if (!(ss >> ep)) ep = "-";
	ss >> halfmove;
	ss >> fullmove;

	int file = 0, rank = 7;
	for (char ch : placement) {
		if (ch == '/') { --rank; file = 0; continue; }
		if (std::isdigit(static_cast<unsigned char>(ch))) { file += ch - '0'; continue; }
		const char* p = std::strchr(kPieceChars, ch);
		if (!p || file > 7 || rank < 0) return false;
		const int idx = int(p - kPieceChars);
		put_piece(make_piece(idx < 6 ? kWhite : kBlack, PieceType(idx % 6)),
		          make_square(file, rank));
		++file;
	}

	stm_ = (side == "b") ? kBlack : kWhite;
	if (stm_ == kBlack) key_ ^= zobrist::side;

	for (char ch : castle) {
		switch (ch) {
			case 'K': castling_ |= kWhiteOO;  break;
			case 'Q': castling_ |= kWhiteOOO; break;
			case 'k': castling_ |= kBlackOO;  break;
			case 'q': castling_ |= kBlackOOO; break;
			default: break;
		}
	}
	key_ ^= zobrist::castling[castling_];

	// Only record an en-passant square when a capture is genuinely available,
	// so that two otherwise identical positions hash identically.
	const int eps = (ep == "-") ? kNoSquare : square_from_string(ep);
	if (eps != kNoSquare && (kPawnAttacks[~stm_][eps] & pieces(stm_, kPawn))) {
		ep_ = eps;
		key_ ^= zobrist::ep_file[file_of(eps)];
	}

	halfmove_ = halfmove;
	ply_ = (fullmove - 1) * 2 + (stm_ == kBlack ? 1 : 0);

	// A position with no king on either side is not usable by the search.
	if (!pieces(kWhite, kKing) || !pieces(kBlack, kKing)) return false;

	set_check_info();
	return true;
}

std::string Position::fen() const {
	std::ostringstream ss;
	for (int rank = 7; rank >= 0; --rank) {
		int gap = 0;
		for (int file = 0; file < 8; ++file) {
			const Piece pc = board_[make_square(file, rank)];
			if (pc == kNoPiece) { ++gap; continue; }
			if (gap) { ss << gap; gap = 0; }
			ss << kPieceChars[int(colour_of(pc)) * 6 + int(type_of(pc))];
		}
		if (gap) ss << gap;
		if (rank) ss << '/';
	}
	ss << (stm_ == kWhite ? " w " : " b ");
	if (!castling_) ss << '-';
	else {
		if (castling_ & kWhiteOO)  ss << 'K';
		if (castling_ & kWhiteOOO) ss << 'Q';
		if (castling_ & kBlackOO)  ss << 'k';
		if (castling_ & kBlackOOO) ss << 'q';
	}
	ss << ' ' << square_to_string(ep_) << ' ' << halfmove_ << ' ' << (ply_ / 2 + 1);
	return ss.str();
}

// -------------------------------------------------------------- attacks ----

Bitboard Position::attackers_to(int sq, Bitboard occ) const {
	// A black pawn attacks sq from the squares a white pawn on sq would attack,
	// and vice versa -- pawn attacks reverse when you swap the colour.
	return (kPawnAttacks[kWhite][sq] & pieces(kBlack, kPawn))
	     | (kPawnAttacks[kBlack][sq] & pieces(kWhite, kPawn))
	     | (kKnightAttacks[sq] & by_type_[kKnight])
	     | (kKingAttacks[sq] & by_type_[kKing])
	     | (bishop_attacks(sq, occ) & (by_type_[kBishop] | by_type_[kQueen]))
	     | (rook_attacks(sq, occ) & (by_type_[kRook] | by_type_[kQueen]));
}

bool Position::legal(Move m) const {
	const Colour us = stm_, them = ~us;
	const int from = from_sq(m), to = to_sq(m);
	const MoveType mt = move_type(m);

	// Castling legality (empty path, no transit through check) is fully
	// established by the generator, which is the only thing that emits it.
	if (mt == kCastling) return true;

	// Apply just the occupancy change, then ask whether our king is attacked.
	// This is exact without needing a full make/unmake.
	Bitboard occ = pieces();
	Bitboard them_bb = by_colour_[them];

	const int capsq = (mt == kEnPassant) ? to - pawn_push(us) : to;
	if (board_[capsq] != kNoPiece && colour_of(board_[capsq]) == them) {
		occ ^= square_bb(capsq);
		them_bb ^= square_bb(capsq);
	}
	occ ^= square_bb(from);
	occ |= square_bb(to);

	const int ksq = (type_of(board_[from]) == kKing) ? to : king_sq(us);

	// attackers_to() reads the unmodified piece bitboards, so mask the result
	// with them_bb to drop a piece this move just captured.
	return !(attackers_to(ksq, occ) & them_bb);
}

bool Position::pseudo_legal(Move m) const {
	if (m == kMoveNone) return false;

	const Colour us = stm_, them = ~us;
	const int from = from_sq(m), to = to_sq(m);
	const Piece pc = board_[from];

	if (pc == kNoPiece || colour_of(pc) != us) return false;
	if (by_colour_[us] & square_bb(to)) return false;

	const PieceType pt = type_of(pc);
	const MoveType mt = move_type(m);

	if (mt == kCastling) {
		if (pt != kKing) return false;
		if (us == kWhite && from == kE1 && to == kG1) return can_castle(kWhite, true);
		if (us == kWhite && from == kE1 && to == kC1) return can_castle(kWhite, false);
		if (us == kBlack && from == kE8 && to == kG8) return can_castle(kBlack, true);
		if (us == kBlack && from == kE8 && to == kC8) return can_castle(kBlack, false);
		return false;
	}

	if (mt == kEnPassant) {
		return pt == kPawn && ep_ != kNoSquare && to == ep_
		    && (kPawnAttacks[us][from] & square_bb(to));
	}

	if (mt == kPromotion && (pt != kPawn || relative_rank(us, to) != 7)) return false;
	if (mt != kPromotion && pt == kPawn && relative_rank(us, to) == 7) return false;

	if (pt == kPawn) {
		const int push = pawn_push(us);
		if (kPawnAttacks[us][from] & square_bb(to))
			return board_[to] != kNoPiece && colour_of(board_[to]) == them;
		if (to == from + push) return board_[to] == kNoPiece;
		if (to == from + 2 * push && relative_rank(us, from) == 1)
			return board_[to] == kNoPiece && board_[from + push] == kNoPiece;
		return false;
	}

	return (attacks_from(pt, from, pieces()) & square_bb(to)) != 0;
}

bool Position::can_castle(Colour c, bool kingside) const {
	const int right = c == kWhite ? (kingside ? kWhiteOO : kWhiteOOO)
	                              : (kingside ? kBlackOO : kBlackOOO);
	if (!(castling_ & right)) return false;
	if (checkers_) return false;

	const int ksq = c == kWhite ? kE1 : kE8;
	const int rsq = c == kWhite ? (kingside ? kH1 : kA1) : (kingside ? kH8 : kA8);

	// The rights bits should already guarantee this, but a hand-edited FEN can
	// claim rights with no rook present.
	if (board_[ksq] != make_piece(c, kKing)) return false;
	if (board_[rsq] != make_piece(c, kRook)) return false;

	// Every square between king and rook must be empty.
	if (kBetween[ksq][rsq] & pieces()) return false;

	// The king may not start, cross, or land on an attacked square.
	const int step = kingside ? 1 : -1;
	for (int i = 1; i <= 2; ++i)
		if (attackers_to(ksq + step * i, pieces()) & by_colour_[~c]) return false;

	return true;
}

// --------------------------------------------------------- making moves ----

void Position::do_move(Move m) {
	const Colour us = stm_, them = ~us;
	const int from = from_sq(m), to = to_sq(m);
	const MoveType mt = move_type(m);
	const Piece moving = board_[from];
	const PieceType mpt = type_of(moving);

	StateInfo st;
	st.key = key_;
	st.pawn_key = pawn_key_;
	st.castling = castling_;
	st.ep = ep_;
	st.halfmove = halfmove_;
	st.checkers = checkers_;
	st.captured = kNoPiece;

	++ply_;
	++halfmove_;

	// Retire the previous en-passant square. It is hashed whenever it is set,
	// so this needs no condition.
	if (ep_ != kNoSquare) {
		key_ ^= zobrist::ep_file[file_of(ep_)];
		ep_ = kNoSquare;
	}

	if (mt == kEnPassant) {
		const int capsq = to - pawn_push(us);
		st.captured = board_[capsq];
		remove_piece(capsq);
		halfmove_ = 0;
	} else if (board_[to] != kNoPiece) {
		st.captured = board_[to];
		remove_piece(to);
		halfmove_ = 0;
	}

	st_.push_back(st);

	move_piece(from, to);

	if (mt == kPromotion) {
		remove_piece(to);
		put_piece(make_piece(us, promotion_type(m)), to);
	} else if (mt == kCastling) {
		int rfrom, rto;
		if (to == kG1)      { rfrom = kH1; rto = kF1; }
		else if (to == kC1) { rfrom = kA1; rto = kD1; }
		else if (to == kG8) { rfrom = kH8; rto = kF8; }
		else                { rfrom = kA8; rto = kD8; }
		move_piece(rfrom, rto);
	}

	if (mpt == kPawn) {
		halfmove_ = 0;
		// A double push only creates an en-passant square if it can be used.
		if (to - from == 2 * pawn_push(us)) {
			const int eps = from + pawn_push(us);
			if (kPawnAttacks[us][eps] & pieces(them, kPawn)) {
				ep_ = eps;
				key_ ^= zobrist::ep_file[file_of(eps)];
			}
		}
	}

	const int new_castling = castling_ & castling_mask_table[from] & castling_mask_table[to];
	if (new_castling != castling_) {
		key_ ^= zobrist::castling[castling_] ^ zobrist::castling[new_castling];
		castling_ = new_castling;
	}

	stm_ = them;
	key_ ^= zobrist::side;

	set_check_info();
}

void Position::undo_move(Move m) {
	const int from = from_sq(m), to = to_sq(m);
	const MoveType mt = move_type(m);

	stm_ = ~stm_;
	const Colour us = stm_;

	if (mt == kPromotion) {
		remove_piece(to);
		put_piece(make_piece(us, kPawn), to);
	} else if (mt == kCastling) {
		int rfrom, rto;
		if (to == kG1)      { rfrom = kH1; rto = kF1; }
		else if (to == kC1) { rfrom = kA1; rto = kD1; }
		else if (to == kG8) { rfrom = kH8; rto = kF8; }
		else                { rfrom = kA8; rto = kD8; }
		move_piece(rto, rfrom);
	}

	move_piece(to, from);

	const StateInfo& st = st_.back();
	if (st.captured != kNoPiece) {
		const int capsq = (mt == kEnPassant) ? to - pawn_push(us) : to;
		put_piece(st.captured, capsq);
	}

	// Restoring these wholesale means do_move never has to unwind its own
	// hashing, which is where incremental Zobrist code usually goes wrong.
	key_ = st.key;
	pawn_key_ = st.pawn_key;
	castling_ = st.castling;
	ep_ = st.ep;
	halfmove_ = st.halfmove;
	checkers_ = st.checkers;

	st_.pop_back();
	--ply_;
}

void Position::do_null_move() {
	StateInfo st;
	st.key = key_;
	st.pawn_key = pawn_key_;
	st.castling = castling_;
	st.ep = ep_;
	st.halfmove = halfmove_;
	st.checkers = checkers_;
	st.captured = kNoPiece;
	st_.push_back(st);

	if (ep_ != kNoSquare) {
		key_ ^= zobrist::ep_file[file_of(ep_)];
		ep_ = kNoSquare;
	}
	stm_ = ~stm_;
	key_ ^= zobrist::side;
	++halfmove_;
	++ply_;
	set_check_info();
}

void Position::undo_null_move() {
	const StateInfo& st = st_.back();
	stm_ = ~stm_;
	key_ = st.key;
	pawn_key_ = st.pawn_key;
	castling_ = st.castling;
	ep_ = st.ep;
	halfmove_ = st.halfmove;
	checkers_ = st.checkers;
	st_.pop_back();
	--ply_;
}

// ----------------------------------------------------------- draw rules ----

bool Position::is_repetition() const {
	// Only positions with the same side to move can repeat, so step back two
	// plies at a time. Nothing before the last irreversible move can match.
	const int limit = std::min(halfmove_, int(st_.size()));
	for (int i = 2; i <= limit; i += 2)
		if (st_[st_.size() - i].key == key_) return true;
	return false;
}

bool Position::is_insufficient_material() const {
	if (by_type_[kPawn] | by_type_[kRook] | by_type_[kQueen]) return false;

	// With no pawns, rooks or queens left, neither side can force mate unless
	// somebody holds two or more minor pieces.
	const int white_minors = popcount(pieces(kWhite, kKnight, kBishop));
	const int black_minors = popcount(pieces(kBlack, kKnight, kBishop));
	return white_minors <= 1 && black_minors <= 1;
}

int Position::non_pawn_material(Colour c) const {
	return kSeeValue[kKnight] * count(c, kKnight)
	     + kSeeValue[kBishop] * count(c, kBishop)
	     + kSeeValue[kRook]   * count(c, kRook)
	     + kSeeValue[kQueen]  * count(c, kQueen);
}

// ----------------------------------------- static exchange evaluation -----

int Position::see(Move m) const {
	if (move_type(m) == kCastling) return 0;

	const int from = from_sq(m), to = to_sq(m);
	Colour side = stm_;

	int gain[32];
	int depth = 0;

	Bitboard occ = pieces();
	Bitboard from_bb = square_bb(from);

	if (move_type(m) == kEnPassant) {
		gain[0] = kSeeValue[kPawn];
		occ ^= square_bb(to - pawn_push(side));
	} else {
		const Piece cap = board_[to];
		gain[0] = (cap == kNoPiece) ? 0 : kSeeValue[type_of(cap)];
	}

	PieceType moving = type_of(board_[from]);
	if (move_type(m) == kPromotion) {
		// The pawn leaves and a new piece arrives on the square being contested.
		gain[0] += kSeeValue[promotion_type(m)] - kSeeValue[kPawn];
		moving = promotion_type(m);
	}

	while (true) {
		++depth;
		gain[depth] = kSeeValue[moving] - gain[depth - 1];

		// Prune once the side to move cannot improve on standing pat.
		if (std::max(-gain[depth - 1], gain[depth]) < 0) break;

		occ ^= from_bb;
		side = ~side;

		// Recomputing against the shrinking occupancy reveals x-ray attackers
		// behind the piece that just left.
		const Bitboard attackers = attackers_to(to, occ) & occ;
		const Bitboard mine = attackers & by_colour_[side];
		if (!mine) break;

		// Always recapture with the least valuable attacker available.
		from_bb = 0;
		for (int pt = kPawn; pt <= kKing; ++pt) {
			const Bitboard b = mine & by_type_[pt];
			if (b) { from_bb = b & (~b + 1); moving = PieceType(pt); break; }
		}
		if (!from_bb) break;

		if (depth >= 30) break;
	}

	// Unwind the sequence: at each step the side to move takes the capture only
	// if it beats declining.
	while (--depth > 0)
		gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);

	return gain[0];
}

std::string Position::to_string() const {
	std::ostringstream ss;
	for (int rank = 7; rank >= 0; --rank) {
		ss << (rank + 1) << ' ';
		for (int file = 0; file < 8; ++file) {
			const Piece pc = board_[make_square(file, rank)];
			ss << (pc == kNoPiece ? '.' : kPieceChars[int(colour_of(pc)) * 6 + int(type_of(pc))]) << ' ';
		}
		ss << '\n';
	}
	ss << "  a b c d e f g h\n" << "fen: " << fen() << '\n';
	return ss.str();
}

} // namespace eng
