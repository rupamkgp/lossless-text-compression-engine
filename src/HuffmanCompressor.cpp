#include "Compressor.h"
#include "FileManager.h"
#include "HuffmanTree.h"
#include "Encoder.h"
#include "Decoder.h"
#include <fstream>
#include <stdexcept>

void HuffmanCompressor::compress() {
    uint64_t originalSize = FileManager::getFileSize(inputPath);

    FileManager fileMgr;
    auto freqMap = fileMgr.calculateFrequencies(inputPath);

    HuffmanTree tree;
    tree.build(freqMap);

    auto codes = tree.generateCodes();

    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to write compressed output: " + outputPath);
    }

    if (originalSize > 0) {
        std::ifstream in(inputPath, std::ios::binary);
        if (!in.is_open()) {
            throw std::runtime_error("Failed to read input file: " + inputPath);
        }
        Encoder encoder;
        encoder.encode(in, out, tree.getRoot(), originalSize, codes);
    } else {
        Encoder encoder;
        std::ifstream dummyIn; 
        encoder.encode(dummyIn, out, nullptr, 0, {});
    }
}

void HuffmanCompressor::decompress() {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Failed to read compressed file: " + inputPath);
    }

    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to write decompressed output: " + outputPath);
    }

    Decoder decoder;
    decoder.decode(in, out);
}
