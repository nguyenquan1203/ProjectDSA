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

// Định nghĩa độ rộng các cột bảng để thống nhất giữa Console và File
const int W_NAME = 25;
const int W_SIZE = 12;
const int W_PCT  = 10;
const int W_TIME = 10;
const int W_SPD  = 12;
const int W_RAM  = 10;

/**
 * Hàm vẽ nội dung bảng dùng chung cho cả màn hình và file
 */
void internalDrawTable(std::ostream& os, const std::vector<BenchMark>& results) {
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
    for (const auto& b : results) {
        double savingPct = (1.0 - (double)b.compressedSize / b.originalSize) * 100.0;
        if (savingPct < 0) savingPct = 0;
        
        double speed = (b.compTime > 0) ? ((double)b.originalSize / 1048576.0) / (b.compTime / 1000.0) : 0;
        double ramMB = (double)b.peekRamUsage / 1048576.0;

        std::string name = b.fileName;
        if (name.length() > W_NAME - 1) name = name.substr(0, W_NAME - 4) + "...";

        os << std::left << std::setw(W_NAME) << name
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
    os << std::string(115, '=') << "\n";
    os << "Tong cong: " << passCount << "/" << results.size() << " file hoan thanh.\n";
}

/**
 * Hiển thị bảng tóm tắt lên Console
 */
void printSummary(const std::vector<BenchMark>& results) {
    if (results.empty()) return;
    std::cout << "\n" << std::string(115, '=') << "\n";
    std::cout << "                         BANG TONG HOP KET QUA KIEM THU\n";
    std::cout << std::string(115, '=') << "\n";
    internalDrawTable(std::cout, results);
}

/**
 * Ghi báo cáo ra file trong thư mục reports/
 */
void writeReport(const std::vector<BenchMark>& results) {
    std::string dir = "reports";
    if (!fs::exists(dir)) fs::create_directories(dir);

    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    std::stringstream ss;
    ss << dir << "/report_" << (1900 + now->tm_year) 
       << std::setfill('0') << std::setw(2) << (1 + now->tm_mon)
       << std::setw(2) << now->tm_mday << "_" << std::setw(2) << now->tm_hour 
       << std::setw(2) << now->tm_min << ".txt";
    
    std::ofstream f(ss.str());
    if (f.is_open()) {
        f << "BAO CAO CHI TIET HE THONG DEFLATE\n\n";
        internalDrawTable(f, results);
        f.close();
        std::cout << "\n[Luu tru] Da xuat file bao cao tai: " << ss.str() << "\n";
    }
}

/**
 * Hàm chính chạy Batch Test
 */
std::vector<BenchMark> runBatchTest(std::string folderPath,
                                    std::function<void(std::string, std::string)> compress,
                                    std::function<void(std::string, std::string)> decompress) {
    std::vector<BenchMark> results;

    if (!fs::exists(folderPath)) {
        std::cout << "Loi: Khong tim thay thu muc: " << folderPath << "\n";
        return results;
    }

    std::cout << "--- Bat dau tien trinh kiem thu ---\n";

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().filename().string();
            // Chi in dong trang thai don gian de theo doi tien do
            std::cout << "Dang xu ly: " << std::left << std::setw(30) << name << "... " << std::flush;
            
            BenchMark res = runFull(entry.path().string(), compress, decompress);
            results.push_back(res);
            
            std::cout << (res.isPassed ? "[SUCCESS]" : "[FAILED]") << "\n";
        }
    }

    // SAU KHI KET THUC VONG LAP MOI IN BANG VA GHI FILE
    printSummary(results);
    writeReport(results);

    return results;
}