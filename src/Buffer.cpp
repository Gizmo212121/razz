#include "Buffer.h"
#include "LineGapBuffer.h"
#include "View.h"

#include <algorithm>
#include <chrono>
#include <fstream>

#include <ncurses.h>

#include <chrono>

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
}

void Buffer::moveCursor(int y, int x)
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

    if (y == static_cast<int>(m_file.numberOfLines())) { return; }

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
}

void Buffer::shiftCursorX(int x)
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
}

void Buffer::shiftCursorY(int y)
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
}

void Buffer::shiftCursorXWithoutGapBuffer(int x)
{
    int moveX = std::clamp(m_cursorX + x, 0, std::max(0, static_cast<int>(m_file[m_cursorY]->lineSize()) - 1));

    if (moveX == m_cursorX) { return; }

    m_cursorX = moveX;
    m_lastXSinceYMove = moveX;

    // moveCursor(m_cursorY, moveX);
    m_view->moveCursor(m_cursorY, m_cursorX);
}

int Buffer::getXPositionOfFirstCharacter(int y)
{
    y = std::max(0, y);

    int lineSize = static_cast<int>(m_file[y]->lineSize());
    for (int i = 0; i < lineSize; i++)
    {
        if (m_file[y]->at(i) != ' ')
        {
            return i;
        }
    }

    return lineSize;
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

int Buffer::findCharacterIndex(char character, bool findForwards)
{
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];

    if (findForwards)
    {
        for (size_t i = m_cursorX + 1; i < lineGapBuffer->lineSize(); i++)
        {
            if (lineGapBuffer->at(i) == character)
            {
                return i;
            }
        }
    }
    else
    {
        for (size_t i = m_cursorX - 1; i > 0; i--)
        {
            if (lineGapBuffer->at(i) == character)
            {
                return i;
            }
        }
    }

    return m_cursorX;
}

void Buffer::insertCharacter(char character)
{
    m_file[m_cursorY]->insertChar(character);
    m_file[m_cursorY]->left();
    moveCursor(m_cursorY, m_cursorX + 1);
}

char Buffer::removeCharacter(bool cursorHeadingLeft)
{
    if (cursorHeadingLeft)
    {
        shiftCursorXWithoutGapBuffer(-1);

        char character = m_file[m_cursorY]->getLine()[m_cursorX];
        m_file[m_cursorY]->deleteChar();

        return character;
    }
    else
    {
        m_file[m_cursorY]->right();

        char character = m_file[m_cursorY]->getLine()[m_cursorX];
        m_file[m_cursorY]->deleteChar();

        int cursorBeforeMove = m_cursorX;
        shiftCursorXWithoutGapBuffer(0);
        if (m_cursorX != cursorBeforeMove)
        {
            m_file[m_cursorY]->left();
        }

        return character;
    }
}

void Buffer::insertLine(bool down)
{
    if (down) { m_file.down(); }

    m_file.insertLine(std::make_shared<LineGapBuffer>(1));

    m_file.up();
    moveCursor(m_cursorY + 1 * down, m_cursorX);
}

void Buffer::insertLine(std::shared_ptr<LineGapBuffer> line, bool down)
{
    if (down) { m_file.down(); }

    m_file.insertLine(line);

    m_file.up();
    moveCursor(m_cursorY + 1 * down, m_cursorX);
}

std::shared_ptr<LineGapBuffer> Buffer::removeLine()
{
    moveCursor(m_cursorY + 1, m_cursorX);
    std::shared_ptr<LineGapBuffer> line = m_file.deleteLine();

    if (m_file.numberOfLines() != 0)
    {
        moveCursor(m_cursorY - 1, m_cursorX);
    }
    else
    {
        insertLine(true);
        moveCursor(0, 0);
    }

    return line;
}

char Buffer::replaceCharacter(char character)
{
    moveCursor(m_cursorY, m_cursorX + 1);
    char replacedChar = removeCharacter(true);

    insertCharacter(character);
    shiftCursorX(-1);

    return replacedChar;
}

void Buffer::writeToFile(const std::string& fileName)
{
    std::ofstream fout;

    fout.open(fileName);

    for (size_t row = 0; row < m_file.numberOfLines(); row++)
    {
        const std::shared_ptr<LineGapBuffer> lineGapBuffer = m_file[row];

        for (size_t column = 0; column < lineGapBuffer->lineSize(); column++)
        {
            fout << lineGapBuffer->at(column);
        }

        if (row < m_file.numberOfLines() - 1) { fout << '\n'; }
    }

    fout.close();
}

void Buffer::saveCurrentFile()
{
    writeToFile(m_fileName);
}

bool Buffer::isCharacterSymbolic(char character)
{
    if ((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z') || (character == '_') || (character >= '0' && character <= '9')) { return false; }
    else { return true; }
}

int Buffer::beginningNextWordIndex()
{
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];
    if (lineGapBuffer->lineSize() == 0) { return m_cursorX; }

    char currentCharacter = m_file[m_cursorY]->at(m_cursorX);
    bool currentCharacterSymbolic = isCharacterSymbolic(currentCharacter);
    bool foundSymbol = false;

    for (size_t index = m_cursorX + 1; index < lineGapBuffer->lineSize(); index++)
    {
        char character = lineGapBuffer->at(index);

        if (currentCharacterSymbolic)
        {
            if (!isCharacterSymbolic(character))
            {
                return index;
            }
        }
        else
        {
            if (foundSymbol)
            {
                if (!isCharacterSymbolic(character))
                {
                    return index;
                }
            }
            else
            {
                foundSymbol = (isCharacterSymbolic(character));
            }
        }
    }

    return m_cursorX;
}

int Buffer::beginningNextSymbolIndex()
{
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];
    if (lineGapBuffer->lineSize() == 0) { return m_cursorX; }

    char currentCharacter = m_file[m_cursorY]->at(m_cursorX);
    bool currentCharacterSymbolic = isCharacterSymbolic(currentCharacter);
    bool foundSpaceOrCharacter = false;

    for (size_t index = m_cursorX + 1; index < lineGapBuffer->lineSize(); index++)
    {
        char character = lineGapBuffer->at(index);

        if (!currentCharacterSymbolic)
        {
            if (isCharacterSymbolic(character) && character != ' ')
            {
                return index;
            }
        }
        else
        {
            if (foundSpaceOrCharacter)
            {
                if (isCharacterSymbolic(character) && character != ' ')
                {
                    return index;
                }
            }
            else
            {
                foundSpaceOrCharacter = (!isCharacterSymbolic(character) || character == ' ');
            }
        }
    }

    return m_cursorX;
}

int Buffer::endNextWordIndex()
{
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];
    size_t lineSize = lineGapBuffer->lineSize();
    if (lineSize == 0) { return m_cursorX; }

    char currentCharacter = m_file[m_cursorY]->at(std::min(m_cursorX + 1, static_cast<int>(lineSize) - 1));
    bool currentCharacterSymbolic = isCharacterSymbolic(currentCharacter);
    bool foundSymbol = false;
    bool foundCharacterOrSpace = false;

    for (size_t index = m_cursorX + 2; index <= lineSize; index++)
    {
        char character = '\0';
        if (index != lineSize) { character = lineGapBuffer->at(index); }

        if (currentCharacterSymbolic)
        {
            if (foundCharacterOrSpace)
            {
                if (isCharacterSymbolic(character))
                {
                    return index - 1;
                }
            }
            else
            {
                foundCharacterOrSpace = (!isCharacterSymbolic(character));
            }
        }
        else
        {
            if (isCharacterSymbolic(character))
            {
                return index - 1;
            }

            if (foundSymbol)
            {
                if (!isCharacterSymbolic(character))
                {
                    return index;
                }
            }
            else
            {
                foundSymbol = (isCharacterSymbolic(character));
            }
        }
    }

    return m_cursorX;
}

int Buffer::endNextSymbolIndex()
{
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];
    size_t lineSize = lineGapBuffer->lineSize();
    if (lineSize == 0) { return m_cursorX; }

    char currentCharacter = m_file[m_cursorY]->at(std::min(m_cursorX + 1, static_cast<int>(lineSize) - 1));
    bool currentCharacterSymbolic = isCharacterSymbolic(currentCharacter) && currentCharacter != ' ';
    bool foundTarget = false;

    for (size_t index = m_cursorX + 2; index <= lineSize; index++)
    {
        char character = 'a';
        if (index != lineSize) { character = lineGapBuffer->at(index); }

        if (!currentCharacterSymbolic)
        {
            if (foundTarget)
            {
                if (!isCharacterSymbolic(character) || character == ' ')
                {
                    return index - 1;
                }
            }
            else
            {
                foundTarget = (isCharacterSymbolic(character) && character != ' ');
            }
        }
        else
        {
            if (!isCharacterSymbolic(character))
            {
                return index - 1;
            }

            if (foundTarget)
            {
                if (isCharacterSymbolic(character) && character != ' ')
                {
                    return index;
                }
            }
            else
            {
                foundTarget = (!isCharacterSymbolic(character) || character == ' ');
            }
        }
    }

    return m_cursorX;
}

int Buffer::endPreviousWordIndex()
{
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];
    if (lineGapBuffer->lineSize() == 0) { return m_cursorX; }
    char currentCharacter = m_file[m_cursorY]->at(m_cursorX);
    bool currentCharacterSymbolic = isCharacterSymbolic(currentCharacter);
    bool foundSpaceOrSymbol = false;

    int startingIndex = std::max(0, m_cursorX - 1);
    for (int index = startingIndex; index >= -1; index--)
    {
        char character = '\0';
        if (index != -1) { character = lineGapBuffer->at(index); };

        if (currentCharacterSymbolic)
        {
            if (!isCharacterSymbolic(character) && character != ' ')
            {
                return index;
            }
        }
        else
        {
            if (foundSpaceOrSymbol)
            {
                if (!isCharacterSymbolic(character))
                {
                    return index;
                }
            }
            else
            {
                foundSpaceOrSymbol = (isCharacterSymbolic(character) || character == ' ');
            }
        }
    }

    return m_cursorX;
}

int Buffer::endPreviousSymbolIndex()
{
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];
    if (lineGapBuffer->lineSize() == 0) { return m_cursorX; }

    char currentCharacter = m_file[m_cursorY]->at(m_cursorX);
    bool currentCharacterSymbolic = isCharacterSymbolic(currentCharacter);
    bool foundSpaceOrCharacter = false;

    int startingIndex = std::max(0, m_cursorX - 1);
    for (int index = startingIndex; index >= -1; index--)
    {
        char character = 'a';
        if (index != -1) { character = lineGapBuffer->at(index); }

        if (!currentCharacterSymbolic)
        {
            if (isCharacterSymbolic(character) && character != ' ')
            {
                return index;
            }
        }
        else
        {
            if (foundSpaceOrCharacter)
            {
                if (isCharacterSymbolic(character) && character != ' ')
                {
                    return index;
                }
            }
            else
            {
                foundSpaceOrCharacter = (!isCharacterSymbolic(character) || character == ' ');
            }
        }
    }

    return m_cursorX;
}

int Buffer::beginningPreviousWordIndex()
{
    if (m_cursorX == 0) { return m_cursorX; }

    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];

    char currentCharacter = m_file[m_cursorY]->at(std::max(0, m_cursorX - 1));
    bool currentCharacterSymbolic = isCharacterSymbolic(currentCharacter);
    bool foundSymbol = false;
    bool foundCharacterOrSpace = false;

    size_t startingIndex = std::max(0, m_cursorX - 2);
    for (int index = startingIndex; index >= -1; index--)
    {
        char character = '\0';
        if (index != -1) { character = lineGapBuffer->at(index); }

        if (currentCharacterSymbolic)
        {
            if (foundCharacterOrSpace)
            {
                if (isCharacterSymbolic(character))
                {
                    return index + 1;
                }
            }
            else
            {
                foundCharacterOrSpace = (!isCharacterSymbolic(character));
            }
        }
        else
        {
            if (isCharacterSymbolic(character))
            {
                return index + 1;
            }

            if (foundSymbol)
            {
                if (!isCharacterSymbolic(character))
                {
                    return index;
                }
            }
            else
            {
                foundSymbol = (isCharacterSymbolic(character));
            }
        }
    }

    return m_cursorX;
}

int Buffer::beginningPreviousSymbolIndex()
{
    if (m_cursorX == 0) { return m_cursorX; }

    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_file[m_cursorY];


    char currentCharacter = m_file[m_cursorY]->at(std::max(m_cursorX - 1, 0));
    bool currentCharacterSymbolic = isCharacterSymbolic(currentCharacter) && currentCharacter != ' ';
    bool foundTarget = false;

    size_t startingIndex = std::max(0, m_cursorX - 2);
    for (int index = startingIndex; index >= -1; index--)
    {
        char character = 'a';
        if (index != -1) { character = lineGapBuffer->at(index); }

        if (!currentCharacterSymbolic)
        {
            if (foundTarget)
            {
                if (!isCharacterSymbolic(character) || character == ' ')
                {
                    return index + 1;
                }
            }
            else
            {
                foundTarget = (isCharacterSymbolic(character) && character != ' ');
            }
        }
        else
        {
            if (!isCharacterSymbolic(character))
            {
                return index + 1;
            }

            if (foundTarget)
            {
                if (isCharacterSymbolic(character) && character != ' ')
                {
                    return index;
                }
            }
            else
            {
                foundTarget = (!isCharacterSymbolic(character) || character == ' ');
            }
        }
    }

    return m_cursorX;
}
