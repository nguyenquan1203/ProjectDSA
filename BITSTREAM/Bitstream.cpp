#include "BitStream.h"
#include <unordered_map>
void writeBit(OutBitStream &stream, int bit) // Ghi bit lẻ
{
    if (bit == 1)
    {
        stream.buffer = stream.buffer | (1 << (7 - stream.bitCount));
    }

    stream.bitCount++;

    if (stream.bitCount == 8)
    {
        stream.out.put(stream.buffer);
        stream.buffer = 0;
        stream.bitCount = 0;
    }
}

void writeBits(OutBitStream &stream, unsigned int val, int length) // Ghi nguyên cụm bit vào file
{
    for (int i = length - 1; i >= 0; --i)
    {
        int bit = (val >> i) & 1;
        writeBit(stream, bit);
    }
}

void flushBitStream(OutBitStream &stream) // Dọn sạch buffer
{
    if (stream.bitCount > 0)
    {
        stream.out.put(stream.buffer);
        stream.buffer = 0;
        stream.bitCount = 0;
    }
}

void closeOutStream(OutBitStream &stream)
{
    flushBitStream(stream); // Phải đẩy nốt bit dư trước khi đóng
    if (stream.out.is_open())
    {
        stream.out.close();
    }
}

int readBit(InBitStream &stream)
{
    if (stream.bitCount == 0)
    {
        if (!stream.in.get((char &)stream.buffer))
        {
            return -1; // Kết thúc file
        }
        stream.bitCount = 8;
    }

    unsigned int bit = (stream.buffer >> (stream.bitCount - 1)) & 1;
    stream.bitCount--;

    return bit;
}

unsigned int readBits(InBitStream &stream, int length) // Đọc chuỗi bit
{
    unsigned int val = 0;
    for (int i = 0; i < length; ++i)
    {
        int bit = readBit(stream);
        if (bit == -1)
            break;
        val = (val << 1) | bit;
    }
    return val;
}
int getSymbol(InBitStream &inStream, Node *root)
{
    if (root == nullptr)
        return -1;

    Node *curr = root;
    if (curr->left == nullptr && curr->right == nullptr)
    {

        readBit(inStream);
        return curr->value;
    }

    // Duyệt cây bình thường cho đến khi chạm lá
    while (curr->left != nullptr && curr->right != nullptr)
    {
        int bit = readBit(inStream);
        if (bit == 0)
            curr = curr->left;
        else if (bit == 1)
            curr = curr->right;
        else
            return -1;
    }
    return curr->value;
}
void writeHeader(OutBitStream &out, const std::unordered_map<int, int> &freqs, size_t totalTokens)
{
    // 1. Ghi số lượng phần tử của bảng tần suất (Dùng 32 bit cho an toàn)
    writeBits(out, freqs.size(), 32);

    // 2. Ghi từng cặp (Giá trị, Tần suất)
    for (auto const &[v, f] : freqs)
    {

        writeBits(out, v, 32);
        writeBits(out, f, 32);
    }
    writeBits(out, totalTokens, 32);
}
std::unordered_map<int, int> readHeader(InBitStream &in, size_t &totalTokens)
{
    std::unordered_map<int, int> freqs;

    size_t mapSize = readBits(in, 32);
    for (size_t i = 0; i < mapSize; ++i)
    {
        int v = readBits(in, 32);
        int f = readBits(in, 32);
        freqs[v] = f;
    }

    totalTokens = readBits(in, 32);
    return freqs;
}

void closeInStream(InBitStream &stream)
{
    if (stream.in.is_open())
    {
        stream.in.close();
    }
}