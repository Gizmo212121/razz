#include "Buffer.h"

#include <cstdio>
#include <fstream>

Buffer::Buffer(const std::string& fileName)
    : m_fileName(fileName)
{
    if (doesFileExist(fileName))
    {
        readFromFile(fileName);
    }
}

bool Buffer::doesFileExist(const std::string& fileName) const
{
    std::ifstream infile(fileName);
    return infile.good();
}

void Buffer::readFromFile(const std::string& fileName)
{
    std::ifstream infile(fileName);

    std::string line;

    while (getline(infile, line))
    {
        m_lines.push_back(line);
    }
}
