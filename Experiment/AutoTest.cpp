#include "../lib/AutoTest.h"
#include <iomanip>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <ctime>
#include <sstream>

namespace fs = std::filesystem;

const int W_NAME = 25;
const int W_SIZE = 12;
const int W_PCT = 10;
const int W_TIME = 10;
const int W_SPD = 12;
const int W_RAM = 10;

void internalDrawTable(std::ostream &os, const std::vector<BenchMark> &results)
{
    os << std::left << std::setw(W_NAME) << "Ten File"
       << std::right << std::setw(W_SIZE) << "Goc(B)"
       << std::setw(W_SIZE) << "Nen(B)"
       << std::setw(W_PCT) << "Save(%)"
       << std::setw(W_TIME) << "T.Nen(ms)"
       << std::setw(W_TIME) << "T.Giai(ms)"
       << std::setw(W_SPD) << "Speed(MB/s)"
       << std::setw(W_RAM) << "RAM(MB)"
       << "  Ket qua\n";
    os << std::string(115, '-') << "\n";

    int passCount = 0;
    for (const auto &b : results)
    {

        double speed = (b.compTime > 0) ? ((double)b.originalSize / 1048576.0) / (b.compTime / 1000.0) : 0;
        double ramMB = (double)b.peakRamUsage / 1048576.0;

        std::string name = b.fileName;
        if (name.length() > W_NAME - 1)
            name = name.substr(0, W_NAME - 4) + "...";

        os << std::left << std::setw(W_NAME) << name
           << std::right << std::setw(W_SIZE) << b.originalSize
           << std::setw(W_SIZE) << b.compressedSize
           << std::fixed << std::setprecision(2)
           << std::setw(W_PCT) << b.getSaving()
           << std::setw(W_TIME) << b.compTime
           << std::setw(W_TIME) << b.decompTime
           << std::setw(W_SPD) << speed
           << std::setw(W_RAM) << ramMB
           << "  " << (b.isPassed ? "[PASS]" : "[FAIL]") << "\n";

        if (b.isPassed)
            passCount++;
    }
    os << std::string(115, '=') << "\n";
    os << "Tong cong: " << passCount << "/" << results.size() << " file hoan thanh.\n";
}

void printSummary(const std::vector<BenchMark> &results)
{
    if (results.empty())
        return;
    std::cout << "\n"
              << std::string(115, '=') << "\n";
    std::cout << "                         BANG TONG HOP KET QUA KIEM THU\n";
    std::cout << std::string(115, '=') << "\n";
    internalDrawTable(std::cout, results);
}

void writeCSV(const std::vector<BenchMark> &results)
{
    std::ofstream f("data_chart.csv");

    if (!f.is_open())
    {
        return;
    }

    f << "FileName,OriginalSize_KB,CompressedSize_KB,SavePercent,CompTime_ms,DecompTime_ms,RAM_MB\n";

    f << std::fixed << std::setprecision(3);
    for (const auto &res : results)
    {
        double orgKB = (double)res.originalSize / 1024.0;
        double compKB = (double)res.compressedSize / 1024.0;
        double ramMB = (double)res.peakRamUsage / (1024.0 * 1024.0);

        f << res.fileName << ","
          << orgKB << ","
          << compKB << ","
          << res.getSaving() << ","
          << res.compTime << "," // Đã là số thực ms
          << res.decompTime << ","
          << ramMB << "\n";
    }
    f.close();
}

void writeReport(const std::vector<BenchMark> &results)
{
    std::string dir = "reports";
    if (!fs::exists(dir))
        fs::create_directories(dir);

    std::time_t t = std::time(nullptr);
    std::tm *now = std::localtime(&t);
    std::stringstream ss;
    ss << dir << "/report_" << (1900 + now->tm_year)
       << std::setfill('0') << std::setw(2) << (1 + now->tm_mon)
       << std::setw(2) << now->tm_mday << "_" << std::setw(2) << now->tm_hour
       << std::setw(2) << now->tm_min << ".txt";

    std::ofstream f(ss.str());
    if (f.is_open())
    {
        f << "BAO CAO CHI TIET HE THONG DEFLATE\n\n";
        internalDrawTable(f, results);
        f.close();
        std::cout << "\n[Luu tru] Da xuat file bao cao tai: " << ss.str() << "\n";
    }
}

std::vector<BenchMark> runBatchTest(std::string folderPath,
                                    std::function<void(std::string, std::string)> compress,
                                    std::function<void(std::string, std::string)> decompress,
                                    bool isComp, bool isDecomp)
{

    std::vector<BenchMark> results;
    if (!fs::exists(folderPath))
    {
        std::cout << "Loi: Khong tim thay thu muc: " << folderPath << "\n";
        return results;
    }
    std::string folderName = fs::path(folderPath).filename().string();
    if (folderName.empty())
        folderName = "default";

    std::string binDir = "compressed_files/" + folderName;
    std::string recDir = "recovered_files/" + folderName;
    if (fs::exists(binDir))
        fs::remove_all(binDir);
    if (fs::exists(recDir))
        fs::remove_all(recDir);
    fs::create_directories(binDir);
    fs::create_directories(recDir);
    std::cout << "--- Batch Test: " << folderName << " ---\n";
    std::cout << "Options: Compress=" << (isComp ? "ON" : "OFF")
              << ", Decompress=" << (isDecomp ? "ON" : "OFF") << "\n";

    for (const auto &entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            std::string name = entry.path().filename().string();
            if (entry.path().extension() == ".bin" || name.find("_dec") != std::string::npos)
                continue;

            std::cout << "Processing: " << std::left << std::setw(25) << name << "... " << std::flush;

            BenchMark res = runFull(entry.path().string(), compress, decompress,
                                    binDir, recDir, isComp, isDecomp);
            results.push_back(res);

            std::cout << (res.isPassed ? "[SUCCESS]" : "[FAIL]") << "\n";
        }
    }
    writeCSV(results);
    bool isSummary;
    char select;
    std::cout << "[?] Ban co muon hien bang tong ket khong? (y/n): ";
    std::cin >> select;
    isSummary = (select == 'y' || select == 'Y');
    if (isSummary)
        printSummary(results);
    writeReport(results);
    if (!isComp && fs::exists(binDir))
    {
        fs::remove_all(binDir);
    }
    if (!isDecomp && fs::exists(recDir))
    {
        fs::remove_all(recDir);
    }
    return results;
}