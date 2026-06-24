#include "Decoder.h"
#include <stdexcept>

namespace {
// Helper to read bit-by-bit from a stream
class BitReader {
private:
    std::istream& in;
    uint8_t buffer;
    int bitCount;

public:
    explicit BitReader(std::istream& is) : in(is), buffer(0), bitCount(0) {}

    int readBit() {
        if (bitCount == 0) {
            int nextByte = in.get();
            if (nextByte == EOF) {
                return -1;
            }
            buffer = static_cast<uint8_t>(nextByte);
            bitCount = 8;
        }
        int bit = (buffer >> (bitCount - 1)) & 1;
        bitCount--;
        return bit;
    }
};

// Rebuild Huffman Tree from serialized format
std::shared_ptr<Node> deserializeTree(BitReader& reader) {
    int bit = reader.readBit();
    if (bit == -1) {
        throw std::runtime_error("Corrupt file: EOF during tree deserialization");
    }

    if (bit == 1) {
        char ch = 0;
        for (int i = 0; i < 8; ++i) {
            int b = reader.readBit();
            if (b == -1) {
                throw std::runtime_error("Corrupt file: EOF during char deserialization");
            }
            ch = static_cast<char>((ch << 1) | (b & 1));
        }
        return std::make_shared<Node>(ch, 0);
    } else {
        auto left = deserializeTree(reader);
        auto right = deserializeTree(reader);
        return std::make_shared<Node>('\0', 0, left, right);
    }
}
} // namespace

void Decoder::decode(std::istream& input, std::ostream& output) {
    // Verify magic bytes
    char magic[4];
    input.read(magic, 4);
    if (!input || magic[0] != 'H' || magic[1] != 'U' || magic[2] != 'F' || magic[3] != 'F') {
        throw std::runtime_error("Header validation failed: Invalid magic bytes");
    }

    // Read expected output size
    uint64_t originalSize = 0;
    input.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));
    if (!input) {
        throw std::runtime_error("Header validation failed: Failed to read original size");
    }

    if (originalSize == 0) {
        return;
    }

    BitReader reader(input);

    // Rebuild tree structure
    auto root = deserializeTree(reader);
    if (!root) {
        throw std::runtime_error("Decoder error: Reconstructed tree is null");
    }

    // Single-leaf edge case
    if (root->isLeaf()) {
        for (uint64_t i = 0; i < originalSize; ++i) {
            output.put(root->ch);
        }
        return;
    }

    // Traverse tree to write symbols
    std::shared_ptr<Node> current = root;
    uint64_t decodedCount = 0;

    while (decodedCount < originalSize) {
        int bit = reader.readBit();
        if (bit == -1) {
            throw std::runtime_error("Decoder error: Unexpected EOF in bitstream");
        }

        if (bit == 0) {
            current = current->left;
        } else if (bit == 1) {
            current = current->right;
        }

        if (!current) {
            throw std::runtime_error("Decoder error: Invalid tree navigation state");
        }

        if (current->isLeaf()) {
            output.put(current->ch);
            decodedCount++;
            current = root;
        }
    }
}
