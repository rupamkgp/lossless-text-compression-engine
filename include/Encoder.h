#ifndef ENCODER_H
#define ENCODER_H

#include "Node.h"
#include <string>
#include <unordered_map>
#include <iostream>
#include <memory>

 // Class responsible for translating text into binary Huffman representation.
// Writes magic bytes, original size, serialized tree shape, and packed body bits.

class Encoder {
public:
    Encoder() = default;
    ~Encoder() = default;


     //@brief Encodes characters from input stream and writes packed bit bytes to output.
     //@param input Readable source stream.
     //@param output Writable binary stream.
     //@param root The root node of the Huffman Tree.
     //@param originalSize Uncompressed file size.
     //@param codes Generated Huffman codes mapping (character -> binary bit string).
     
    
    void encode(std::istream& input, std::ostream& output,
                std::shared_ptr<Node> root, uint64_t originalSize,
                const std::unordered_map<char, std::string>& codes);
};

#endif // ENCODER_H
