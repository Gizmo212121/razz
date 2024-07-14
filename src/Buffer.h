#pragma once

#include "GapBuffer.h"

class View;

class Buffer
{

private:

    View* m_view;

    std::string m_fileName;
    std::vector<GapBuffer> m_lines;
    int m_cursorX;
    int m_cursorY;
    int m_lastXSinceYMove;

private:

    bool doesFileExist(const std::string& fileName) const;
    void readFromFile(const std::string& fileName);

public:

    Buffer(const std::string& fileName, View* view);

    void moveCursor(int y, int x, bool render = true);

    void shiftCursorX(int x, bool render = true);
    void shiftCursorY(int y, bool render = true);

    void shiftCursorXWithoutGapBuffer(int x, bool render = true);

    void shiftCursorFullRight();
    void shiftCursorFullLeft();
    void shiftCursorFullTop();
    void shiftCursorFullBottom();

    void insertCharacter(char character);
    char removeCharacter(bool cursorHeadingLeft = true);

    // GETTERS
    const std::vector<GapBuffer>& getLines() const { return m_lines ; }
    const GapBuffer& getGapBuffer(int y) const { return m_lines[y]; }
    std::pair<int, int> getCursorPos() const { return std::pair<int, int>(m_cursorY, m_cursorX) ; }
    int cursorXBeforeYMove() const { return m_lastXSinceYMove ; }

};
