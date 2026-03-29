#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include "../lib/File.h"

std::string getBaseName(const std::string &filename)
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos)
        return filename;
    return filename.substr(0, lastdot);
}
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
void writeHeaderExtension(OutBitStream &out, const std::string &ext)
{
    // Lưu đuôi file
    writeBits(out, (int)ext.size(), 8);
    for (char c : ext)
        writeBits(out, (unsigned char)c, 8);
}

std::string readHeaderExtension(InBitStream &in)
{
    // Đọc đuôi file
    int extLen = readBits(in, 8);
    if (extLen < 0 || extLen > 255)
        return "";
    std::string ext = "";
    for (int i = 0; i < extLen; ++i)
    {
        int c = readBits(in, 8);
        if (c == -1)
            break;
        ext += (char)c;
    }
    return ext;
}
