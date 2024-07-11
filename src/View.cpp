#include "View.h"
#include "Buffer.h"
#include <ncurses.h>

View::View(Buffer* buffer)
    : m_buffer(buffer)
{
    display();
}

void View::moveCursor(int y, int x)
{
    move(y, x);
}

void View::display()
{
    if (!m_buffer->getLines().empty())
    {
        move(0, 0);

        int rowCount = 1;

        for (const std::string& line : m_buffer->getLines())
        {
            addstr(line.c_str());
            move(rowCount++, 0);
        }

        std::pair<int, int> cursorPos = m_buffer->getCursorPos();
        move(cursorPos.first, cursorPos.second);

        refresh();
    }
}

void View::displayCurrentLine(int y)
{
    std::pair<int, int> cursorPos = m_buffer->getCursorPos();
    move(y, 0);
    clrtoeol();
    addstr(m_buffer->getLines()[y].c_str());
    move(cursorPos.first, cursorPos.second);
    refresh();
}
