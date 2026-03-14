#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>

#include "lib/Huffman.h"
#include "lib/LZ77.h"
#include "lib/Bitstream.h"
#include "lib/File.h"
std::string getBaseName(const std::string &filename)
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos)
        return filename;
    return filename.substr(0, lastdot);
}

void compressFile()
{
    // 1. Đọc dữ liệu đầu vào
    std::string inputName;
    std::cout << "Nhap ten file can nen ";
    std::cin >> inputName;

    std::string base = getBaseName(inputName);
    std::string outputName = base + "_compressed.bin";
    std::string inputData = readFile(inputName);
    if (inputData.empty())
    {
        std::cerr << "Loi: File dau vao trong hoac khong ton tai.\n";
        return;
    }

    std::vector<Token> tokens = lz77_compress(inputData, 1024, 1024);

    std::unordered_map<int, int> freqs = caculateFreqs(tokens);
    auto p_queue = generateQueue(freqs);
    Node *root = generateTree(p_queue);

    std::unordered_map<int, BitCode> mapCodes;
    if (root != nullptr)
        generateCode(root, 0, 0, mapCodes);
    OutBitStream outStream(outputName);
    writeHeader(outStream, freqs, tokens.size());

    for (const auto &t : tokens)
    {
        if (!t.isMatch)
        {
            int val = (int)(unsigned char)t.literal;
            writeBits(outStream, mapCodes[val].val, mapCodes[val].length);
        }
        else
        {
            int lenVal = t.length + lengthSize;
            writeBits(outStream, mapCodes[lenVal].val, mapCodes[lenVal].length);

            int distVal = t.distance + matchSize;
            writeBits(outStream, mapCodes[distVal].val, mapCodes[distVal].length);
        }
    }
    closeOutStream(outStream);
    deleteTree(root);
    std::cout << "Nen thanh cong! Kich thuoc Token: " << tokens.size() << "\n";
}
void decompressFile()
{
    std::string inputName;
    std::cout << "Nhap ten file nen (.bin): ";
    std::cin >> inputName;

    std::string outputName = getBaseName(inputName);

    size_t pos = outputName.find("_compressed");
    if (pos != std::string::npos)
        outputName = outputName.substr(0, pos);
    outputName += "_recovered.txt";
    InBitStream inStream(inputName);
    if (!inStream.in.is_open())
    {
        std::cerr << "Loi: Khong the mo file nen!\n";
        return;
    }
    size_t totalTokens = 0;
    std::unordered_map<int, int> freqs = readHeader(inStream, totalTokens);

    auto p_queue = generateQueue(freqs);
    Node *root = generateTree(p_queue);

    std::vector<Token> decodedTokens = decodeTokens(inStream, root, totalTokens);

    std::string finalData = lz77_decode(decodedTokens);

    saveToFile(outputName, finalData);
    deleteTree(root);
    closeInStream(inStream);

    std::cout << "Giai nen thanh cong! File ket qua: " << outputName << "\n";
}

int main()
{
    int choice;
    std::cout << "\n===== HE THONG NEN DEFLATE =====\n";
    std::cout << "1. Nen file\n";
    std::cout << "2. Giai nen file\n";
    std::cout << "Lua chon: ";
    std::cin >> choice;

    if (choice == 1)
        compressFile();
    else if (choice == 2)
        decompressFile();
    system("pause");
    return 0;
}