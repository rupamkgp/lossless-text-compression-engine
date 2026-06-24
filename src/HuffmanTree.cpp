#include "HuffmanTree.h"
#include <queue>
#include <vector>
#include <map>
#include <sstream>

void HuffmanTree::build(const std::unordered_map<char, uint64_t>& freqMap) {
    if (freqMap.empty()) {
        root = nullptr;
        return;
    }

    // Sort character frequencies to ensure deterministic heap layout
    std::map<char, uint64_t> sortedFreqMap(freqMap.begin(), freqMap.end());
    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, CompareNode> pq;
    uint64_t idCounter = 0;

    for (const auto& pair : sortedFreqMap) {
        pq.push(std::make_shared<Node>(pair.first, pair.second, idCounter++));
    }

    // Single character edge case (branch requires two nodes)
    if (pq.size() == 1) {
        auto singleNode = pq.top();
        pq.pop();
        auto dummyNode = std::make_shared<Node>('\0', 0, idCounter++);
        auto parent = std::make_shared<Node>('\0', singleNode->frequency, singleNode, dummyNode, idCounter++);
        root = parent;
        return;
    }

    // Build the tree
    while (pq.size() > 1) {
        auto left = pq.top();
        pq.pop();
        auto right = pq.top();
        pq.pop();

        auto parent = std::make_shared<Node>('\0', left->frequency + right->frequency, left, right, idCounter++);
        pq.push(parent);
    }

    root = pq.top();
}

std::unordered_map<char, std::string> HuffmanTree::generateCodes() const {
    std::unordered_map<char, std::string> codes;
    if (root != nullptr) {
        generateCodesHelper(root, "", codes);
    }
    return codes;
}

void HuffmanTree::generateCodesHelper(std::shared_ptr<Node> node, const std::string& code,
                                     std::unordered_map<char, std::string>& codes) const {
    if (node == nullptr) {
        return;
    }
    if (node->isLeaf()) {
        codes[node->ch] = code;
        return;
    }
    generateCodesHelper(node->left, code + "0", codes);
    generateCodesHelper(node->right, code + "1", codes);
}

std::string HuffmanTree::serializeTreeJSON() const {
    if (root == nullptr) {
        return "null";
    }
    std::stringstream ss;
    serializeTreeJSONHelper(root, ss);
    return ss.str();
}

void HuffmanTree::serializeTreeJSONHelper(std::shared_ptr<Node> node, std::ostream& ss) const {
    if (node == nullptr) {
        ss << "null";
        return;
    }
    ss << "{";
    ss << "\"frequency\":" << node->frequency << ",";
    if (node->isLeaf()) {
        ss << "\"ascii\":" << static_cast<int>(static_cast<uint8_t>(node->ch)) << ",";
        ss << "\"name\":";
        std::string name;
        char ch = node->ch;
        if (ch == ' ') name = "Space";
        else if (ch == '\n') name = "\\\\n";
        else if (ch == '\r') name = "\\\\r";
        else if (ch == '\t') name = "\\\\t";
        else if (static_cast<uint8_t>(ch) >= 32 && static_cast<uint8_t>(ch) < 127) {
            name = std::string(1, ch);
        } else {
            name = "Char_" + std::to_string(static_cast<int>(static_cast<uint8_t>(ch)));
        }
        
        ss << "\"";
        for (char c : name) {
            if (c == '"') ss << "\\\"";
            else if (c == '\\') ss << "\\\\";
            else ss << c;
        }
        ss << "\"";
    } else {
        ss << "\"left\":";
        serializeTreeJSONHelper(node->left, ss);
        ss << ",\"right\":";
        serializeTreeJSONHelper(node->right, ss);
    }
    ss << "}";
}

