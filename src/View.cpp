#include "View.h"
#include "Buffer.h"
#include <ncurses.h>

#include <term.h>

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

        for (size_t row = 0; row < m_buffer->getLines().size(); row++)
        {
            for (size_t column = 0; column < m_buffer->getGapBuffer(row).lineSize(); column++)
            {
                move(row, column);
                addch(m_buffer->getGapBuffer(row)[column]);
            }
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

    for (size_t column = 0; column < m_buffer->getGapBuffer(y).lineSize(); column++)
    {
        addch(m_buffer->getGapBuffer(y)[column]);
    }

    move(cursorPos.first, cursorPos.second);

    refresh();
}

void View::displayCurrentLineGapBuffer(int y)
{
    std::pair<int, int> cursorPos = m_buffer->getCursorPos();
    move(40, 0);
    clrtoeol();

    for (size_t column = 0; column < m_buffer->getGapBuffer(y).bufferSize(); column++)
    {
        const std::vector<char>& line = m_buffer->getGapBuffer(y).getLine();
        size_t preIndex = m_buffer->getGapBuffer(y).preGapIndex();
        size_t postIndex = m_buffer->getGapBuffer(y).postGapIndex();

        if (column < preIndex || column >= postIndex)
        {
            addch(line[column]);
        }
        else
        {
            addch('_');
        }
    }

    move(cursorPos.first, cursorPos.second);

    refresh();
}

void View::normalCursor()
{
    putp("\033[2 q");
    fflush(stdout);
}

void View::insertCursor()
{
    putp("\033[5 q");
    fflush(stdout);
}

void View::replaceCursor()
{
    putp("\033[3 q");
    fflush(stdout);
}
