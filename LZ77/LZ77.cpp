#include "LZ77.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
// Hàm nén LZ77
#include <iostream>
std::vector<Token> lz77_compress(
    const std::string &s,
    uint16_t windowSize,
    uint16_t maxMatchLen)
{
    const int n = static_cast<int>(s.size());
    std::vector<Token> tokens;
    std::unordered_map<std::string, std::vector<int>> index;

    int i = 0;
    while (i < n)
    {
        index.clear();
        // Xây dựng chỉ mục cho cửa sổ trượt
        for (int j = std::max(0, i - windowSize); j + 2 < i; j++)
        {
            index[s.substr(j, 3)].push_back(j);
        }

        uint16_t bestLen = 0;
        uint16_t bestdistance = 0;
        uint16_t lookahead = std::min(maxMatchLen, static_cast<uint16_t>(n - i));
        std::string key = s.substr(i, std::min(static_cast<uint16_t>(3), lookahead));

        if (index.count(key))
        {
            for (int pos : index[key])
            {
                int distance = i - pos;
                if (distance > windowSize)
                    continue;

                int len = 0;
                // BỎ ĐIỀU KIỆN: len < (i - pos) ĐỂ HỖ TRỢ OVERLAP
                // Điều này giúp nén chuỗi "aaaaa" thành 1 Match thay vì nhiều Literal
                while (i + len < n &&
                       s[pos + len] == s[i + len] &&
                       len < maxMatchLen)
                {
                    len++;
                }

                if (len > bestLen)
                {
                    bestLen = len;
                    bestdistance = distance;
                }
            }
        }
        // Chỉ tạo Match nếu độ dài đủ lớn (thường >= 3) để thực sự có lợi về dung lượng
        if (bestLen >= 3)
        {
            tokens.push_back(Token{
                true,         // isMatch
                0,            // literal
                bestdistance, // distance
                bestLen       // length
            });
            i += bestLen;
        }
        else
        {
            tokens.push_back(Token{
                false,
                s[i],
                0,
                0});
            i += 1;
        }
    }
    return tokens;
}

// Hàm giải mã LZ77
std::string lz77_decode(const std::vector<Token> &tokens)
{
    std::string result = "";

    for (const auto &t : tokens)
    {
        if (!t.isMatch)
        {
            result += t.literal;
        }
        else
        {
            int currentSize = (int)result.size();

            // KIỂM TRA AN TOÀN: Tránh lỗi Assertion '__pos <= size()'
            // startPos phải >= 0, nghĩa là t.distance không được lớn hơn kích thước hiện tại
            if (t.distance > 0 && t.distance <= currentSize)
            {
                int startPos = currentSize - t.distance;
                for (int i = 0; i < t.length; ++i)
                {
                    // Copy từng ký tự một để xử lý được trường hợp Overlap
                    // Ví dụ: nén "aaaaa" với dist=1, len=4.
                    // Khi copy 'a' thứ 2, nó sẽ lấy 'a' thứ 1 vừa mới được thêm vào kết quả.
                    result += result[startPos + i];
                }
            }
        }
    }
    return result;
}