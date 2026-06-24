cmake_minimum_required(VERSION 3.12)
project(LosslessTextCompressionEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure include directory search path
include_directories(include)

# Define library source files
set(LIB_SOURCES
    src/HuffmanTree.cpp
    src/Encoder.cpp
    src/Decoder.cpp
    src/FileManager.cpp
    src/HuffmanCompressor.cpp
)

# Create static library for reuse across binary and test suite
add_library(huffman_lib STATIC ${LIB_SOURCES})

# Build the main CLI executable
add_executable(huffman_app src/main.cpp)
target_link_libraries(huffman_app PRIVATE huffman_lib)

# Build the unit tests executable
add_executable(huffman_tests tests/test_huffman.cpp)
target_link_libraries(huffman_tests PRIVATE huffman_lib)
