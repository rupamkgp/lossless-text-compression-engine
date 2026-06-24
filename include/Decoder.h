#ifndef DECODER_H
#define DECODER_H

#include "Node.h"
#include <iostream>
#include <memory>

class Decoder {
public:
    Decoder() = default;
    ~Decoder() = default;

    // Decodes binary stream to original text
    void decode(std::istream& input, std::ostream& output);
};

#endif // DECODER_H
