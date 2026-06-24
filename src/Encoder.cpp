#include "Encoder.h"
#include <stdexcept>

namespace {
// Helper class to handle bit-level writing to an output stream
class BitWriter {
private:
    std::ostream& out;
    uint8_t buffer;
    int bitCount;

public:
    explicit BitWriter(std::ostream& os) : out(os), buffer(0), bitCount(0) {}

    ~BitWriter() {
        flush();
    }

    void writeBit(int bit) {
        buffer = static_cast<uint8_t>((buffer << 1) | (bit & 1));
        bitCount++;
        if (bitCount == 8) {
            out.put(static_cast<char>(buffer));
            buffer = 0;
            bitCount = 0;
        }
    }

    void writeBits(const std::string& bits) {
        for (char c : bits) {
            writeBit(c == '1' ? 1 : 0);
        }
    }

    void flush() {
        if (bitCount > 0) {
            buffer <<= (8 - bitCount);
            out.put(static_cast<char>(buffer));
            buffer = 0;
            bitCount = 0;
        }
    }
};

// Serialize tree structure (0 = internal, 1 + char = leaf)
void serializeTree(std::shared_ptr<Node> node, BitWriter& writer) {
    if (!node) return;
    if (node->isLeaf()) {
        writer.writeBit(1);
        for (int i = 7; i >= 0; --i) {
            writer.writeBit((node->ch >> i) & 1);
        }
    } else {
        writer.writeBit(0);
        serializeTree(node->left, writer);
        serializeTree(node->right, writer);
    }
}
} // namespace

void Encoder::encode(std::istream& input, std::ostream& output,
                     std::shared_ptr<Node> root, uint64_t originalSize,
                     const std::unordered_map<char, std::string>& codes) {
    // Write header metadata
    output.write("HUFF", 4);
    output.write(reinterpret_cast<const char*>(&originalSize), sizeof(originalSize));

    if (originalSize == 0) {
        return;
    }

    if (root == nullptr) {
        throw std::runtime_error("Encoder error: Huffman Tree is empty but original size is non-zero.");
    }

    BitWriter writer(output);

    // Serialize tree
    serializeTree(root, writer);

    // Encode text payload
    char ch;
    input >> std::noskipws;
    while (input.get(ch)) {
        auto it = codes.find(ch);
        if (it == codes.end()) {
            throw std::runtime_error("Encoder error: Found character in stream not present in Huffman code map.");
        }
        writer.writeBits(it->second);
    }
    
    writer.flush();
}
