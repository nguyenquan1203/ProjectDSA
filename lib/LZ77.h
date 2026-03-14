#include <vector>
#include <string>
#include <cstdint>
#include "CommonTypes.h"
// struct Token
// {
//     bool isMatch;
//     char literal;
//     uint16_t distance;
//     uint16_t length;
// };
std::vector<Token> lz77_compress(
    const std::string &s,
    uint16_t windowSize = 32768,
    uint16_t maxMatchLen = 256);
std::string lz77_decode(const std::vector<Token> &tokens);
// int max(int a, int b);
// int min(int a, int b);
// std::string substr(const std::string &s, int pos, int len);
