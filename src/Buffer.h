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
    int m_lastXSinceYMove;

private:

    bool doesFileExist(const std::string& fileName) const;
    void readFromFile(const std::string& fileName);

public:

    Buffer(const std::string& fileName);

    void shiftCursorX(int x);
    void shiftCursorY(int y);
    void shiftCursorFullRight();
    void shiftCursorFullLeft();
    void shiftCursorFullTop();
    void shiftCursorFullBottom();

    // GETTERS
    const std::vector<std::string>& getLines() const { return m_lines ; }
    std::pair<int, int> getCursorPos() const { return std::pair<int, int>(m_cursorY, m_cursorX) ; }
    int cursorXBeforeYMove() const { return m_lastXSinceYMove ; }

};
