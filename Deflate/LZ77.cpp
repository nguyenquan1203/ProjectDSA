#include "LZ77.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
inline uint32_t hash3(const std::string &s, int i)
{
    uint8_t c1 = (uint8_t)s[i];
    uint8_t c2 = (uint8_t)s[i + 1];
    uint8_t c3 = (uint8_t)s[i + 2];

    // Dịch bit và XOR để tạo mã băm rải đều trong khoảng 0 -> 65535
    return ((c1 << 10) ^ (c2 << 5) ^ c3) & 65535;
}

std::vector<Token> lz77_compress(const std::string &s, int windowSize, int maxMatchLen)
{
    int n = s.size();

    // Sửa thành 65536 để mảng có thể chứa index từ 0 đến 65535
    const int HASH_SIZE = 65536;

    std::vector<int> head(HASH_SIZE, -1);
    std::vector<int> prev(n, -1);

    std::vector<Token> tokens;
    int i = 0;

    while (i < n)
    {
        int bestLen = 0;
        int bestDist = 0;

        if (i + 2 < n)
        {
            uint32_t h = hash3(s, i);
            int candidate = head[h];
            while (candidate != -1)
            {
                int distance = i - candidate;
                if (distance > windowSize)
                    break;

                int len = 0;
                while (i + len < n &&
                       s[candidate + len] == s[i + len] &&
                       len < maxMatchLen)
                {
                    len++;
                }

                if (len > bestLen)
                {
                    bestLen = len;
                    bestDist = distance;
                }

                // Chuyển sang vị trí cũ hơn có cùng mã Hash
                candidate = prev[candidate];
            }

            // Cập nhật Hash Table
            prev[i] = head[h];
            head[h] = i;
        }

        if (bestLen >= 3)
        {
            tokens.push_back({true, 0, bestDist, bestLen});
            i += bestLen;
        }
        else
        {
            tokens.push_back({false, s[i], 0, 0});
            i++;
        }
    }

    return tokens;
}
std::string lz77_decode(const std::vector<Token> &tokens)
{
    std::string result = "";

    size_t expectedSize = 0;
    for (const auto &t : tokens)
    {
        expectedSize += t.isMatch ? t.length : 1;
    }
    result.reserve(expectedSize);

    for (const auto &t : tokens)
    {
        if (!t.isMatch)
        {
            result += t.literal;
        }
        else
        {
            size_t currentSize = result.size();

            if (t.distance > 0 && (size_t)t.distance <= currentSize)
            {
                size_t startPos = currentSize - (size_t)t.distance;
                for (size_t i = 0; i < (size_t)t.length; ++i)
                {
                    // Copy từng ký tự một để xử lý được trường hợp Overlap
                    // Ví dụ: nén "aaaaa" với dist=1, len=4.
                    result += result[startPos + i];
                }
            }
            else
            {
                std::cerr << "[Canh bao] Loi giai ma LZ77: distance ("
                          << t.distance << ") vuot qua du lieu hien tai ("
                          << currentSize << ")!\n";
            }
        }
    }
    return result;
}
