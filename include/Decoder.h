#ifndef DECODER_H
#define DECODER_H

#include "Node.h"
#include <iostream>
#include <memory>



class Decoder {
public:
    Decoder() = default;
    ~Decoder() = default;


    //Decodes the compressed binary stream and writes original characters to output.
    void decode(std::istream& input, std::ostream& output);
};

#endif // DECODER_H
