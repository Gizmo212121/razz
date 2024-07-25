#pragma once

class Buffer;

class View
{

private:

    Buffer* m_buffer;

    int m_linesDown = 0;


    void adjustLinesAfterScrolling(int relativeCursorPosY, int upperLineMoveThreshold, int lowerLineMoveThreshold);
    void printCharacter(int y, int x, char character);
    void clearRemainingLines(int maxRender);

public:

    View(Buffer* buffer);

    void moveCursor(int y, int x);

    void display();

    void displayBackend();
    void displayCurrentLineGapBuffer(int y);
    void displayCurrentFileGapBuffer();

    void normalCursor();
    void insertCursor();
    void replaceCursor();

};
