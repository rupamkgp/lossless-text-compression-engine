#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <unordered_map>

class FileManager {
public:
    FileManager() = default;
    ~FileManager() = default;

    static bool fileExists(const std::string& filepath);
    static uint64_t getFileSize(const std::string& filepath);

    std::unordered_map<char, uint64_t> calculateFrequencies(const std::string& filepath);
};

#endif // FILEMANAGER_H
