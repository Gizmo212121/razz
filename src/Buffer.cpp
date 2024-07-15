#include "Buffer.h"
#include "View.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <ncurses.h>
#include <term.h>

Buffer::Buffer(const std::string& fileName, View* view)
    : m_view(view), m_fileName(fileName), m_cursorX(0), m_cursorY(0), m_lastXSinceYMove(0)
{
    if (doesFileExist(fileName))
    {
        readFromFile(fileName);
    }
    else
    {
        m_lines.push_back(GapBuffer<char>(GapBuffer<char>::initialBufferSize));
    }
}

bool Buffer::doesFileExist(const std::string& fileName) const
{
    std::ifstream infile(fileName);
    return infile.good();
}

void Buffer::readFromFile(const std::string& fileName)
{
    std::ifstream infile(fileName);

    std::string line;

    int lineIter = 0;
    while (getline(infile, line))
    {
        m_lines.push_back(GapBuffer<char>(GapBuffer<char>::initialBufferSize, line));
        while (m_lines[lineIter].preGapIndex() > 0) { m_lines[lineIter].left(); }
        lineIter++;
    }
}

void Buffer::moveCursor(int y, int x, bool render)
{
    m_cursorY = std::clamp(y, 0, static_cast<int>(m_lines.size()));

    int moveX = std::clamp(x, 0, static_cast<int>(m_lines[m_cursorY].lineSize()));

    int relativeDistance = moveX - m_lines[y].preGapIndex();

    if (relativeDistance > 0)
    {
        for (int i = 0; i < relativeDistance; i++) { m_lines[m_cursorY].right(); }
    }
    else
    {
        for (int i = 0; i < abs(relativeDistance); i++) { m_lines[m_cursorY].left(); }
    }

    m_cursorX = moveX;
    m_lastXSinceYMove = moveX;

    m_view->moveCursor(m_cursorY, m_cursorX);
    if (render) { m_view->displayCurrentLine(m_cursorY); }
}

void Buffer::shiftCursorX(int x, bool render)
{

    int moveX = std::clamp(m_cursorX + x, 0, std::max(0, static_cast<int>(m_lines[m_cursorY].lineSize()) - 1));

    if (moveX == m_cursorX) { return; }

    m_cursorX = moveX;
    m_lastXSinceYMove = moveX;

    if (x > 0)
    {
        for (int i = 0; i < x; i++) { m_lines[m_cursorY].right(); }
    }
    else
    {
        for (int i = 0; i < abs(x); i++) { m_lines[m_cursorY].left(); }
    }

    m_view->moveCursor(m_cursorY, m_cursorX);
    if (render) { m_view->displayCurrentLine(m_cursorY); }
}

void Buffer::shiftCursorY(int y, bool render)
{
    m_cursorY = std::clamp(m_cursorY + y, 0, static_cast<int>(m_lines.size()) - 1);

    int moveX = std::clamp(m_lastXSinceYMove, 0, std::max(0, static_cast<int>(m_lines[m_cursorY].lineSize()) - 1));

    int relativeMoveX = moveX - m_lines[m_cursorY].preGapIndex();

    if (relativeMoveX == 0) 
    { 
        m_view->moveCursor(m_cursorY, m_cursorX);
        m_view->displayCurrentLine(m_cursorY);
        m_cursorX = moveX;
        return; 
    }

    m_cursorX = moveX;

    if (relativeMoveX > 0)
    {
        for (int i = 0; i < relativeMoveX; i++) { m_lines[m_cursorY].right(); }
    }
    else
    {
        for (int i = 0; i < abs(relativeMoveX); i++) { m_lines[m_cursorY].left(); }
    }

    m_view->moveCursor(m_cursorY, m_cursorX);
    if (render) { m_view->displayCurrentLine(m_cursorY); }
}

void Buffer::shiftCursorXWithoutGapBuffer(int x, bool render)
{
    int moveX = std::clamp(m_cursorX + x, 0, std::max(0, static_cast<int>(m_lines[m_cursorY].lineSize()) - 1));

    if (moveX == m_cursorX) { return; }

    m_cursorX = moveX;
    m_lastXSinceYMove = moveX;

    // moveCursor(m_cursorY, moveX);
    m_view->moveCursor(m_cursorY, m_cursorX);
    if (render) { m_view->displayCurrentLine(m_cursorY); }
}

void Buffer::shiftCursorFullRight()
{
    shiftCursorX(static_cast<int>(m_lines[m_cursorY].lineSize()) - 1 - m_cursorX);
}

void Buffer::shiftCursorFullLeft()
{
    for (int i = 0; i < static_cast<int>(m_lines[m_cursorY].lineSize()); i++)
    {
        if (m_lines[m_cursorY][i] != ' ')
        {
            moveCursor(m_cursorY, i);
            break;
        }

        moveCursor(m_cursorY, 0);
    }
}

void Buffer::shiftCursorFullTop()
{
    shiftCursorY(- m_cursorY);
}

void Buffer::shiftCursorFullBottom()
{
    int fullBottomIndex = std::max(0, static_cast<int>(m_lines.size() - 1));
    shiftCursorY(fullBottomIndex - m_cursorY);
}

void Buffer::insertCharacter(char character, bool render)
{
    m_lines[m_cursorY].insertItem(character);
    m_lines[m_cursorY].left();
    moveCursor(m_cursorY, m_cursorX + 1, render);
}

char Buffer::removeCharacter(bool cursorHeadingLeft, bool render)
{
    if (cursorHeadingLeft)
    {
        shiftCursorXWithoutGapBuffer(-1, false);

        char character = m_lines[m_cursorY].getLine()[m_cursorX];
        m_lines[m_cursorY].deleteItem();

        if (render) { m_view->displayCurrentLine(m_cursorY); }

        return character;
    }
    else
    {
        m_lines[m_cursorY].right();

        char character = m_lines[m_cursorY].getLine()[m_cursorX];
        m_lines[m_cursorY].deleteItem();

        int cursorBeforeMove = m_cursorX;
        shiftCursorXWithoutGapBuffer(0, false);
        if (m_cursorX != cursorBeforeMove)
        {
            m_lines[m_cursorY].left();
        }

        if (render) { m_view->displayCurrentLine(m_cursorY); }

        return character;
    }
}

char Buffer::replaceCharacter(char character)
{
    moveCursor(m_cursorY, m_cursorX + 1);
    char replacedChar = removeCharacter(true, false);

    insertCharacter(character, false);
    shiftCursorX(-1, false);

    m_view->displayCurrentLine(m_cursorY);

    return replacedChar;
}
