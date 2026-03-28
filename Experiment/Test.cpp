#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <filesystem>

#include "../lib/AutoTest.h"
#include "../lib/BenchMark.h"
#include "../lib/Huffman.h"
#include "../lib/LZ77.h"
#include "../lib/Bitstream.h"
#include "../lib/File.h"

namespace fs = std::filesystem;

#define lengthSize 256
#define matchSize 1280

void writeHeaderExtensionOnly(OutBitStream &out, const std::string &ext)
{
    writeBits(out, (int)ext.size(), 8);
    for (char c : ext)
        writeBits(out, (unsigned char)c, 8);
}

std::string readHeaderExtensionOnly(InBitStream &in)
{
    int extLen = readBits(in, 8);
    if (extLen < 0 || extLen > 255)
        return "";
    std::string ext = "";
    for (int i = 0; i < extLen; ++i)
    {
        int c = readBits(in, 8);
        if (c == -1)
            break;
        ext += (char)c;
    }
    return ext;
}

void compressFile(std::string inputName, std::string outputName)
{
    std::string inputData = readFile(inputName);
    if (inputData.empty())
        return;

    std::string ext = fs::path(inputName).extension().string();

    std::vector<Token> tokens = lz77_compress(inputData, 1024, 1024);
    std::unordered_map<int, int> freqs = caculateFreqs(tokens);
    auto p_queue = generateQueue(freqs);
    Node *root = generateTree(p_queue);

    std::unordered_map<int, BitCode> mapCodes;
    if (root)
        generateCode(root, 0, 0, mapCodes);

    size_t payloadBits = 0;
    if (root)
    {
        for (const auto &t : tokens)
        {
            int key = t.isMatch ? (t.length + lengthSize) : (unsigned char)t.literal;
            if (mapCodes.count(key))
            {
                payloadBits += mapCodes[key].length;
            }
            if (t.isMatch && mapCodes.count(t.distance + matchSize))
            {
                payloadBits += mapCodes[t.distance + matchSize].length;
            }
        }
    }

    size_t headerBits = 1 + 1 + ext.size() + 4 + (freqs.size() * 8) + 4;
    size_t totalBits = headerBits + payloadBits;
    size_t originalBits = inputData.size() * 8;

    OutBitStream out(outputName);
    if (totalBits >= originalBits || !root)
    {
        writeBits(out, 0, 8);
        writeHeaderExtensionOnly(out, ext);
        for (unsigned char c : inputData)
            writeBits(out, c, 8);
    }
    else
    {

        writeBits(out, 1, 8);
        writeHeader(out, freqs, tokens.size(), ext);
        for (const auto &t : tokens)
        {
            if (!t.isMatch)
            {
                int v = (unsigned char)t.literal;
                writeBits(out, mapCodes[v].val, mapCodes[v].length);
            }
            else
            {
                int lIdx = t.length + lengthSize;
                int dIdx = t.distance + matchSize;
                writeBits(out, mapCodes[lIdx].val, mapCodes[lIdx].length);
                writeBits(out, mapCodes[dIdx].val, mapCodes[dIdx].length);
            }
        }
    }
    closeOutStream(out);
    if (root)
        deleteTree(root);
}

void decompressFile(std::string inputName, std::string outputName)
{
    InBitStream in(inputName);
    if (!in.in.is_open())
        return;

    int flag = readBits(in, 8);
    if (flag == 0)
    {
        std::string ext = readHeaderExtensionOnly(in);
        std::string data = "";
        int c;
        while ((c = readBits(in, 8)) != -1)
        {
            data += (char)c;
        }
        saveToFile(outputName, data);
    }
    else if (flag == 1)
    {
        size_t totalTokens = 0;
        std::string ext;
        std::unordered_map<int, int> freqs = readHeader(in, totalTokens, ext);

        auto p_queue = generateQueue(freqs);
        Node *root = generateTree(p_queue);
        if (root)
        {
            std::vector<Token> dTokens = decodeTokens(in, root, totalTokens);
            std::string finalData = lz77_decode(dTokens);
            saveToFile(outputName, finalData);
            deleteTree(root);
        }
    }
    closeInStream(in);
}

// int main()
// {

//     std::cout << "====================================================\n";
//     std::cout << "   BENCHMARK SYSTEM: SMART HYBRID DEFLATE           \n";
//     std::cout << "   (LZ77 + Huffman with Automatic Store Logic)      \n";
//     std::cout << "====================================================\n";
//     // 1. Nhập folder cần nén
//     std::string testFolder;
//     std::cout << "[?] Nhap duong dan thu muc test (Mac dinh: test_cases): ";
//     std::getline(std::cin, testFolder);
//     if (testFolder.empty())
//         testFolder = "test_cases";

//     if (!fs::exists(testFolder))
//     {
//         std::cout << "[Loi] Khong tim thay thu muc: " << testFolder << "\n";
//         std::cout << "====================================================\n";
//         std::cout << "Nhan phim bat ky de thoat...";
//         std::cin.ignore(1000, '\n');
//         std::cin.get();
//         return 1;
//     }

//     // 2. Lựa chọn chế độ Nén (isComp)
//     char cChoice, dChoice;
//     bool isComp = true;
//     bool isDecomp = true;

//     std::cout << "[?] Ban co muon thuc hien NEN khong? (y/n): ";
//     std::cin >> cChoice;
//     isComp = (cChoice == 'y' || cChoice == 'Y');

//     // 3. Lựa chọn chế độ Giải nén (isDecomp)
//     std::cout << "[?] Ban co muon thuc hien GIAI NEN khong? (y/n): ";
//     std::cin >> dChoice;
//     isDecomp = (dChoice == 'y' || dChoice == 'Y');

//     // Kiểm tra nếu không chọn gì cả
//     if (!isComp && !isDecomp)
//     {
//         std::cout << "[!] Ban khong chon bat ky tac vu nao. Ket thuc.\n";
//         std::cout << "====================================================\n";
//         std::cout << "Nhan phim bat ky de thoat...";
//         std::cin.ignore(1000, '\n');
//         std::cin.get();
//         return 0;
//     }

//     std::cout << "\n--- DANG KHOI TAO TIEN TRINH ---\n";
//     std::cout << "> Che do Nen:      [" << (isComp ? "BAT" : "TAT") << "]\n";
//     std::cout << "> Che do Giai nen: [" << (isDecomp ? "BAT" : "TAT") << "]\n";
//     std::cout << "----------------------------------------------------\n";

//     std::vector<BenchMark> results = runBatchTest(
//         testFolder,
//         compressFile,
//         decompressFile,
//         isComp,
//         isDecomp);

//     // 5. Kết xuất kết quả
//     if (!results.empty())
//     {
//         std::cout << "\n[OK] Kiem thu hoan tat. Xem chi tiet trong file report.txt\n";
//     }

//     std::cout << "====================================================\n";
//     std::cout << "Nhan phim bat ky de thoat...";
//     std::cin.ignore(1000, '\n');
//     std::cin.get();
//     return 0;
// }