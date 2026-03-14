#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <string>

std::string readFile(const std::string &filename);
void saveToFile(const std::string &filename, const std::string &data);

#endif