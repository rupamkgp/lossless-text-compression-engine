#include "HuffmanTree.h"
#include "Compressor.h"
#include "FileManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <iomanip>
#include <cstdio>
#include <stdexcept>
#include <algorithm>

// Structure to hold benchmarks for report generation
struct BenchmarkMetric {
    std::string testName;
    uint64_t originalSize;
    uint64_t compressedSize;
    double ratio;
    double reduction;
    double compressTimeMs;
    double decompressTimeMs;
    bool integrityPassed;
};

// Global benchmark results collection
std::vector<BenchmarkMetric> benchmarkResults;

// Test framework console markers for unit tests
#define RUN_TEST(testName) \
    std::cout << "[RUNNING] " << #testName << " ... "; \
    if (testName()) { \
        std::cout << "\033[32m[PASSED]\033[0m\n"; \
        passedCount++; \
    } else { \
        std::cout << "\033[31m[FAILED]\033[0m\n"; \
        failedCount++; \
    }

// Helper to run full round-trip compression and decompression for unit tests
bool runRoundTripTest(const std::string& originalText, const std::string& tempBaseName) {
    std::string inputPath = tempBaseName + "_input.txt";
    std::string compressedPath = tempBaseName + "_output.huff";
    std::string decompressedPath = tempBaseName + "_decompressed.txt";

    // Write input
    {
        std::ofstream out(inputPath, std::ios::binary);
        out << originalText;
    }

    // Compress
    try {
        HuffmanCompressor compressor(inputPath, compressedPath);
        compressor.compress();
    } catch (...) {
        std::remove(inputPath.c_str());
        return false;
    }

    // Decompress
    try {
        HuffmanCompressor decompressor(compressedPath, decompressedPath);
        decompressor.decompress();
    } catch (...) {
        std::remove(inputPath.c_str());
        std::remove(compressedPath.c_str());
        return false;
    }

    // Verify text match
    std::string decompressedText;
    {
        std::ifstream in(decompressedPath, std::ios::binary);
        char ch;
        in >> std::noskipws;
        while (in.get(ch)) {
            decompressedText += ch;
        }
    }

    // Clean up
    std::remove(inputPath.c_str());
    std::remove(compressedPath.c_str());
    std::remove(decompressedPath.c_str());

    return (originalText == decompressedText);
}

// -------------------------------------------------------------
// Unit Tests (Functionality and Edge Cases)
// -------------------------------------------------------------

bool testEmptyString() {
    return runRoundTripTest("", "test_empty");
}

bool testSingleCharacter() {
    return runRoundTripTest("a", "test_single");
}

bool testRepeatedCharacter() {
    return runRoundTripTest("aaaaaaaaaaaaaaaaaaaa", "test_repeated");
}

bool testTwoCharacters() {
    return runRoundTripTest("ab", "test_two");
}

bool testRepeatedTwoCharacters() {
    return runRoundTripTest("abababababababababab", "test_repeated_two");
}

bool testWhitespaceOnly() {
    return runRoundTripTest(" \n\t \r \n \t", "test_whitespace");
}

bool testNumbersOnly() {
    return runRoundTripTest("1234567890987654321", "test_numbers");
}

bool testSymbolsOnly() {
    return runRoundTripTest("!@#$%^&*()_+=-`~[]\\{}|;':\",./<>?", "test_symbols");
}

bool testMixedAlphaNumeric() {
    return runRoundTripTest("hello123world456!", "test_mixed");
}

bool testLongParagraph() {
    std::string paragraph = "Huffman coding is an entropy encoding algorithm used for lossless data compression. "
                            "The program was designed by David A. Huffman while he was a Ph.D. student at MIT, "
                            "and published in the 1952 paper \"A Method for the Construction of Minimum-Redundancy Codes\". "
                            "The output from Huffman's algorithm can be viewed as a variable-length code table for encoding "
                            "a source symbol (such as a character in a file).";
    return runRoundTripTest(paragraph, "test_paragraph");
}

bool testAllASCII() {
    std::string allAscii;
    for (int i = 32; i < 127; ++i) {
        allAscii += static_cast<char>(i);
    }
    return runRoundTripTest(allAscii, "test_ascii");
}

bool testBinaryLikePattern() {
    std::string binaryPattern;
    for (int i = 0; i < 100; ++i) {
        binaryPattern += (i % 2 == 0) ? "A" : "B";
    }
    return runRoundTripTest(binaryPattern, "test_binary_like");
}

bool testHuffmanTreeStructureEmpty() {
    HuffmanTree tree;
    std::unordered_map<char, uint64_t> freqMap;
    tree.build(freqMap);
    return (tree.getRoot() == nullptr);
}

bool testHuffmanTreeStructureSingle() {
    HuffmanTree tree;
    std::unordered_map<char, uint64_t> freqMap = {{'a', 5}};
    tree.build(freqMap);
    auto root = tree.getRoot();
    if (!root || root->isLeaf()) return false;
    if (!root->left || root->left->ch != 'a' || root->left->frequency != 5) return false;
    if (!root->right || root->right->frequency != 0) return false; // Dummy node
    return true;
}

bool testHuffmanTreeStructureMulti() {
    HuffmanTree tree;
    std::unordered_map<char, uint64_t> freqMap = {{'a', 5}, {'b', 10}, {'c', 15}};
    tree.build(freqMap);
    auto root = tree.getRoot();
    if (!root || root->frequency != 30) return false;
    auto codes = tree.generateCodes();
    if (codes['a'].empty() || codes['b'].empty() || codes['c'].empty()) return false;
    return true;
}

bool testFileMissing() {
    try {
        HuffmanCompressor compressor("nonexistent_file_xyz.txt", "out.huff");
        compressor.compress();
        return false; 
    } catch (const std::runtime_error& e) {
        return true; 
    }
}

bool testFileEmpty() {
    std::string emptyFile = "test_empty_file.txt";
    std::string compressedFile = "test_empty_file.huff";
    std::string decompressedFile = "test_empty_file_decomp.txt";

    {
        std::ofstream out(emptyFile, std::ios::binary);
    }

    try {
        HuffmanCompressor compressor(emptyFile, compressedFile);
        compressor.compress();

        HuffmanCompressor decompressor(compressedFile, decompressedFile);
        decompressor.decompress();

        uint64_t origSz = FileManager::getFileSize(emptyFile);
        uint64_t decompSz = FileManager::getFileSize(decompressedFile);

        std::remove(emptyFile.c_str());
        std::remove(compressedFile.c_str());
        std::remove(decompressedFile.c_str());

        return (origSz == 0 && decompSz == 0);
    } catch (...) {
        std::remove(emptyFile.c_str());
        std::remove(compressedFile.c_str());
        std::remove(decompressedFile.c_str());
        return false;
    }
}

bool testCorruptedHeader() {
    std::string corruptFile = "corrupt_test.huff";
    {
        std::ofstream out(corruptFile, std::ios::binary);
        out.write("BAD_MAGIC_BYTES", 15);
    }

    try {
        HuffmanCompressor decompressor(corruptFile, "out_test.txt");
        decompressor.decompress();
        std::remove(corruptFile.c_str());
        return false; 
    } catch (const std::runtime_error& e) {
        std::remove(corruptFile.c_str());
        std::string msg = e.what();
        return (msg.find("Header validation") != std::string::npos);
    }
}

bool testUnicodeReadyDiscussion() {
    std::string unicodeText = "Hello 世界 🌍! UTF-8 has symbols: µ, ¶, ¿, and emojis: 🚀✨.";
    return runRoundTripTest(unicodeText, "test_unicode");
}

bool testLargeTextCompression() {
    std::string largeText;
    for (int i = 0; i < 500; ++i) {
        largeText += "Lorem ipsum dolor sit amet, consectetur adipiscing elit. ";
        largeText += "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ";
        largeText += "1234567890!@#$%^&*() \n";
    }
    return runRoundTripTest(largeText, "test_large");
}

bool testIntegrityCheck() {
    std::string specChars = "\x01\x02\x03\x00\xFF\x7F\x80\n\r\t";
    return runRoundTripTest(specChars, "test_integrity");
}

// -------------------------------------------------------------
// Benchmark Suite Runner
// -------------------------------------------------------------

bool runBenchmarkCase(const std::string& name, const std::string& content) {
    // Sanitize name for file paths
    std::string safeName = name;
    std::replace(safeName.begin(), safeName.end(), ' ', '_');
    std::replace(safeName.begin(), safeName.end(), '/', '_');
    std::replace(safeName.begin(), safeName.end(), '(', '_');
    std::replace(safeName.begin(), safeName.end(), ')', '_');

    std::string inputPath = "bench_" + safeName + "_input.txt";
    std::string compressedPath = "bench_" + safeName + "_output.huff";
    std::string decompressedPath = "bench_" + safeName + "_decompressed.txt";

    // Write input content
    {
        std::ofstream out(inputPath, std::ios::binary);
        out << content;
    }

    // Compress and time
    double compressTime = 0.0;
    try {
        auto start = std::chrono::high_resolution_clock::now();
        HuffmanCompressor compressor(inputPath, compressedPath);
        compressor.compress();
        auto end = std::chrono::high_resolution_clock::now();
        compressTime = std::chrono::duration<double, std::milli>(end - start).count();
    } catch (const std::exception& e) {
        std::cerr << "\n[BENCHMARK ERROR] " << name << " compression failed: " << e.what() << "\n";
        std::remove(inputPath.c_str());
        return false;
    }

    // Decompress and time
    double decompressTime = 0.0;
    try {
        auto start = std::chrono::high_resolution_clock::now();
        HuffmanCompressor decompressor(compressedPath, decompressedPath);
        decompressor.decompress();
        auto end = std::chrono::high_resolution_clock::now();
        decompressTime = std::chrono::duration<double, std::milli>(end - start).count();
    } catch (const std::exception& e) {
        std::cerr << "\n[BENCHMARK ERROR] " << name << " decompression failed: " << e.what() << "\n";
        std::remove(inputPath.c_str());
        std::remove(compressedPath.c_str());
        return false;
    }

    // Verify integrity
    std::string decompressedText;
    {
        std::ifstream in(decompressedPath, std::ios::binary);
        char ch;
        in >> std::noskipws;
        while (in.get(ch)) {
            decompressedText += ch;
        }
    }

    uint64_t originalSize = content.size();
    uint64_t compressedSize = FileManager::getFileSize(compressedPath);

    // Clean up temporary files
    std::remove(inputPath.c_str());
    std::remove(compressedPath.c_str());
    std::remove(decompressedPath.c_str());

    bool integrityPassed = (content == decompressedText);

    double ratio = static_cast<double>(originalSize) / (compressedSize == 0 ? 1 : compressedSize);
    double reduction = 0.0;
    if (originalSize > 0) {
        reduction = (1.0 - (static_cast<double>(compressedSize) / originalSize)) * 100.0;
    }

    benchmarkResults.push_back({
        name,
        originalSize,
        compressedSize,
        ratio,
        reduction,
        compressTime,
        decompressTime,
        integrityPassed
    });

    return integrityPassed;
}

void runBenchmarks() {
    std::cout << "\n======================================================\n";
    std::cout << "           RUNNING HUFFMAN BENCHMARK SUITE              \n";
    std::cout << "========================================================\n";

    // 1. English Literature (Classic Prose)
    {
        std::string text = "It was the best of times, it was the worst of times, "
                           "it was the age of wisdom, it was the age of foolishness, "
                           "it was the epoch of belief, it was the epoch of incredulity, "
                           "it was the season of light, it was the season of darkness, "
                           "it was the spring of hope, it was the winter of despair, "
                           "we had everything before us, we had nothing before us, "
                           "we were all going direct to Heaven, we were all going direct the other way—"
                           "in short, the period was so far like the present period, that some of its "
                           "noisiest authorities insisted on its being received, for good or for evil, "
                           "in the superlative degree of comparison only.";
        std::string repeated;
        for (int i = 0; i < 8; ++i) repeated += text + "\n";
        runBenchmarkCase("English Literature (Prose)", repeated);
    }

    // 2. C++ Source Code
    {
        std::string code = 
            "#include <iostream>\n"
            "#include <vector>\n"
            "#include <string>\n"
            "#include <algorithm>\n\n"
            "class Node {\n"
            "public:\n"
            "    char ch;\n"
            "    int freq;\n"
            "    Node* left;\n"
            "    Node* right;\n"
            "    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}\n"
            "    bool isLeaf() const { return !left && !right; }\n"
            "};\n\n"
            "void printCodes(Node* root, std::string str) {\n"
            "    if (!root) return;\n"
            "    if (root->isLeaf()) {\n"
            "        std::cout << root->ch << \": \" << str << \"\\n\";\n"
            "    }\n"
            "    printCodes(root->left, str + \"0\");\n"
            "    printCodes(root->right, str + \"1\");\n"
            "}\n";
        std::string repeated;
        for (int i = 0; i < 8; ++i) repeated += code + "\n";
        runBenchmarkCase("C++ Source Code", repeated);
    }

    // 3. System Log
    {
        std::stringstream ss;
        for (int i = 0; i < 60; ++i) {
            ss << "[2026-06-24 22:34:21] INFO  [Worker-" << (i % 5) 
               << "] Task " << i << " processed successfully. "
               << "Bytes read: " << (i * 128) << ". Status: OK.\n";
        }
        runBenchmarkCase("System Log File", ss.str());
    }

    // 4. JSON Data
    {
        std::stringstream ss;
        ss << "[\n";
        for (int i = 0; i < 25; ++i) {
            ss << "  {\n"
               << "    \"id\": " << (1000 + i) << ",\n"
               << "    \"name\": \"Employee_" << i << "\",\n"
               << "    \"email\": \"employee_" << i << "@company.com\",\n"
               << "    \"isActive\": " << ((i % 2 == 0) ? "true" : "false") << ",\n"
               << "    \"roles\": [\"user\", \"developer\"]\n"
               << "  }" << (i == 24 ? "" : ",") << "\n";
        }
        ss << "]\n";
        runBenchmarkCase("JSON Data Response", ss.str());
    }

    // 5. XML Configuration
    {
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<configuration>\n";
        for (int i = 0; i < 25; ++i) {
            ss << "  <property id=\"" << i << "\">\n"
               << "    <name>system.parameter.node_" << i << "</name>\n"
               << "    <value>value_settings_" << (i * 10) << "</value>\n"
               << "    <description>Dynamic system setting node for index " << i << "</description>\n"
               << "  </property>\n";
        }
        ss << "</configuration>\n";
        runBenchmarkCase("XML Configuration", ss.str());
    }

    // 6. Markdown Documentation
    {
        std::string md = 
            "# Lossless Huffman Compression Engine\n\n"
            "This project implements a high-performance lossless text compression engine.\n"
            "It uses Huffman Coding with bit-level tree serialization.\n\n"
            "## Build Instructions\n"
            "- Run `make` or compile directly with `clang++`.\n"
            "- Ensure C++17 support is enabled.\n\n"
            "## Command Line Arguments\n"
            "1. `-c` : Compress file\n"
            "2. `-d` : Decompress file\n"
            "3. `-api` : Output metrics in structured JSON format.\n\n"
            "### Example Usage:\n"
            "```bash\n"
            "./huffman_app -c input.txt output.huff\n"
            "```\n";
        std::string repeated;
        for (int i = 0; i < 10; ++i) repeated += md + "\n";
        runBenchmarkCase("Markdown Documentation", repeated);
    }

    // 7. CSS Stylesheet
    {
        std::string css = 
            ".container {\n"
            "    width: 100%;\n"
            "    padding-right: 15px;\n"
            "    padding-left: 15px;\n"
            "    margin-right: auto;\n"
            "    margin-left: auto;\n"
            "}\n"
            ".btn-primary-gradient {\n"
            "    color: #ffffff;\n"
            "    background-image: linear-gradient(180deg, #007bff 0%, #0056b3 100%);\n"
            "    border-color: #0056b3;\n"
            "    border-radius: 4px;\n"
            "    padding: 10px 20px;\n"
            "    font-family: 'Inter', sans-serif;\n"
            "    font-weight: 600;\n"
            "    transition: background 0.2s ease-in-out;\n"
            "}\n";
        std::string repeated;
        for (int i = 0; i < 12; ++i) repeated += css + "\n";
        runBenchmarkCase("CSS Stylesheet", repeated);
    }

    // 8. CSV Dataset
    {
        std::stringstream ss;
        ss << "Index,FirstName,LastName,Email,Department,Salary,DateHired\n";
        for (int i = 0; i < 50; ++i) {
            ss << i << ",John,Doe_" << i << ",john.doe." << i << "@enterprise.org,Engineering," 
               << (80000 + i * 500) << ",2026-06-" << (1 + i % 28) << "\n";
        }
        runBenchmarkCase("CSV Employee Dataset", ss.str());
    }

    // 9. Python Script
    {
        std::string py = 
            "import os\n"
            "import sys\n"
            "import math\n\n"
            "def compute_entropy(data):\n"
            "    if not data:\n"
            "        return 0\n"
            "    frequencies = {}\n"
            "    for char in data:\n"
            "        frequencies[char] = frequencies.get(char, 0) + 1\n"
            "    entropy = 0.0\n"
            "    for count in frequencies.values():\n"
            "        p = count / len(data)\n"
            "        entropy -= p * math.log2(p)\n"
            "    return entropy\n\n"
            "if __name__ == '__main__':\n"
            "    sample = 'huffman algorithm sample text for entropy'\n"
            "    print(f'Entropy: {compute_entropy(sample):.4f}')\n";
        std::string repeated;
        for (int i = 0; i < 12; ++i) repeated += py + "\n";
        runBenchmarkCase("Python Script File", repeated);
    }

    // 10. HTML Page
    {
        std::string html = 
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <title>Huffman Engine Benchmark UI</title>\n"
            "    <style>body { font-family: sans-serif; background: #121212; color: #fff; }</style>\n"
            "</head>\n"
            "<body>\n"
            "    <div class=\"container\">\n"
            "        <h1>Engine Dashboard</h1>\n"
            "        <p>Integrity: <span class=\"status-green\">PASSED</span></p>\n"
            "        <p>Compression Ratio: <span class=\"ratio\">1.85x</span></p>\n"
            "    </div>\n"
            "</body>\n"
            "</html>\n";
        std::string repeated;
        for (int i = 0; i < 10; ++i) repeated += html + "\n";
        runBenchmarkCase("HTML Page Template", repeated);
    }

    // 11. SQL Schema & Queries
    {
        std::string sql = 
            "CREATE TABLE IF NOT EXISTS system_metrics (\n"
            "    metric_id INT PRIMARY KEY AUTO_INCREMENT,\n"
            "    metric_name VARCHAR(100) NOT NULL,\n"
            "    value DOUBLE NOT NULL,\n"
            "    recorded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP\n"
            ");\n"
            "INSERT INTO system_metrics (metric_name, value) VALUES ('cpu_load', 0.42);\n"
            "INSERT INTO system_metrics (metric_name, value) VALUES ('memory_used_gb', 8.5);\n"
            "SELECT * FROM system_metrics WHERE metric_name = 'cpu_load' ORDER BY recorded_at DESC;\n";
        std::string repeated;
        for (int i = 0; i < 15; ++i) repeated += sql + "\n";
        runBenchmarkCase("SQL Database Schema", repeated);
    }

    // 12. Lorem Ipsum
    {
        std::string lorem = 
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
            "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
            "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
            "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
            "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n";
        std::string repeated;
        for (int i = 0; i < 15; ++i) repeated += lorem + "\n";
        runBenchmarkCase("Lorem Ipsum Text", repeated);
    }

    // 13. Technical Manual
    {
        std::string manual = 
            "Transmission Control Protocol (TCP) is one of the main protocols of the Internet "
            "Protocol Suite. It originated in the initial network implementation in which it "
            "complemented the Internet Protocol (IP). Therefore, the entire suite is commonly "
            "referred to as TCP/IP. TCP provides reliable, ordered, and error-checked delivery "
            "of a stream of octets (bytes) between applications running on hosts communicating "
            "via an IP network. Major internet applications such as the World Wide Web, "
            "email, remote administration, and file transfer rely on TCP.\n";
        std::string repeated;
        for (int i = 0; i < 10; ++i) repeated += manual + "\n";
        runBenchmarkCase("TCP/IP Protocol Manual", repeated);
    }

    // 14. Repetitive DNA Sequence
    {
        std::string dna = "AAAAACCCCCGGGGGTTTTT";
        std::string repeated;
        for (int i = 0; i < 150; ++i) repeated += dna; // 3 KB
        runBenchmarkCase("Repetitive DNA Sequence", repeated);
    }

    // 15. Numeric Data
    {
        std::stringstream ss;
        for (int i = 0; i < 100; ++i) {
            ss << i << " | 0.123456 | -0.987654 | 3.14159265 | 2.7182818 | 1.41421356 | " << (i * 0.01) << "\n";
        }
        runBenchmarkCase("Numeric Matrix Data", ss.str());
    }

    // 16. Email Thread
    {
        std::string email = 
            "From: coordinator@huffman-project.org\n"
            "To: team-all@huffman-project.org\n"
            "Date: Wed, 24 Jun 2026 22:34:21 GMT\n"
            "Subject: Release Candidate for Huffman Compression Engine\n\n"
            "Hi team,\n"
            "I'm pleased to announce that our pre-order tree serialization has been merged. "
            "Please run the benchmark suites and post your compression ratios. "
            "We are targeting an average of 1.83x across standard text formats.\n"
            "Best regards,\n"
            "Project Lead Coordinator\n";
        std::string repeated;
        for (int i = 0; i < 8; ++i) repeated += email + "\n------------------\n";
        runBenchmarkCase("Corporate Email Thread", repeated);
    }

    // 17. Key-Value Config (INI)
    {
        std::string ini = 
            "[database]\n"
            "host = 127.0.0.1\n"
            "port = 5432\n"
            "username = db_admin_account\n"
            "password = high_entropy_password_123!\n"
            "max_connections = 150\n"
            "[caching]\n"
            "redis_host = 10.0.5.12\n"
            "redis_port = 6379\n"
            "ttl_seconds = 3600\n"
            "[logging]\n"
            "level = DEBUG\n"
            "file_path = /var/log/huffman_app.log\n";
        std::string repeated;
        for (int i = 0; i < 10; ++i) repeated += ini + "\n";
        runBenchmarkCase("INI Key-Value Config", repeated);
    }

    // 18. Base64 Encoded Dump
    {
        std::string hexStr = 
            "SGVsbG8gV29ybGQhIFRoaXMgaXMgYSBiYXNlNjQgZW5jb2RlZCBzdHJpbmcgdXNlZCBmb3Ig\n"
            "YmVuY2htYXJraW5nIHRoZSBIdWZmY29kZSBjb21wcmVzc2lvbiBlbmdpbmUuIEl0IGNvbnRh\n"
            "aW5zIGEgdmFyaWV0eSBvZiBhbHBoYW51bWVyaWMgY2hhcmFjdGVycyBhbmQgc29tZSBwYWRk\n"
            "aW5nLg==\n";
        std::string repeated;
        for (int i = 0; i < 25; ++i) repeated += hexStr; // ~4 KB
        runBenchmarkCase("Base64 Encoded Dump", repeated);
    }

    // 19. JavaScript Source Code
    {
        std::string js = 
            "const fs = require('fs');\n"
            "const path = require('path');\n\n"
            "function benchmarkFile(filePath) {\n"
            "    const start = process.hrtime.bigint();\n"
            "    const content = fs.readFileSync(filePath, 'utf8');\n"
            "    const counts = {};\n"
            "    for (const char of content) {\n"
            "        counts[char] = (counts[char] || 0) + 1;\n"
            "    }\n"
            "    const end = process.hrtime.bigint();\n"
            "    const timeMs = Number(end - start) / 1000000;\n"
            "    return { timeMs, size: content.length };\n"
            "}\n"
            "module.exports = { benchmarkFile };\n";
        std::string repeated;
        for (int i = 0; i < 12; ++i) repeated += js + "\n";
        runBenchmarkCase("JavaScript Source Code", repeated);
    }

    // 20. FAQ Q&A Document
    {
        std::string faq = 
            "Q: How is Huffman coding lossless?\n"
            "A: Huffman coding assigns prefix codes to symbols based on their frequency. Since no code is a prefix of another, decompression parses the exact same symbols uniquely and perfectly.\n"
            "Q: Why does a small file show negative space reduction?\n"
            "A: The file includes a small metadata header (12 bytes for Magic + Size and ~90 bytes for the tree representation). For tiny files, this overhead is larger than the bits saved.\n"
            "Q: What is the benefit of pre-order tree serialization?\n"
            "A: It serializes the tree recursively using single bits for nodes, which takes roughly 10 bits per unique character, saving ~85% of header metadata compared to simple tables.\n";
        std::string repeated;
        for (int i = 0; i < 8; ++i) repeated += faq + "\n";
        runBenchmarkCase("Q&A FAQ Document", repeated);
    }
}

void printBenchmarkReport() {
    std::cout << "\n";
    std::cout << "=================================================================================================================\n";
    std::cout << "                                         HUFFMAN COMPRESSION ENGINE BENCHMARK REPORT                                    \n";
    std::cout << "===================================================================================================================\n";
    std::cout << " " << std::left << std::setw(32) << "Benchmark Case Name"
              << std::right << std::setw(15) << "Original (B)"
              << std::setw(16) << "Compressed (B)"
              << std::setw(12) << "Ratio"
              << std::setw(15) << "Space Red."
              << std::setw(15) << "Comp. (ms)"
              << std::setw(15) << "Decomp. (ms)"
              << std::setw(12) << "Integrity" << "\n";
    std::cout << "------------------------------------------------------------------------------------------------------------------------\n";

    uint64_t totalOriginal = 0;
    uint64_t totalCompressed = 0;
    double totalCompTime = 0;
    double totalDecompTime = 0;
    size_t integrityPassedCount = 0;
    
    double sumRatio = 0.0;
    double sumReduction = 0.0;
    
    double maxReduction = -999999.0;
    std::string bestCaseName = "None";
    double minReduction = 999999.0;
    std::string worstCaseName = "None";

    for (const auto& res : benchmarkResults) {
        std::string integrityStr = res.integrityPassed ? "PASSED" : "FAILED";
        
        std::cout << " " << std::left << std::setw(32) << res.testName
                  << std::right << std::setw(15) << res.originalSize
                  << std::setw(16) << res.compressedSize
                  << std::fixed << std::setprecision(2)
                  << std::setw(11) << res.ratio << "x"
                  << std::setw(14) << res.reduction << "%"
                  << std::fixed << std::setprecision(3)
                  << std::setw(14) << res.compressTimeMs << "ms"
                  << std::setw(14) << res.decompressTimeMs << "ms"
                  << std::right << std::setw(12) << integrityStr << "\n";

        totalOriginal += res.originalSize;
        totalCompressed += res.compressedSize;
        sumRatio += res.ratio;
        sumReduction += res.reduction;
        totalCompTime += res.compressTimeMs;
        totalDecompTime += res.decompressTimeMs;
        
        if (res.integrityPassed) integrityPassedCount++;

        if (res.reduction > maxReduction) {
            maxReduction = res.reduction;
            bestCaseName = res.testName;
        }
        if (res.reduction < minReduction) {
            minReduction = res.reduction;
            worstCaseName = res.testName;
        }
    }

    std::cout << "------------------------------------------------------------------------------------------------------------------------\n";
    
    double avgRatio = benchmarkResults.empty() ? 0.0 : (sumRatio / benchmarkResults.size());
    double avgReduction = benchmarkResults.empty() ? 0.0 : (sumReduction / benchmarkResults.size());
    double overallReduction = totalOriginal > 0 ? ((1.0 - (static_cast<double>(totalCompressed) / totalOriginal)) * 100.0) : 0.0;

    std::cout << " " << std::left << std::setw(32) << "Averages & Totals"
              << std::right << std::setw(15) << totalOriginal
              << std::setw(16) << totalCompressed
              << std::fixed << std::setprecision(2)
              << std::setw(11) << avgRatio << "x"
              << std::setw(14) << overallReduction << "%"
              << std::fixed << std::setprecision(3)
              << std::setw(14) << totalCompTime << "ms"
              << std::setw(14) << totalDecompTime << "ms"
              << std::right << std::setw(12) << (std::to_string(integrityPassedCount) + "/" + std::to_string(benchmarkResults.size())) << "\n";
    std::cout << "========================================================================================================================\n";
    
    std::cout << "\n[BENCHMARK METRICS SUMMARY]\n"
              << "  - Total Benchmark Cases:     " << benchmarkResults.size() << "\n"
              << "  - Average Compression Ratio: " << std::fixed << std::setprecision(2) << avgRatio << "x\n"
              << "  - Average Space Reduction:   " << avgReduction << "%\n"
              << "  - Overall Space Reduction:   " << overallReduction << "%\n"
              << "  - Best Compression Case:     " << bestCaseName << " (" << std::fixed << std::setprecision(2) << maxReduction << "% reduction)\n"
              << "  - Worst Compression Case:    " << worstCaseName << " (" << std::fixed << std::setprecision(2) << minReduction << "% reduction)\n"
              << "  - Total Compression Time:    " << totalCompTime << " ms\n"
              << "  - Total Decompression Time:  " << totalDecompTime << " ms\n"
              << "  - Zero Data Loss Verified:   " << (integrityPassedCount == benchmarkResults.size() ? "YES" : "NO") << "\n\n";
}

int main() {
    int passedCount = 0;
    int failedCount = 0;

    std::cout << "========================================================\n";
    std::cout << "           RUNNING HUFFMAN UNIT TESTS                   \n";
    std::cout << "========================================================\n";

    RUN_TEST(testEmptyString);
    RUN_TEST(testSingleCharacter);
    RUN_TEST(testRepeatedCharacter);
    RUN_TEST(testTwoCharacters);
    RUN_TEST(testRepeatedTwoCharacters);
    RUN_TEST(testWhitespaceOnly);
    RUN_TEST(testNumbersOnly);
    RUN_TEST(testSymbolsOnly);
    RUN_TEST(testMixedAlphaNumeric);
    RUN_TEST(testLongParagraph);
    RUN_TEST(testAllASCII);
    RUN_TEST(testBinaryLikePattern);
    RUN_TEST(testHuffmanTreeStructureEmpty);
    RUN_TEST(testHuffmanTreeStructureSingle);
    RUN_TEST(testHuffmanTreeStructureMulti);
    RUN_TEST(testFileMissing);
    RUN_TEST(testFileEmpty);
    RUN_TEST(testCorruptedHeader);
    RUN_TEST(testUnicodeReadyDiscussion);
    RUN_TEST(testLargeTextCompression);
    RUN_TEST(testIntegrityCheck);

    std::cout << "========================================================\n";
    std::cout << "                      UNIT TEST SUMMARY                 \n";
    std::cout << "========================================================\n";
    std::cout << "  Total Run: " << (passedCount + failedCount) << "\n";
    std::cout << "  Passed:    \033[32m" << passedCount << "\033[0m\n";
    
    if (failedCount > 0) {
        std::cout << "  Failed:    \033[31m" << failedCount << "\033[0m\n";
        std::cout << "  Aborting benchmarks due to unit test failures.\n";
        return 1;
    } else {
        std::cout << "  All unit tests passed successfully!\n";
    }

    // Run benchmarks
    runBenchmarks();
    printBenchmarkReport();

    // Verify benchmark integrity
    bool benchPassed = true;
    for (const auto& res : benchmarkResults) {
        if (!res.integrityPassed) {
            benchPassed = false;
            break;
        }
    }

    return benchPassed ? 0 : 1;
}
