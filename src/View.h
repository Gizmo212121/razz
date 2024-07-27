#pragma once

#include "LineGapBuffer.h"

#include <memory>
#include <deque>
#include <vector>
#include <stdio.h>

class Buffer;

class View
{

private:

    Buffer* m_buffer;

    int m_prevLinesDown = 0;
    int m_linesDown = 0;

    void adjustLinesAfterScrolling(int relativeCursorPosY, int upperLineMoveThreshold, int lowerLineMoveThreshold);
    void printCharacter(int y, int x, char character);
    void clearRemainingLines(int maxRender, int extraLinesFromWrapping);
    int indexOfFirstNonSpaceCharacter(const std::shared_ptr<LineGapBuffer>& line) const;

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
