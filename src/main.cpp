#include "Compressor.h"
#include "FileManager.h"
#include "HuffmanTree.h"
#include "Encoder.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <sstream>

void printUsage() {
    std::cout << "========================================================\n"
              << "          Lossless Text Compression Engine (C++17)      \n"
              << "========================================================\n"
              << "Usage:\n"
              << "  Standard Mode:\n"
              << "    Compress:   ./huffman_app -c <input_file> <output_file>\n"
              << "    Decompress: ./huffman_app -d <input_file> <output_file>\n"
              << "  API Mode (Outputs JSON):\n"
              << "    Compress:   ./huffman_app -api -c <input_file> <output_file>\n"
              << "    Decompress: ./huffman_app -api -d <input_file> <output_file>\n"
              << "========================================================\n";
}

// Escape control and quote characters for JSON safety
std::string escapeJSONString(const std::string& input) {
    std::stringstream ss;
    for (char c : input) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\n') ss << "\\n";
        else if (c == '\r') ss << "\\r";
        else if (c == '\t') ss << "\\t";
        else if (static_cast<uint8_t>(c) < 32) {
            char hex[8];
            snprintf(hex, sizeof(hex), "\\u%04x", static_cast<uint8_t>(c));
            ss << hex;
        } else {
            ss << c;
        }
    }
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage();
        return 1;
    }

    bool isApiMode = false;
    std::string mode;
    std::string inputPath;
    std::string outputPath;

    // Parse command line arguments
    if (std::string(argv[1]) == "-api") {
        if (argc < 5) {
            std::cerr << "{\"error\":\"Missing arguments for API mode\"}\n";
            return 1;
        }
        isApiMode = true;
        mode = argv[2];
        inputPath = argv[3];
        outputPath = argv[4];
    } else {
        mode = argv[1];
        inputPath = argv[2];
        outputPath = argv[3];
    }

    try {
        if (mode == "-c") {
            if (!isApiMode) {
                std::cout << "[INFO] Compressing: " << inputPath << " to " << outputPath << " ...\n";
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            
            FileManager fileMgr;
            uint64_t originalSize = FileManager::getFileSize(inputPath);
            auto freqMap = fileMgr.calculateFrequencies(inputPath);
            
            HuffmanTree tree;
            tree.build(freqMap);
            auto codes = tree.generateCodes();

            std::ofstream out(outputPath, std::ios::binary);
            if (!out.is_open()) {
                throw std::runtime_error("Compression error: Could not open output file: " + outputPath);
            }

            std::string bitString;
            if (originalSize > 0) {
                std::ifstream in(inputPath, std::ios::binary);
                if (!in.is_open()) {
                    throw std::runtime_error("Compression error: Could not open input file: " + inputPath);
                }
                Encoder encoder;
                encoder.encode(in, out, tree.getRoot(), originalSize, codes);

                // Reconstruct bitstream for frontend UI visualization
                in.clear();
                in.seekg(0);
                char ch;
                in >> std::noskipws;
                while (in.get(ch)) {
                    bitString += codes[ch];
                }
            } else {
                Encoder encoder;
                std::ifstream dummyIn;
                encoder.encode(dummyIn, out, nullptr, 0, {});
            }
            out.close();
            
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;

            uint64_t compressedSize = FileManager::getFileSize(outputPath);
            double ratio = static_cast<double>(originalSize) / static_cast<double>(compressedSize == 0 ? 1 : compressedSize);
            double reduction = 0.0;
            if (originalSize > 0) {
                reduction = (1.0 - (static_cast<double>(compressedSize) / static_cast<double>(originalSize))) * 100.0;
            }

            std::string effectiveness;
            if (reduction > 25.0) {
                effectiveness = "Excellent Compression";
            } else if (reduction > 5.0) {
                effectiveness = "Good Compression";
            } else if (reduction >= 0.0) {
                effectiveness = "Neutral Compression";
            } else {
                effectiveness = "Compression Not Beneficial";
            }

            if (isApiMode) {
                // Serialize codes map as JSON
                std::stringstream codesJSON;
                codesJSON << "{";
                bool first = true;
                for (const auto& pair : codes) {
                    if (!first) codesJSON << ",";
                    first = false;
                    codesJSON << "\"" << static_cast<int>(static_cast<uint8_t>(pair.first)) << "\":\"" << pair.second << "\"";
                }
                codesJSON << "}";

                std::cout << "{"
                          << "\"originalSize\":" << originalSize << ","
                          << "\"compressedSize\":" << compressedSize << ","
                          << "\"ratio\":" << ratio << ","
                          << "\"reduction\":" << reduction << ","
                          << "\"effectiveness\":\"" << effectiveness << "\","
                          << "\"timeMs\":" << duration.count() << ","
                          << "\"codes\":" << codesJSON.str() << ","
                          << "\"bitString\":\"" << bitString << "\","
                          << "\"treeJSON\":" << tree.serializeTreeJSON()
                          << "}\n";
            } else {
                std::cout << "\n========================================\n"
                          << "          COMPRESSION SUCCESS\n"
                          << "========================================\n"
                          << std::fixed << std::setprecision(2)
                          << "Original Size:     " << originalSize << " bytes\n"
                          << "Compressed Size:   " << compressedSize << " bytes\n"
                          << "Compression Ratio: " << ratio << "x\n"
                          << "Reduction:         " << reduction << "%\n"
                          << "Effectiveness:     " << effectiveness << "\n"
                          << "Execution Time:    " << duration.count() << " ms\n"
                          << "========================================\n";
            }

        } else if (mode == "-d") {
            if (!isApiMode) {
                std::cout << "[INFO] Decompressing: " << inputPath << " to " << outputPath << " ...\n";
            }

            auto start = std::chrono::high_resolution_clock::now();

            HuffmanCompressor compressor(inputPath, outputPath);
            compressor.decompress();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;

            uint64_t compressedSize = FileManager::getFileSize(inputPath);
            uint64_t decompressedSize = FileManager::getFileSize(outputPath);

            std::string decompressedText;
            if (isApiMode) {
                std::ifstream in(outputPath, std::ios::binary);
                if (in.is_open()) {
                    char ch;
                    in >> std::noskipws;
                    while (in.get(ch)) {
                        decompressedText += ch;
                    }
                }
            }

            if (isApiMode) {
                std::cout << "{"
                          << "\"compressedSize\":" << compressedSize << ","
                          << "\"decompressedSize\":" << decompressedSize << ","
                          << "\"timeMs\":" << duration.count() << ","
                          << "\"decompressedText\":\"" << escapeJSONString(decompressedText) << "\""
                          << "}\n";
            } else {
                std::cout << "\n========================================\n"
                          << "         DECOMPRESSION SUCCESS\n"
                          << "========================================\n"
                          << std::fixed << std::setprecision(2)
                          << "Compressed Size:   " << compressedSize << " bytes\n"
                          << "Decompressed Size: " << decompressedSize << " bytes\n"
                          << "Execution Time:    " << duration.count() << " ms\n"
                          << "========================================\n";
            }
        } else {
            if (isApiMode) {
                std::cerr << "{\"error\":\"Unknown mode '" << mode << "'\"}\n";
            } else {
                std::cerr << "[ERROR] Unknown mode '" << mode << "'\n";
                printUsage();
            }
            return 1;
        }
    } catch (const std::exception& e) {
        if (isApiMode) {
            std::cout << "{\"error\":\"" << escapeJSONString(e.what()) << "\"}\n";
        } else {
            std::cerr << "\n\033[31m[ERROR] Operation failed: " << e.what() << "\033[0m\n";
        }
        return 1;
    }

    return 0;
}
