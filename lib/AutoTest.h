#ifndef AutoTest_h
#define AutoTest_h

#include "BenchMark.h"
#include <vector>

std::vector<BenchMark> runBatchTest(std::string folderPath,
                                    std::function<void(std::string, std::string)> compress,
                                    std::function<void(std::string, std::string)> decompress,
                                    bool isComp = false, bool isDecomp = false);

void printSummary(const std::vector<BenchMark> &results);

void writeReport(const std::vector<BenchMark> &results);

#endif