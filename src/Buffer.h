#pragma once

#include <string>
#include <vector>

class Buffer
{

private:

    std::string m_fileName;
    std::vector<std::string> m_lines;
    int m_cursorX;
    int m_cursorY;

private:

    bool doesFileExist(const std::string& fileName) const;
    void readFromFile(const std::string& fileName);

public:

    Buffer(const std::string& fileName);

    const std::vector<std::string>& getLines() const { return m_lines ; }

};
