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
    void displayCurrentLine(int y);
    void displayCurrentLineGapBuffer(int y);

    void normalCursor();
    void insertCursor();
    void replaceCursor();

};
