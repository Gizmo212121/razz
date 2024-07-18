#pragma once

#include "FileGapBuffer.h"

class View;

class Buffer
{

private:

    View* m_view;

    std::string m_fileName;

    FileGapBuffer m_file;

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

    void insertCharacter(char character, bool render = true);
    char removeCharacter(bool cursorHeadingLeft = true, bool render = true);
    char replaceCharacter(char character);

    void insertLine(bool down, bool render = true);
    std::shared_ptr<LineGapBuffer> deleteLine(bool render = true);

    // GETTERS
    const FileGapBuffer& getFileGapBuffer() const { return m_file ; }
    const std::shared_ptr<LineGapBuffer>& getLineGapBuffer(int y) const { return m_file[y]; }
    std::pair<int, int> getCursorPos() const { return std::pair<int, int>(m_cursorY, m_cursorX) ; }
    int cursorXBeforeYMove() const { return m_lastXSinceYMove ; }

};
