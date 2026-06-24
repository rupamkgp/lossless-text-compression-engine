#ifndef ENCODER_H
#define ENCODER_H

#include "Node.h"
#include <string>
#include <unordered_map>
#include <iostream>
#include <memory>

class Encoder {
public:
    Encoder() = default;
    ~Encoder() = default;

    void encode(std::istream& input, std::ostream& output,
                std::shared_ptr<Node> root, uint64_t originalSize,
                const std::unordered_map<char, std::string>& codes);
};

#endif // ENCODER_H
