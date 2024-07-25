#pragma once

class Buffer;

class View
{

private:

    Buffer* m_buffer;

public:

    View(Buffer* buffer);

    void moveCursor(int y, int x);

    void display();
    void displayWithDoubleBuffer();

    void displayBackend();

    void displayCurrentLineGapBuffer(int y);
    void displayCurrentFileGapBuffer();

    void normalCursor();
    void insertCursor();
    void replaceCursor();

};
