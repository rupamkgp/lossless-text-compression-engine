#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H

#include "Node.h"
#include <unordered_map>
#include <string>
#include <memory>



// Class managing Huffman Tree construction and prefix code generation.


class HuffmanTree {
public:
    HuffmanTree() = default;
    ~HuffmanTree() = default;

    

    // freqMap Map of characters to their frequency count
    void build(const std::unordered_map<char, uint64_t>& freqMap);

    /**
     * @brief Generate prefix codes for all symbols by traversing the tree.
     * Complexity: O(n) where n is the number of leaf nodes.
     * @return std::unordered_map<char, std::string> Map of characters to their binary string representation.
     */


    std::unordered_map<char, std::string> generateCodes() const;

 
     //Get the root node of the Huffman Tree.
    std::shared_ptr<Node> getRoot() const { return root; }


     // Set the root node directly.
    void setRoot(std::shared_ptr<Node> r) { root = r; }

    
    // Serializes the tree structure into a JSON string.

    std::string serializeTreeJSON() const;

private:
    std::shared_ptr<Node> root = nullptr;

    
    // Recursive DFS helper to generate Huffman codes.

    void generateCodesHelper(std::shared_ptr<Node> node, const std::string& code,
                             std::unordered_map<char, std::string>& codes) const;

    
    // Recursive helper to serialize nodes to JSON.
    
    void serializeTreeJSONHelper(std::shared_ptr<Node> node, std::ostream& ss) const;
};

#endif // HUFFMANTREE_H
