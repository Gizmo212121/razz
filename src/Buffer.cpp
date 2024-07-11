#include "Buffer.h"
#include "View.h"

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
        m_lines.push_back(line);
    }
}

void Buffer::moveCursor(int y, int x)
{
    m_cursorY = y;
    m_cursorX = x;
    
}

void Buffer::shiftCursorX( int x)
{

    int minMoveX = std::min(m_cursorX + x, static_cast<int>(m_lines[m_cursorY].size()) - 1);
    int moveX = std::max(0, minMoveX);

    // if (!moveX) { m_lastXSinceYMove = moveX ; }
    m_lastXSinceYMove = moveX;

    m_cursorX = moveX;

    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::shiftCursorY(int y)
{
    int minMoveY = std::min(m_cursorY + y, static_cast<int>(m_lines.size()) - 1);
    int moveY = std::max(minMoveY, 0);

    m_cursorY = moveY;

    int minCursorX = std::min(static_cast<int>(m_lines[m_cursorY].size() - 1), m_lastXSinceYMove);
    m_cursorX = std::max(0, minCursorX);

    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::shiftCursorFullRight()
{
    m_cursorX = static_cast<int>(m_lines[m_cursorY].size()) - 1;
    m_lastXSinceYMove = m_cursorX;
    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::shiftCursorFullLeft()
{
    for (int i = 0; i < static_cast<int>(m_lines.size()); i++)
    {
        if (m_lines[m_cursorY][i] != ' ')
        {
            m_cursorX = i;
            m_lastXSinceYMove = i;
            move(m_cursorY, i);
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

    int minCursorX = std::min(static_cast<int>(m_lines[m_cursorY].size() - 1), m_lastXSinceYMove);
    m_cursorX = std::max(0, minCursorX);

    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::shiftCursorFullBottom()
{
    m_cursorY = static_cast<int>(m_lines.size()) - 1;

    int minCursorX = std::min(static_cast<int>(m_lines[m_cursorY].size() - 1), m_lastXSinceYMove);
    m_cursorX = std::max(0, minCursorX);

    move(m_cursorY, m_cursorX);

    refresh();
}

void Buffer::insertCharacter(char character)
{
    m_lines[m_cursorY].insert(m_lines[m_cursorY].begin() + m_cursorX, character);

    m_cursorX += 1;
    move(m_cursorY, m_cursorX);
}

void Buffer::insertCharacter(char character, int y, int x)
{
    m_lines[y].insert(m_lines[y].begin() + x - 1, character);
}

void Buffer::removeCharacter()
{
    m_lines[m_cursorY].erase(m_cursorX, 1);
}

void Buffer::removeCharacter(int y, int x)
{
    m_lines[y].erase(x - 1, 1);
}
