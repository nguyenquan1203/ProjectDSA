#ifndef Huffman_h
#define Huffman_h
#include "Bitstream.h"
#include <string>
#include <unordered_map>
#include <queue>
#include "CommonTypes.h"

struct BitCode
{
    unsigned int val;
    unsigned int length; // 12 bit
};

struct cmp
{
    bool operator()(Node *l, Node *r)
    {
        // Nếu tần suất bằng nhau, so sánh theo giá trị để đảm bảo cây luôn duy nhất
        if (l->freq == r->freq)
        {
            return l->value > r->value;
        }
        return l->freq > r->freq;
    }
};
std::unordered_map<int, int> caculateFreqs(const std::vector<Token> &tokens); // đếm tần số

std::priority_queue<Node *, std::vector<Node *>, cmp> generateQueue(std::unordered_map<int, int> &freqs); // tạo hàng đợi

Node *generateTree(std::priority_queue<Node *, std::vector<Node *>, cmp> &p_queue); // tạo cây

void generateCode(Node *root, unsigned int curCode, unsigned char curLen, std::unordered_map<int, BitCode> &mapCodes); // tạo Mapcode

std::vector<Token> decodeTokens(InBitStream &inStream, Node *root, size_t totalTokens);
void deleteTree(Node *root); // xóa cây

#endif