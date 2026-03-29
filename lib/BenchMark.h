#ifndef BenchMark_h
#define BenchMark_h

#include <iostream>
#include <string>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <windows.h>
#include <psapi.h>

namespace fs = std::filesystem;
using clk = std::chrono::high_resolution_clock;

struct BenchMark
{
    std::string fileName;
    size_t originalSize;    // Bytes
    size_t compressedSize;  // Bytes
    long long compTime;     // ms
    long long decompTime;   // ms
    long long peakRamUsage; // bytes
    bool isPassed;

    double getSaving() const
    {
        if (originalSize == 0)
            return 0;
        return (1.0 - (double)compressedSize / originalSize) * 100.0;
    }

    double getRatio() const
    {
        if (compressedSize == 0)
            return 0;
        return (double)originalSize / compressedSize;
    }

    double getCompSpeed() const
    {
        if (compTime == 0)
            return 0;
        return (originalSize / (1024.0 * 1024.0)) / (compTime / 1000.0);
    }

    double getDecompSpeed() const
    {
        if (decompTime == 0)
            return 0;
        return (originalSize / (1024.0 * 1024.0)) / (decompTime / 1000.0);
    }
};

long long getFileSize(std::string filePath);

bool compareFile(std::string filePath1, std::string filePath2);

clk::time_point getCurrentTime();

double getDurationMs(clk::time_point time2, clk::time_point time1);

double getRunTime(std::function<void()> func);

size_t getPeakRSS();

BenchMark runFull(std::string fileName,
                  std::function<void(std::string, std::string)> compress,
                  std::function<void(std::string, std::string)> decompress,
                  std::string binDir, std::string recDir,
                  bool isComp, bool isDecomp);

#endif