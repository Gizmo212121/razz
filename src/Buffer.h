#pragma once

#include "FileGapBuffer.h"

class View;

const int WHITESPACE_PER_TAB = 4;

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

    void moveCursor(int y, int x);

    void shiftCursorX(int x);
    void shiftCursorY(int y);

    void shiftCursorXWithoutGapBuffer(int x);

    int getXPositionOfFirstCharacter(int y);

    void shiftCursorFullRight();
    void shiftCursorFullLeft();
    void shiftCursorFullTop();
    void shiftCursorFullBottom();

    // Finds first occurence of character and returns its index. Searches forwards first, then backwards. If no character found, return original mouse position
    int findCharacterIndex(char character);

    bool isCharacterSymbolic(char character);
    int beginningNextWordIndex();
    int beginningNextSymbolIndex();


    void insertCharacter(char character);
    char removeCharacter(bool cursorHeadingLeft = true);
    char replaceCharacter(char character);

    void insertLine(bool down);
    void insertLine(std::shared_ptr<LineGapBuffer> line, bool down);
    std::shared_ptr<LineGapBuffer> removeLine();

    void writeToFile(const std::string& fileName);
    void saveCurrentFile();

    // SETTERS
    void setFileName(const std::string& fileName) { m_fileName = fileName; }

    // GETTERS
    const FileGapBuffer& getFileGapBuffer() const { return m_file ; }
    const std::shared_ptr<LineGapBuffer>& getLineGapBuffer(int y) const { return m_file[y]; }
    std::pair<int, int> getCursorPos() const { return std::pair<int, int>(m_cursorY, m_cursorX) ; }
    int cursorXBeforeYMove() const { return m_lastXSinceYMove ; }
    const std::string& fileName() { return m_fileName; }

};
