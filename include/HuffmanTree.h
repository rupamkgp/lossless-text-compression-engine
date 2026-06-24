#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H

#include "Node.h"
#include <unordered_map>
#include <string>
#include <memory>

class HuffmanTree {
public:
    HuffmanTree() = default;
    ~HuffmanTree() = default;

    void build(const std::unordered_map<char, uint64_t>& freqMap);
    std::unordered_map<char, std::string> generateCodes() const;

    std::shared_ptr<Node> getRoot() const { return root; }
    void setRoot(std::shared_ptr<Node> r) { root = r; }

    std::string serializeTreeJSON() const;

private:
    std::shared_ptr<Node> root = nullptr;

    void generateCodesHelper(std::shared_ptr<Node> node, const std::string& code,
                             std::unordered_map<char, std::string>& codes) const;

    void serializeTreeJSONHelper(std::shared_ptr<Node> node, std::ostream& ss) const;
};

#endif // HUFFMANTREE_H
