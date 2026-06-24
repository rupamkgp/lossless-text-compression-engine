#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <string>

//Base abstract interface for compression engines.
//Design enables modular support for future formats (e.g. Binary, PDF, Image compression)
//by keeping the interface simple and independent of algorithmic details.
 
class Compressor {
public:
    virtual ~Compressor() = default;

    //Performs compression
    virtual void compress() = 0;

    //Performs decompression
    virtual void decompress() = 0;
};


class HuffmanCompressor : public Compressor {
private:
    std::string inputPath;
    std::string outputPath;

public:
    // Construct a HuffmanCompressor instance with file targets.
    HuffmanCompressor(std::string input, std::string output)
        : inputPath(std::move(input)), outputPath(std::move(output)) {}

    ~HuffmanCompressor() override = default;


    // Compress the target input file using Huffman Coding.
    void compress() override;

    // Decompress the target input file back to original form
    void decompress() override;
};

#endif // COMPRESSOR_H
