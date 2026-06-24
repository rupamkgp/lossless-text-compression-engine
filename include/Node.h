#ifndef NODE_H
#define NODE_H

#include <memory>



//

struct Node {
    char ch;                    // Character value (valid for leaf nodes)
    uint64_t frequency;         // Frequency of character occurrence
    uint64_t id;                // Unique incremental ID to enforce stable tree construction
    std::shared_ptr<Node> left;  // Pointer to the left child
    std::shared_ptr<Node> right; // Pointer to the right child

    
     // Construct a new Leaf Node.
     
    Node(char c, uint64_t freq, uint64_t nodeId = 0)
        : ch(c), frequency(freq), id(nodeId), left(nullptr), right(nullptr) {}

    //Construct a new Internal Node.
    
    Node(char c, uint64_t freq, std::shared_ptr<Node> l, std::shared_ptr<Node> r, uint64_t nodeId = 0)
        : ch(c), frequency(freq), id(nodeId), left(l), right(r) {}


    // Checks if the current node is a leaf node.
     
    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};



 // Comparator struct for the min-heap priority queue.

struct CompareNode {
    bool operator()(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) const {
        if (lhs->frequency == rhs->frequency) {
            return lhs->id > rhs->id; // Enforce stable ordering
        }
        return lhs->frequency > rhs->frequency;
    }
};

#endif // NODE_H
