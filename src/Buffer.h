#pragma once

#include "FileGapBuffer.h"

class Buffer
{

private:

    std::filesystem::path m_filePath;

    FileGapBuffer m_file;

    int m_cursorX;
    int m_cursorY;
    int m_lastXSinceYMove;

    std::pair<int, int> m_lastYankInitialPos;
    std::pair<int, int> m_lastYankFinalPos;

private:

    void readFromFile(const std::string& fileName);

public:

    Buffer();
    Buffer(const std::string& fileName);

    void moveCursor(int y, int x);

    void shiftCursorX(int x);
    void shiftCursorY(int y);

    void shiftCursorXWithoutGapBuffer(int x);

    int getXPositionOfFirstCharacter(int y);

    void shiftCursorFullRight();
    void shiftCursorFullLeft();
    void shiftCursorFullTop();
    void shiftCursorFullBottom();

    // Finds first occurence of character and returns its index. If no character found, return original mouse position
    int findCharacterIndex(char character, bool findForwards);

    bool isCharacterSymbolic(char character);

    int beginningNextWordIndex();
    int beginningNextSymbolIndex();

    int endNextWordIndex();
    int endNextSymbolIndex();

    int endPreviousWordIndex();
    int endPreviousSymbolIndex();

    int beginningPreviousWordIndex();
    int beginningPreviousSymbolIndex();

    int indexOfFirstNonSpaceCharacter(const std::shared_ptr<LineGapBuffer>& line) const;


    void insertCharacter(char character);
    char removeCharacter(bool cursorHeadingLeft = true);
    char replaceCharacter(char character);

    void insertLine(bool down);
    void insertLine(std::shared_ptr<LineGapBuffer> line, bool down);
    std::shared_ptr<LineGapBuffer> removeLine();

    void writeToFile(const std::filesystem::path& filePath);
    void saveCurrentFile();

    // SETTERS
    void setFileName(const std::filesystem::path& filePath) { m_filePath = filePath; }
    void setLastYankInitialCursor(const std::pair<int, int>& pos) { m_lastYankInitialPos = pos; }
    void setLastYankFinalCursor(const std::pair<int, int>& pos) { m_lastYankFinalPos = pos; }

    // GETTERS
    const FileGapBuffer& getFileGapBuffer() const { return m_file ; }
    const std::shared_ptr<LineGapBuffer>& getLineGapBuffer(int y) const { return m_file[y]; }
    std::pair<int, int> getCursorPos() const { return std::pair<int, int>(m_cursorY, m_cursorX) ; }
    int cursorXBeforeYMove() const { return m_lastXSinceYMove ; }
    const std::filesystem::path& filePath() const { return m_filePath; }
    const std::pair<int, int>& lastYankInitialCursor() const { return m_lastYankInitialPos; }
    const std::pair<int, int>& lastYankFinalCursor() const { return m_lastYankFinalPos; }

};
