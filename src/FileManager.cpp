#include "FileManager.h"
#include <fstream>
#include <stdexcept>

bool FileManager::fileExists(const std::string& filepath) {
    std::ifstream f(filepath.c_str());
    return f.good();
}

uint64_t FileManager::getFileSize(const std::string& filepath) {
    if (!fileExists(filepath)) {
        throw std::runtime_error("File not found: " + filepath);
    }
    std::ifstream in(filepath, std::ifstream::ate | std::ifstream::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Unable to open file: " + filepath);
    }
    return static_cast<uint64_t>(in.tellg());
}

std::unordered_map<char, uint64_t> FileManager::calculateFrequencies(const std::string& filepath) {
    std::ifstream in(filepath, std::ifstream::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Unable to read file: " + filepath);
    }

    std::unordered_map<char, uint64_t> freqMap;
    char ch;
    in >> std::noskipws;
    while (in.get(ch)) {
        freqMap[ch]++;
    }
    return freqMap;
}
