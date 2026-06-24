# Lossless Text Compression Engine using (C++)
Lossless Text Compression Engine built with C++ and Huffman Coding. Supports efficient text file compression/decompression, binary bit-packing, compression statistics, data integrity validation, and a responsive web interface for real-time compression analysis.

---

## Project Architecture Diagram
The system uses a clean modular architecture. The abstract interface `Compressor` defines the contract, allowing easy extensions for future formats (such as PDF, binary, or image compressors) without altering the main orchestration flow.

##  System Architecture

```mermaid
graph TD

    A[Web Interface<br>HTML/CSS/JavaScript] --> B[Node.js Server]
    B --> C[HuffmanCompressor]

    C --> D[FileManager]
    C --> E[HuffmanTree]
    C --> F[Encoder]
    C --> G[Decoder]

    E --> H[Node Structure]

    D --> I[Input Files]
    D --> J[Compressed .huff Files]
    D --> K[Decompressed Text Files]

    F --> L[Bitstream Generation]
    G --> M[Text Reconstruction]

    classDef component fill:#2563eb,color:#fff,stroke:#1e40af,stroke-width:2px;
    classDef interface fill:#10b981,color:#fff,stroke:#047857,stroke-width:2px;
    classDef storage fill:#f59e0b,color:#fff,stroke:#b45309,stroke-width:2px;

    class C,E,F,G,D component;
    class H interface;
    class I,J,K storage;
```

### Compression Workflow

```mermaid
graph LR

    A[Input Text] --> B[Frequency Analysis]
    B --> C[Min Heap Construction]
    C --> D[Huffman Tree]
    D --> E[Code Generation]
    E --> F[Bit Encoding]
    F --> G[Compressed .huff File]
```

### Decompression Workflow

```mermaid
graph LR

    A[Compressed .huff File]
    --> B[Read Metadata]
    --> C[Reconstruct Huffman Tree]
    --> D[Decode Bitstream]
    --> E[Restore Original Text]
    --> F[Integrity Validation]
```

### Class Relationships

```mermaid
classDiagram

    class Compressor{
        <<interface>>
        +compress()
        +decompress()
    }

    class HuffmanCompressor
    class HuffmanTree
    class Encoder
    class Decoder
    class FileManager
    class Node

    Compressor <|-- HuffmanCompressor

    HuffmanCompressor --> HuffmanTree
    HuffmanCompressor --> Encoder
    HuffmanCompressor --> Decoder
    HuffmanCompressor --> FileManager

    HuffmanTree --> Node
```
