#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <unordered_map>
#include <iostream>

// Class responsible for file operations, integrity checks, and frequency analysis.

class FileManager {
public:
    FileManager() = default;
    ~FileManager() = default;

    // Checks if a file exists and is readable.
    static bool fileExists(const std::string& filepath);

    // Returns the size of a file in bytes.
    static uint64_t getFileSize(const std::string& filepath);

    // Reads a file stream to compute character/byte frequencies.
    // filepath Path to the input file.
    // Map of characters to occurrence count.
    std::unordered_map<char, uint64_t> calculateFrequencies(const std::string& filepath);
};

#endif // FILEMANAGER_H
