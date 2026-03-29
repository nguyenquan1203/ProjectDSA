#ifndef BitStream_h
#define BitStream_h

#include <fstream>
#include <string>
#include <unordered_map>
#include "CommonTypes.h"
struct OutBitStream
{
    std::ofstream out;
    unsigned char buffer;
    int bitCount;

    // Khởi tạo trạng thái ban đầu
    OutBitStream(const std::string &filename) : buffer(0), bitCount(0)
    {
        out.open(filename, std::ios::binary);
    }
};

struct InBitStream
{
    std::ifstream in;
    unsigned char buffer;
    int bitCount;
    // Khởi tạo trạng thái ban đầu
    InBitStream(const std::string &filename) : buffer(0), bitCount(0)
    {
        in.open(filename, std::ios::binary);
    }
};

void writeBit(OutBitStream &stream, int bit); // Ghi 1 bit

void writeBits(OutBitStream &stream, unsigned int val, int length); // Ghi nhiều bit

void flushBitStream(OutBitStream &stream); // Đẩy bit thừa ra file

void closeOutStream(OutBitStream &stream); // Đóng file ghi an toàn

int readBit(InBitStream &stream); // Đọc 1 bit

int readBits(InBitStream &stream, int length); // Đọc nhiều bit

void closeInStream(InBitStream &stream); // Đóng file

int getSymbol(InBitStream &inStream, Node *root);

std::string getExtension(const std::string &filename);

void writeHeader(OutBitStream &out, const std::unordered_map<int, int> &freqs, size_t totalTokens, const std::string &ext);

std::unordered_map<int, int> readHeader(InBitStream &in, size_t &totalTokens, std::string &ext);

#endif
