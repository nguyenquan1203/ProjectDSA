#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include "File.h"
std::string readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        std::cerr << "Loi: Khong the mo file " << filename << std::endl;
        return "";
    }

    // Lấy kích thước file và reset con trỏ về đầu file
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Tạo chuỗi có kích thước vừa đủ và đọc dữ liệu
    std::string buffer(size, '\0');
    if (file.read(&buffer[0], size))
    {
        return buffer;
    }

    return "";
}
void saveToFile(const std::string &filename, const std::string &data)
{
    std::ofstream out(filename, std::ios::binary);
    if (out.is_open())
    {
        out.write(data.c_str(), data.size());
        out.close();
    }
}