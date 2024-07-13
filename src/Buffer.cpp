#include "Buffer.h"
#include "View.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <ncurses.h>

Buffer::Buffer(const std::string& fileName, View* view)
    : m_view(view), m_fileName(fileName), m_cursorX(0), m_cursorY(0), m_lastXSinceYMove(0)
{
    if (doesFileExist(fileName))
    {
        readFromFile(fileName);
    }
    else
    {
        m_lines.push_back(GapBuffer(GapBuffer::initialBufferSize));
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

    while (getline(infile, line))
    {
        m_lines.push_back(GapBuffer(GapBuffer::initialBufferSize, line));
    }
}

void Buffer::moveCursor(int y, int x)
{
    m_cursorY = std::clamp(y, 0, static_cast<int>(m_lines.size()));
    m_cursorX = std::clamp(x, 0, static_cast<int>(m_lines[m_cursorY].lineSize()));

    m_view->moveCursor(m_cursorY, m_cursorX);
}

void Buffer::shiftCursorX(int x)
{

    int minMoveX = std::min(m_cursorX + x, static_cast<int>(m_lines[m_cursorY].lineSize()) - 1);
    int moveX = std::max(0, minMoveX);

    if (moveX) { m_lastXSinceYMove = moveX ; }

    moveCursor(m_cursorY, moveX);

    refresh();
}

void Buffer::shiftCursorY(int y)
{
    int minMoveY = std::min(m_cursorY + y, static_cast<int>(m_lines.size()) - 1);
    int moveY = std::max(minMoveY, 0);

    m_cursorY = moveY;

    int minCursorX = std::min(static_cast<int>(m_lines[m_cursorY].lineSize() - 1), m_lastXSinceYMove);
    m_cursorX = std::max(0, minCursorX);

    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::shiftCursorFullRight()
{
    m_cursorX = static_cast<int>(m_lines[m_cursorY].lineSize()) - 1;
    m_lastXSinceYMove = m_cursorX;
    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::shiftCursorFullLeft()
{
    for (int i = 0; i < static_cast<int>(m_lines[m_cursorY].lineSize()); i++)
    {
        if (m_lines[m_cursorY][i] != ' ')
        {
            m_cursorX = i;
            m_lastXSinceYMove = i;
            moveCursor(m_cursorY, i);
            break;
        }

        m_cursorX = 0;
        m_lastXSinceYMove = 0;
        move(m_cursorY, 0);
    }

    refresh();
}

void Buffer::shiftCursorFullTop()
{
    m_cursorY = 0;

    int minCursorX = std::min(static_cast<int>(m_lines[m_cursorY].lineSize() - 1), m_lastXSinceYMove);
    m_cursorX = std::max(0, minCursorX);

    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::shiftCursorFullBottom()
{
    m_cursorY = static_cast<int>(m_lines.size()) - 1;

    int minCursorX = std::min(static_cast<int>(m_lines[m_cursorY].lineSize() - 1), m_lastXSinceYMove);
    m_cursorX = std::max(0, minCursorX);

    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::insertCharacter(char character)
{
    // m_lines[m_cursorY].insert(m_lines[m_cursorY].begin() + m_cursorX, character);
    //
    // m_cursorX += 1;
    // move(m_cursorY, m_cursorX);

    m_lines[m_cursorY].insertChar(character);
    moveCursor(m_cursorY, m_cursorX + 1);
}

void Buffer::insertCharacter(char character, int y, int x)
{
    // m_lines[y].insert(m_lines[y].begin() + x, character);

    m_lines[y].insertChar(character);
}

char Buffer::removeCharacter()
{
    // char character = m_lines[m_cursorY][m_cursorX];
    // m_lines[m_cursorY].erase(m_cursorX, 1);
    // return character;

    char character = m_lines[m_cursorY].deleteChar();
    return character;

}

char Buffer::removeCharacter(int y, int x)
{
    // char character = m_lines[y][x];
    // m_lines[y].erase(std::max(0, x), 1);
    char character = m_lines[y].deleteChar();
    return character;
}
