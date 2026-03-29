#include <vector>
#include <string>
#include <cstdint>
#include "CommonTypes.h"

std::vector<Token> lz77_compress(const std::string &s, int windowSize = 1024, int maxMatchLen = 1024);

std::string lz77_decode(const std::vector<Token> &tokens);

std::string readFile(const std::string &filename);
