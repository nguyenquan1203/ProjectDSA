#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <string>
#include "Bitstream.h"
std::string readFile(const std::string &filename);
void saveToFile(const std::string &filename, const std::string &data);
void writeHeaderExtension(OutBitStream &out, const std::string &ext);
std::string readHeaderExtension(InBitStream &in);
std::string getBaseName(const std::string &filename);
#endif
