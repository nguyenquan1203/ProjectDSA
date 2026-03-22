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
                  std::function<void(std::string, std::string)> decompress)
{

    BenchMark result;
    fs::path p(fileName);

    // Khai báo các thư mục đích
    std::string binDir = "compressed_files";
    std::string recDir = "recovered_files";

    // Tạo thư mục nếu chưa tồn tại
    if (!fs::exists(binDir))
        fs::create_directories(binDir);
    if (!fs::exists(recDir))
        fs::create_directories(recDir);

    std::string file1 = fileName;                                                           // File gốc
    std::string file2 = binDir + "/" + p.stem().string() + ".bin";                          // File nén
    std::string file3 = recDir + "/" + p.stem().string() + "_dec" + p.extension().string(); // File giải nén

    result.fileName = p.filename().string();
    result.originalSize = (size_t)getFileSize(file1);

    // Đo quá trình nén
    auto t1 = clk::now();
    compress(file1, file2);
    auto t2 = clk::now();
    result.compTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    result.compressedSize = (size_t)getFileSize(file2);
    result.peekRamUsage = (long long)getPeakRSS();

    // Đo quá trình giải nén
    auto t3 = clk::now();
    decompress(file2, file3);
    auto t4 = clk::now();
    result.decompTime = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();

    // Kiểm tra tính đúng đắn
    result.isPassed = compareFile(file1, file3);

    // Lưu ý: Không xóa file2 (file nén) và file3 (file giải nén)
    // để người dùng có thể kiểm tra trong các folder tương ứng.

    return result;
}