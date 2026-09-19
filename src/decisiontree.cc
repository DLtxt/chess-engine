#include "moves/move.h"
#include "decisiontree.h"

std::shared_ptr<DecisionTree> DecisionTree::AddDecision(std::unique_ptr<Move> decision) {
	auto new_decision = std::make_shared<DecisionTree>(std::move(decision), std::make_shared<DecisionTree>(*this));
	// auto new_decision = std::make_shared<DecisionTree>();
	// new_decision->decision = std::move(decision);
	// new_decision->last_decision = std::make_shared<DecisionTree>(*this);
	next_decisions.push_back(new_decision);
	return new_decision;
}

std::shared_ptr<DecisionTree> DecisionTree::UndoDecision(Board* board) {
	decision->Undo(board);
	return last_decision; 
}