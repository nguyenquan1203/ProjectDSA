#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../lib/Huffman.h"
#include "../lib/LZ77.h"
#include "../lib/Bitstream.h"
#include "../lib/File.h"

#define lengthSize 256
#define matchSize 1280
void compress(std::string inputName)
{
    // Đọc dữ liệu
    std::string inputData = readFile(inputName);
    if (inputData.empty())
        return;
    std::string outputName = getBaseName(inputName) + ".bin";

    std::string ext = getExtension(inputName);

    std::vector<Token> tokens = lz77_compress(inputData, 1024, 1024);
    std::unordered_map<int, int> freqs = caculateFreqs(tokens);
    auto p_queue = generateQueue(freqs);
    Node *root = generateTree(p_queue);

    std::unordered_map<int, BitCode> mapCodes;
    if (root)
        generateCode(root, 0, 0, mapCodes);

    // Tính toán kích thước dự kiến (đơn vị: bit)
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
    if (totalBits > originalBits || !root)
    {
        // CỜ 0: LƯU THÔ
        writeBits(out, 0, 8);
        writeHeaderExtension(out, ext);
        for (unsigned char c : inputData)
            writeBits(out, c, 8);
    }
    else
    {
        // CỜ 1: NÉN
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

void decompress(std::string inputName)
{
    InBitStream in(inputName);
    if (!in.in.is_open())
        return;

    int flag = readBits(in, 8);
    std::string outputName = getBaseName(inputName) + "_recovered";
    if (flag == 0)
    {
        // Giải nén cho file không nén (Lưu thô)
        std::string ext = readHeaderExtension(in);
        std::string data = "";
        int c;
        while ((c = readBits(in, 8)) != -1)
        {
            data += (char)c;
        }
        outputName += ext;
        saveToFile(outputName, data);
    }
    else if (flag == 1)
    {
        // Giải nén cho file có nén
        size_t totalTokens = 0;
        std::string ext;
        std::unordered_map<int, int> freqs = readHeader(in, totalTokens, ext);

        auto p_queue = generateQueue(freqs);
        Node *root = generateTree(p_queue);
        if (root)
        {
            std::vector<Token> dTokens = decodeTokens(in, root, totalTokens);
            std::string finalData = lz77_decode(dTokens);
            outputName += ext;
            saveToFile(outputName, finalData);
            deleteTree(root);
        }
    }
    closeInStream(in);
}

int main()
{
    std::string inputFile, outputFile;
    int option;
    std::cout << "Nhap ten file muon nen hoac giai nen: ";
    std::cin >> inputFile;
    std::cout << "Nhap lua chon cua ban (1 la nen, 2 la giai nen) ";
    std::cin >> option;
    if (option == 1)
    {
        compress(inputFile);
    }
    else
    {
        decompress(inputFile);
    }
    system("pause");
    return 0;
}