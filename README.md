# DEFLATE Data Compression Engine


A high-performance C++ compression suite implementing the **DEFLATE** algorithm (LZ77 + Huffman Coding). Designed for the Data Structures and Algorithms (DSA) course to explore the efficiency of hybrid compression on diverse datasets.

## Overview

This engine provides a complete pipeline for file compression and decompression. By combining dictionary-based encoding (LZ77) with entropy-based encoding (Huffman), the system achieves significant reduction in redundancy for large-scale data.

* **Repository:** [nguyenquan1203/ProjectDSA](https://github.com/nguyenquan1203/ProjectDSA)
* **Dataset:** [Download Dataset (Google Drive)](https://drive.google.com/drive/folders/1QzfY9ciPuGCJPWq2nDD5Lfq-7FMsh2tp) — Contains 237 diverse test cases.
* **Core Logic:** Located in `./src` and `./Deflate`.

---

## Build & Run

```bash
# 1. Compile the source code into an executable named 'main'
g++ src/main.cpp Deflate/*.cpp -Ilib -o main

# 2. Run the program
./main
```

## Project Report

*Read this in [Vietnamese](README-vi.md).*

---

### 1. Problem Statement
Data compression is critical for optimizing storage and network transmission. The core challenge is achieving a strict balance between **Compression Ratio**, **Execution Time**, and **Memory (RAM) Consumption**. Furthermore, compression algorithms often struggle with micro-files, where metadata overhead exceeds the actual data. This project benchmarks the DEFLATE algorithm against diverse datasets to identify its strengths, physical limits, and real-world applicability.

---

### 2. Finalized Dataset
We rigorously tested the engine on **237 files** across three distinct categories:
* **High Redundancy (100 files):** OpenStack system logs and container data.
* **Standard Benchmark (27 files):** The Canterbury and Silesia corpus (e.g., `alice29.txt`, `mozilla.tar`, `kennedy.xls`).
* **Edge-case Stress Test (110 files):** Very small source code files (`.cpp`, `.h`, `.py`) to evaluate behavior on micro-data.

---

### 3. Core Algorithms & Data Structures 

* **LZ77 via Hash Table & Linked Lists:** 
  A sliding window search takes $O(N^2)$ time. To optimize this, we built a custom $3$-byte hashing function (`hash3`). This maps $3$-character sequences to a **65536-entry Hash Table**. To handle hash collisions and track older string occurrences, we implemented a linked-list fallback mechanism using `head` and `prev` arrays. This reduces historical match lookups to an average time complexity of near $O(1)$.
* **Dynamic Huffman Coding:** 
  LZ77 output tokens are further compressed using a Huffman tree. We count the frequency of each token and push them into a **Priority Queue (Min-Heap)**. The prefix-free tree is built in $O(K \log K)$ time (where $K$ is the number of unique tokens), allowing us to assign optimal bit-codes to minimize the final payload.
* **Heuristic Fallback (Flag 0/1):** 
  The engine dynamically calculates the predicted `totalBits`. If the compressed size (payload + Huffman tree header) exceeds the original size, the system automatically triggers a **"Raw Storage" fallback (Flag 0)**.
* **Bit-level Manipulation:** 
  Standard C++ I/O streams operate at the byte level. To write variable-length Huffman codes, we engineered custom `InBitStream` and `OutBitStream` classes to manipulate and flush individual bits using bitwise operations.

---

### 4. Experiment Results & Analysis

To evaluate the efficiency of the engine, we define the **Saving Percentage ($S$)** as follows:

$$S = \left( 1 - \frac{\text{Compressed Size}}{\text{Original Size}} \right) \times 100\%$$

#### Performance Summary Table

| Dataset Category | Avg. Save % | Speed Trend | Peak RAM |
| :--- | :--- | :--- | :--- |
| **High Redundancy (Logs)** | **70% - 82%** | ~10.5 MB/s | ~11 MB - 391 MB |
| **Standard (Text/Binaries)**| **35% - 53%** | ~8.4 MB/s | ~15 MB - 636 MB |
| **Micro-files (Source Code)** | **Negative** | < 1.0 MB/s | ~6 MB |

#### The Meaning Behind the Numbers:
* **The Success of Logs:** The 80% saving on OpenStack logs proves the strength of our Hash Table LZ77 implementation. Logs contain highly predictable string patterns (timestamps, status codes). The sliding window efficiently captured these long matches, demonstrating that DEFLATE is exceptional for server backups.
* **Asymmetric Processing:** Across the standard corpus, we observed that **decompression is consistently 2x to 3x faster than compression** (e.g., `webster` compressed in 6449 ms but decompressed in 2777 ms). This happens because compression requires heavy Hash Table lookups and Min-Heap sorting, while decompression only requires simple tree traversal and array indexing.
* **The Micro-file Limit:** Without our heuristic fallback, files under 1KB yielded negative savings (down to -600%). **The meaning:** To decode Huffman, the frequency tree must be embedded in the file header. For micro-files, this metadata header is physically larger than the raw data itself.
* **Memory Bottleneck:** While small files used minimal RAM, processing large files like `mozilla.tar` (51 MB) required up to **636 MB of RAM**. This exposes a memory trade-off: to achieve high speed, we store the entire LZ77 token array in memory before passing it to the Huffman encoder, rather than streaming it in blocks.

---

### 5. Discussion & Engineering Significance
This project extends beyond standard algorithmic implementation, demonstrating several core principles of system design and real-world software engineering:

* **The Space-Time Tradeoff:** Our engine explicitly trades memory footprint for execution speed. By allocating $O(N)$ RAM to store arrays and Hash Tables in-memory, we achieved near $O(1)$ lookups. Understanding this tradeoff is crucial for designing modern, high-throughput systems.
* **Production-Aware Architecture:** A naive compression tool blindly compresses everything it receives. By implementing the `totalBits` heuristic (Flag 0 fallback), our engine demonstrates "production awareness"—recognizing that knowing *when not to compress* is just as critical as knowing *how* to compress. 
* **Web Infrastructure Alignment:** In modern web environments (like CDNs and APIs), data is compressed once on the server and decompressed millions of times by client browsers. The highly asymmetric nature of our engine (computationally heavy to write, ultra-fast to read) makes this specific LZ77+Huffman hybrid the industry standard for HTTP delivery.

---

### 6. Extensions
To evolve this engine we propose the following architectural extensions:

* **$O(1)$ Space Complexity via Chunking:** The current $636$ MB RAM bottleneck can be resolved by implementing Block-Level Streaming. By reading, compressing, and flushing data in independent $64$ KB chunks, the engine's memory footprint becomes $O(1)$ (constant), allowing it to process multi-gigabyte files (e.g., 4K videos or database dumps) even on low-RAM IoT devices.
* **API & IoT Optimization (Static Trees):** To solve the micro-file negative saving issue, we can implement an option to use a pre-calculated, hardcoded Static Huffman Tree. This eliminates the header overhead entirely, making the engine highly efficient for tiny JSON payloads in REST APIs.
* **Multi-threading:** Because independent $64$ KB chunks do not rely on each other's historical data, the LZ77 hashing and Huffman encoding can be parallelized across multiple CPU cores, drastically increasing the MB/s throughput on modern multi-core processors.

## Authors

* **Nguyễn Quân** - [@nguyenquan1203](https://github.com/nguyenquan1203)
* **Phạm Thiện Nhân** - [@Loser-r-6663](https://github.com/Loser-r-6663)
* **Lê Quốc An** - [@anle6329](https://github.com/anle6329)
