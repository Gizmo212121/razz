#pragma once

#include "LineGapBuffer.h"

class Buffer;

class View
{

private:

    Buffer* m_buffer;

    int m_reservedColumnsForLineNumbering = 0;

    int m_prevLinesDown = 0;
    int m_linesDown = 0;

    void adjustLinesAfterScrolling(int relativeCursorPosY, int upperLineMoveThreshold, int lowerLineMoveThreshold);
    void printCharacter(int y, int x, char character);
    void clearRemainingLines(int maxRender, int extraLinesFromWrapping);
    int indexOfFirstNonSpaceCharacter(const std::shared_ptr<LineGapBuffer>& line) const;
    int numberOfDigits(int x);
    void moveCursor(const std::pair<int, int>& cursorPos, int cursorIndexOfFirstNonSpace, int extraLinesFromWrappingBeforeCursor);

    int printLine(const std::shared_ptr<LineGapBuffer>& lineGapBuffer, int row, int indexOfFirstNonSpace, int extraLinesFromWrapping, int relativeCursorY, int extraLinesFromWrappingBeforeCursor);

public:

    View();
    View(Buffer* buffer);

    void display();

    void displayBackend();
    void displayCurrentLineGapBuffer(int y);
    void displayCurrentFileGapBuffer();

    void normalCursor();
    void insertCursor();
    void replaceCursor();

};
