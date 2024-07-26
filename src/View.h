#pragma once

#include <deque>
#include <vector>
#include <stdio.h>

class Buffer;

class View
{

private:

    Buffer* m_buffer;

    int m_linesDown = 0;

    void initializeWrapQueue();

    void adjustLinesAfterScrolling(int relativeCursorPosY, int upperLineMoveThreshold, int lowerLineMoveThreshold);
    void printCharacter(int y, int x, char character);
    void clearRemainingLines(int maxRender);

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
