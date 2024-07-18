#include "Buffer.h"
#include "View.h"

#include <algorithm>
#include <fstream>

Buffer::Buffer(const std::string& fileName, View* view)
    : m_view(view), m_fileName(fileName), m_file(1), m_cursorX(0), m_cursorY(0), m_lastXSinceYMove(0)
{
    if (fileName == "NO_NAME")
    {
        m_file.insertLine(std::make_shared<LineGapBuffer>(LineGapBuffer::initialBufferSize));
    }
    else
    {
        if (doesFileExist(fileName))
        {
            readFromFile(fileName);
        }
        else
        {
            m_file.insertLine(std::make_shared<LineGapBuffer>(LineGapBuffer::initialBufferSize));
        }
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
        m_file.insertLine(std::make_shared<LineGapBuffer>(LineGapBuffer::initialBufferSize, line));
        while (m_file[lineIter]->preGapIndex() > 0) { m_file[lineIter]->left(); }
        lineIter++;
    }

    while (m_file.preGapIndex() > 0)
    {
        m_file.up();
    }

    m_file.down();
}

void Buffer::moveCursor(int y, int x, bool render)
{
    int moveY = std::clamp(y, 0, static_cast<int>(m_file.numberOfLines()));

    int relativeYDistance = moveY - m_file.preGapIndex();

    if (relativeYDistance > 0)
    {
        for (int i = 0; i < relativeYDistance; i++) { m_file.down(); }
    }
    else
    {
        for (int i = 0; i < abs(relativeYDistance); i++) { m_file.up(); }
    }

    if (y == m_file.numberOfLines()) { return; }

    m_cursorY = moveY;

    int moveX = std::clamp(x, 0, static_cast<int>(m_file[m_cursorY]->lineSize()));

    int relativeXDistance = moveX - m_file[y]->preGapIndex();

    if (relativeXDistance > 0)
    {
        for (int i = 0; i < relativeXDistance; i++) { m_file[m_cursorY]->right(); }
    }
    else
    {
        for (int i = 0; i < abs(relativeXDistance); i++) { m_file[m_cursorY]->left(); }
    }

    m_cursorX = moveX;
    m_lastXSinceYMove = moveX;

    m_view->moveCursor(m_cursorY, m_cursorX);
    if (render) { m_view->displayCurrentLine(m_cursorY); }
}

void Buffer::shiftCursorX(int x, bool render)
{

    int moveX = std::clamp(m_cursorX + x, 0, std::max(0, static_cast<int>(m_file[m_cursorY]->lineSize()) - 1));

    if (moveX == m_cursorX) { return; }

    m_cursorX = moveX;
    m_lastXSinceYMove = moveX;

    if (x > 0)
    {
        for (int i = 0; i < x; i++) { m_file[m_cursorY]->right(); }
    }
    else
    {
        for (int i = 0; i < abs(x); i++) { m_file[m_cursorY]->left(); }
    }

    m_view->moveCursor(m_cursorY, m_cursorX);
    if (render) { m_view->displayCurrentLine(m_cursorY); }
}

void Buffer::shiftCursorY(int y, bool render)
{
    int moveY = std::clamp(m_cursorY + y, 0, static_cast<int>(m_file.numberOfLines()) - 1);

    int relativeMoveY = moveY - m_file.preGapIndex();

    if (relativeMoveY > 0)
    {
        for (int i = 0; i < relativeMoveY; i++) { m_file.down(); }
    }
    else
    {
        for (int i = 0; i < abs(relativeMoveY); i++) { m_file.up(); }
    }

    m_cursorY = moveY;

    int moveX = std::clamp(m_lastXSinceYMove, 0, std::max(0, static_cast<int>(m_file[m_cursorY]->lineSize()) - 1));

    int relativeMoveX = moveX - m_file[m_cursorY]->preGapIndex();

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
        for (int i = 0; i < relativeMoveX; i++) { m_file[m_cursorY]->right(); }
    }
    else
    {
        for (int i = 0; i < abs(relativeMoveX); i++) { m_file[m_cursorY]->left(); }
    }

    m_view->moveCursor(m_cursorY, m_cursorX);
    if (render) { m_view->displayCurrentLine(m_cursorY); }
}

void Buffer::shiftCursorXWithoutGapBuffer(int x, bool render)
{
    int moveX = std::clamp(m_cursorX + x, 0, std::max(0, static_cast<int>(m_file[m_cursorY]->lineSize()) - 1));

    if (moveX == m_cursorX) { return; }

    m_cursorX = moveX;
    m_lastXSinceYMove = moveX;

    // moveCursor(m_cursorY, moveX);
    m_view->moveCursor(m_cursorY, m_cursorX);
    if (render) { m_view->displayCurrentLine(m_cursorY); }
}

void Buffer::shiftCursorFullRight()
{
    shiftCursorX(static_cast<int>(m_file[m_cursorY]->lineSize()) - 1 - m_cursorX);
}

void Buffer::shiftCursorFullLeft()
{
    for (int i = 0; i < static_cast<int>(m_file[m_cursorY]->lineSize()); i++)
    {
        if (m_file[m_cursorY]->at(i) != ' ')
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
    int fullBottomIndex = std::max(0, static_cast<int>(m_file.numberOfLines() - 1));
    shiftCursorY(fullBottomIndex - m_cursorY);
}

void Buffer::insertCharacter(char character, bool render)
{
    m_file[m_cursorY]->insertChar(character);
    m_file[m_cursorY]->left();
    moveCursor(m_cursorY, m_cursorX + 1, render);
}

char Buffer::removeCharacter(bool cursorHeadingLeft, bool render)
{
    if (cursorHeadingLeft)
    {
        shiftCursorXWithoutGapBuffer(-1, false);

        char character = m_file[m_cursorY]->getLine()[m_cursorX];
        m_file[m_cursorY]->deleteChar();

        if (render) { m_view->displayCurrentLine(m_cursorY); }

        return character;
    }
    else
    {
        m_file[m_cursorY]->right();

        char character = m_file[m_cursorY]->getLine()[m_cursorX];
        m_file[m_cursorY]->deleteChar();

        int cursorBeforeMove = m_cursorX;
        shiftCursorXWithoutGapBuffer(0, false);
        if (m_cursorX != cursorBeforeMove)
        {
            m_file[m_cursorY]->left();
        }

        if (render) { m_view->displayCurrentLine(m_cursorY); }

        return character;
    }
}

void Buffer::insertLine(bool down, bool render)
{
    if (down) { m_file.down(); }

    m_file.insertLine(std::make_shared<LineGapBuffer>(1));

    m_file.up();
    moveCursor(m_cursorY + 1 * down, m_cursorX, false);

    if (render) { m_view->display(); }
}

std::shared_ptr<LineGapBuffer> Buffer::deleteLine(bool render)
{
    // shiftCursorY(1);
    // std::shared_ptr<LineGapBuffer> line = m_file.deleteLine();
    // shiftCursorY(-1);

    moveCursor(m_cursorY + 1, m_cursorX);
    std::shared_ptr<LineGapBuffer> line = m_file.deleteLine();
    moveCursor(m_cursorY - 1, m_cursorX);

    if (render) { m_view->display(); }

    return line;
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
