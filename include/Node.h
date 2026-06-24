#ifndef NODE_H
#define NODE_H

#include <memory>

struct Node {
    char ch;
    uint64_t frequency;
    uint64_t id; // stable order fallback
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    // Leaf constructor
    Node(char c, uint64_t freq, uint64_t nodeId = 0)
        : ch(c), frequency(freq), id(nodeId), left(nullptr), right(nullptr) {}

    // Internal node constructor
    Node(char c, uint64_t freq, std::shared_ptr<Node> l, std::shared_ptr<Node> r, uint64_t nodeId = 0)
        : ch(c), frequency(freq), id(nodeId), left(l), right(r) {}

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

// Heap comparator
struct CompareNode {
    bool operator()(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) const {
        if (lhs->frequency == rhs->frequency) {
            return lhs->id > rhs->id;
        }
        return lhs->frequency > rhs->frequency;
    }
};

#endif // NODE_H
