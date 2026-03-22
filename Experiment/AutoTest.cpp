#include "AutoTest.h"
#include <iomanip>
#include <map>

const int W_NAME = 25;
const int W_SIZE = 12;
const int W_PCT  = 10;
const int W_TIME = 10;
const int W_SPD  = 12;
const int W_RAM  = 10;

std::vector<BenchMark> runBatchTest(std::string folderPath,
                                    std::function<void(std::string, std::string)> compress,
                                    std::function<void(std::string, std::string)> decompress)
{
    std::vector<BenchMark> results;

    if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
    {
        std::cout << "Loi: Thu muc khong ton tai! \n";
        return results;
    }

    std::cout << "--- Bat dau chay test tu dong ---\n";

    for (const auto &entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            std::string ext = entry.path().extension().string();
            std::string fileName = entry.path().filename().string();

            // Bỏ qua các file kết quả trung gian để tránh vòng lặp vô hạn
            if (ext == ".bin" || fileName.find("_rescovered") != std::string::npos)
            {
                continue;
            }

            std::string curFile = entry.path().string();
            BenchMark res = runFull(curFile, compress, decompress);
            results.push_back(res);

            std::cout << "[Checking] " << fileName << " -> " << (res.isPassed ? "PASS" : "FAIL") << "\n";
        }
    }
    return results;
}

void writeReport(std::ostream &os, const std::vector<BenchMark> &results)
{
    if (results.empty())
        return;

    const int W_NAME = 25;
    const int W_SIZE = 12;
    const int W_PCT = 10;
    const int W_RAT = 8;
    const int W_SPD = 12;
    const int W_RAM = 10;
    const int W_RES = 10;

    os << "\n"
       << std::string(130, '=') << "\n";
    os << std::left << std::setw(W_NAME) << "File Name"
       << std::right << std::setw(W_SIZE) << "Original(B)"
       << std::right << std::setw(W_SIZE) << "Compressed(B)"
       << std::right << std::setw(W_PCT) << "Saving(%)"
       << std::right << std::setw(W_RAT) << "Ratio"
       << std::right << std::setw(W_SPD) << "Cp(MB/s)"
       << std::right << std::setw(W_SPD) << "Dc(MB/s)"
       << std::right << std::setw(W_RAM) << "RAM(MB)"
       << std::right << std::setw(W_RES) << "Result" << "\n";
    os << std::string(130, '-') << "\n";

    double sumSaving = 0, sumRatio = 0, sumCompS = 0, sumDecompS = 0, sumRAM = 0;
    int passCount = 0;

    for (const auto &r : results)
    {
        if (r.isPassed)
            passCount++;

        double saving = r.getSaving();
        double ratio = r.getRatio();
        double cSpd = r.getCompSpeed();
        double dSpd = r.getDecompSpeed();
        double ramMB = (double)r.peekRamUsage / (1024.0 * 1024.0);

        sumSaving += saving;
        sumRatio += ratio;
        sumCompS += cSpd;
        sumDecompS += dSpd;
        sumRAM += ramMB;

        std::string name = r.fileName;
        if (name.length() > W_NAME - 3)
            name = name.substr(0, W_NAME - 6) + "...";

        os << std::left << std::setw(W_NAME) << name
           << std::right << std::setw(W_SIZE) << r.originalSize
           << std::right << std::setw(W_SIZE) << r.compressedSize
           << std::right << std::fixed << std::setprecision(2) << std::setw(W_PCT) << saving
           << std::right << std::fixed << std::setprecision(2) << std::setw(W_RAT) << ratio
           << std::right << std::fixed << std::setprecision(2) << std::setw(W_SPD) << cSpd
           << std::right << std::fixed << std::setprecision(2) << std::setw(W_SPD) << dSpd
           << std::right << std::fixed << std::setprecision(2) << std::setw(W_RAM) << ramMB
           << std::right << std::setw(W_RES) << (r.isPassed ? "PASSED" : "FAILED") << "\n";
    }

    double n = (double)results.size();
    os << std::string(130, '=') << "\n";
    os << "\n"
       << std::setw(40) << "" << ">>> GLOBAL AVERAGE STATISTICS <<<\n\n";

    auto printRow = [&](std::string m, double v, std::string d)
    {
        os << std::left << std::setw(30) << m
           << std::fixed << std::setprecision(2) << std::setw(20) << v << d << "\n";
    };

    printRow("Average Saving", sumSaving / n, "%");
    printRow("Average Compression Ratio", sumRatio / n, "x");
    printRow("Average Comp. Speed", sumCompS / n, "MB/s");
    printRow("Average Decomp. Speed", sumDecompS / n, "MB/s");
    printRow("Average Peak RAM Usage", sumRAM / n, "MB");

    os << std::string(60, '=') << "\n";
    double successRate = (passCount / n) * 100.0;
    os << "Overall Result: " << passCount << "/" << results.size()
       << " files passed (" << std::fixed << std::setprecision(2) << successRate << "%).\n";
    os << std::string(130, '=') << "\n";
}

/**
 * Hàm runFull: Thực hiện benchmark và lưu file vào các thư mục riêng biệt
 */


/**
 * Hàm printSummary: Tạo file log riêng biệt trong thư mục "reports"
 */
void printSummary(const std::vector<BenchMark> &results)
{
    if (results.empty()) {
        std::cout << "Khong co du lieu ket qua de hien thi.\n";
        return;
    }

    std::cout << "\n" << std::string(115, '=') << "\n";
    std::cout << std::left << std::setw(W_NAME) << "Ten File"
              << std::right << std::setw(W_SIZE) << "Goc(B)"
              << std::setw(W_SIZE) << "Nen(B)"
              << std::setw(W_PCT) << "Save(%)"
              << std::setw(W_TIME) << "T.Nen(ms)"
              << std::setw(W_TIME) << "T.Giai(ms)"
              << std::setw(W_SPD) << "Speed(MB/s)"
              << std::setw(W_RAM) << "RAM(MB)"
              << "  Ket qua\n";
    std::cout << std::string(115, '-') << "\n";

    int passCount = 0;
    for (const auto &b : results)
    {
        std::string shortName = fs::path(b.fileName).filename().string();
        if (shortName.length() > (W_NAME - 1))
            shortName = shortName.substr(0, W_NAME - 4) + "...";

        // Tính toán các thông số dựa trên các hàm getter trong BenchMark
        double savingPct = (1.0 - (double)b.compressedSize / b.originalSize) * 100.0;
        if (savingPct < 0) savingPct = 0; // Logic nén thông minh đảm bảo không âm

        // Tính tốc độ nén (MB/s)
        double speed = 0;
        if (b.compTime > 0) {
            speed = ((double)b.originalSize / 1048576.0) / (b.compTime / 1000.0);
        }

        double ramMB = b.peekRamUsage / 1048576.0;

        std::cout << std::left << std::setw(W_NAME) << shortName
                  << std::right << std::setw(W_SIZE) << b.originalSize
                  << std::setw(W_SIZE) << b.compressedSize
                  << std::fixed << std::setprecision(2)
                  << std::setw(W_PCT) << savingPct
                  << std::setw(W_TIME) << (int)b.compTime
                  << std::setw(W_TIME) << (int)b.decompTime
                  << std::setw(W_SPD) << speed
                  << std::setw(W_RAM) << ramMB
                  << "  " << (b.isPassed ? "[PASS]" : "[FAIL]") << "\n";

        if (b.isPassed) passCount++;
    }

    std::cout << std::string(115, '=') << "\n";
    std::cout << "Tong cong: " << passCount << "/" << results.size() << " file vuot qua kiem tra.\n";
    std::cout << std::string(115, '=') << "\n\n";
}