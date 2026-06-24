#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <string>

// Base interface for compression
class Compressor {
public:
    virtual ~Compressor() = default;
    virtual void compress() = 0;
    virtual void decompress() = 0;
};

class HuffmanCompressor : public Compressor {
private:
    std::string inputPath;
    std::string outputPath;

public:
    HuffmanCompressor(std::string input, std::string output)
        : inputPath(std::move(input)), outputPath(std::move(output)) {}
    ~HuffmanCompressor() override = default;

    void compress() override;
    void decompress() override;
};

#endif // COMPRESSOR_H
