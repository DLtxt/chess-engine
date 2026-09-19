#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include <memory>
#include <vector>

class Board;
class Move;

class DecisionTree {
	std::shared_ptr<Move> decision;
	std::shared_ptr<DecisionTree> last_decision;
	std::vector<std::shared_ptr<DecisionTree>> next_decisions;

	public:
		DecisionTree() : last_decision{nullptr} {}
		DecisionTree(std::shared_ptr<Move> decision, std::shared_ptr<DecisionTree> last) : decision{std::move(decision)}, last_decision{last} {}
		std::shared_ptr<DecisionTree> AddDecision(std::unique_ptr<Move> decision);
		std::shared_ptr<DecisionTree> UndoDecision(Board*);
		std::shared_ptr<Move>& GetDecision() { return decision; }

		DecisionTree(const DecisionTree& other) {
			decision = other.decision;
			last_decision = other.last_decision;
			next_decisions = other.next_decisions;
		}
};

#endif 