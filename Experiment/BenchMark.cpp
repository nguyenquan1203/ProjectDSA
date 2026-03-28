#include "BenchMark.h"

long long getFileSize(std::string filePath)
{
    if (!fs::exists(filePath))
        return -1;
    return fs::file_size(filePath);
}

bool compareFile(std::string filePath1, std::string filePath2)
{
    std::ifstream f1(filePath1, std::ios::binary);
    std::ifstream f2(filePath2, std::ios::binary);

    if (!f1.is_open() || !f2.is_open())
        return false;

    f1.seekg(0, std::ios::end);
    f2.seekg(0, std::ios::end);
    if (f1.tellg() != f2.tellg())
        return false;

    f1.seekg(0, std::ios::beg);
    f2.seekg(0, std::ios::beg);

    char b1, b2;
    while (f1.get(b1) && f2.get(b2))
        if (b1 != b2)
            return false;

    return true;
}

clk::time_point getCurrentTime()
{
    return clk::now();
}

double getDurationMs(clk::time_point time2, clk::time_point time1)
{
    std::chrono::duration<double, std::milli> dur = time2 - time1;
    return dur.count();
}

double getRunTime(std::function<void()> func)
{
    clk::time_point t1 = getCurrentTime();
    if (func)
        func();
    clk::time_point t2 = getCurrentTime();
    double dur = getDurationMs(t2, t1);
    return dur;
}

size_t getPeakRSS()
{
    PROCESS_MEMORY_COUNTERS info;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info)))
    {
        return (size_t)info.PeakWorkingSetSize;
    }
    return 0;
}

BenchMark runFull(std::string fileName,
                  std::function<void(std::string, std::string)> compress,
                  std::function<void(std::string, std::string)> decompress,
                  std::string binDir, std::string recDir,
                  bool isComp, bool isDecomp)
{

    BenchMark result;
    fs::path p(fileName);

    std::string file1 = fileName;
    std::string file2 = binDir + "/" + p.stem().string() + ".bin";
    std::string file3 = recDir + "/" + p.stem().string() + "_dec" + p.extension().string();

    if (!fs::exists(binDir))
        fs::create_directories(binDir);
    if (!fs::exists(recDir))
        fs::create_directories(recDir);

    result.fileName = p.filename().string();
    result.originalSize = (size_t)getFileSize(file1);
    result.isPassed = false; // Mặc định là false

    // --- KHỐI 1: QUÁ TRÌNH NÉN ---
    if (isComp)
    {
        auto t1 = clk::now();
        compress(file1, file2);
        auto t2 = clk::now();
        result.compTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        result.compressedSize = (size_t)getFileSize(file2);
    }
    else
    {
        result.compTime = 0;
        result.compressedSize = fs::exists(file2) ? (size_t)getFileSize(file2) : 0;
    }

    result.peekRamUsage = (long long)getPeakRSS();

    // --- KHỐI 2: QUÁ TRÌNH GIẢI NÉN ---
    if (isDecomp && fs::exists(file2))
    {
        auto t3 = clk::now();
        decompress(file2, file3);
        auto t4 = clk::now();
        result.decompTime = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();

        result.isPassed = compareFile(file1, file3);
    }
    else
    {
        result.decompTime = 0;
        if (!isDecomp && isComp)
            result.isPassed = fs::exists(file2);
    }

    return result;
}