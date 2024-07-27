#include "View.h"
#include "Buffer.h"
#include "Command.h"
#include <ncurses.h>

#include <term.h>

View::View(Buffer* buffer)
    : m_buffer(buffer)
{
    display();
}

void View::adjustLinesAfterScrolling(int relativeCursorPosY, int upperLineMoveThreshold, int lowerLineMoveThreshold)
{
    m_prevLinesDown = m_linesDown;

    if (relativeCursorPosY <= upperLineMoveThreshold)
    {
        m_linesDown = std::max(0, m_linesDown + relativeCursorPosY - upperLineMoveThreshold);
    }
    else if (relativeCursorPosY >= lowerLineMoveThreshold)
    {
        m_linesDown += relativeCursorPosY - lowerLineMoveThreshold;
    }
}

int View::indexOfFirstNonSpaceCharacter(const std::shared_ptr<LineGapBuffer>& line) const
{
    for (size_t i = 0; i < line->lineSize(); i++)
    {
        if (line->at(i) != ' ') { return static_cast<int>(i); }
    }

    return 0;
}

void View::display()
{
    if (!m_buffer->getFileGapBuffer().bufferSize()) { return; }

    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    const int relativeCursorPosY = cursorPos.first - m_linesDown;

    const int upperLineMoveThreshold = LINES / 4;
    const int lowerLineMoveThreshold = upperLineMoveThreshold * 3;

    adjustLinesAfterScrolling(relativeCursorPosY, upperLineMoveThreshold, lowerLineMoveThreshold);

    move(0, 0);

    curs_set(0);

    int numLines = static_cast<int>(m_buffer->getFileGapBuffer().numberOfLines());
    int extraLinesFromWrapping = 0;
    int extraLinesFromWrappingBeforeCursor = 0;
    int maxRender = std::min(LINES, numLines - m_linesDown);
    int maxRenderCopy = std::min(LINES, numLines - m_linesDown);
    int cursorIndexOfFirstNonSpace = 0;

    bool scrolledBuffer = (m_prevLinesDown != m_linesDown);

    for (int row = 0; row < maxRender; row++)
    {
        maxRender = maxRenderCopy - extraLinesFromWrapping;

        const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(row + m_linesDown);

        if (!lineGapBuffer) { break; }

        size_t lineSize = lineGapBuffer->lineSize();
        int indexOfFirstNonSpace = indexOfFirstNonSpaceCharacter(lineGapBuffer);

        if (indexOfFirstNonSpace >= COLS) { continue; }

        move(row + extraLinesFromWrapping, 0);

        // Must clear all lines when scrolling
        if (scrolledBuffer) { clrtoeol(); }

        int newLinesCreatedByCurrentLine = 0;

        for (int column = 0; column < static_cast<int>(lineSize); column++)
        {
            char character = lineGapBuffer->at(column);

            if (column >= COLS)
            {
                newLinesCreatedByCurrentLine = (column - COLS) / (COLS - indexOfFirstNonSpace) + 1;

                int newCursorY = row + extraLinesFromWrapping + newLinesCreatedByCurrentLine;
                int newCursorX = (column - COLS) % (COLS - indexOfFirstNonSpace) + indexOfFirstNonSpace;

                if ((column - COLS) % (COLS - indexOfFirstNonSpace) == 0)
                {
                    move(newCursorY, 0);
                    clrtoeol();
                }

                printCharacter(newCursorY, newCursorX, character);
            }
            else
            {
                printCharacter(row + extraLinesFromWrapping, column, character);
            }
        }

        extraLinesFromWrapping += newLinesCreatedByCurrentLine;

        if (!newLinesCreatedByCurrentLine)
        {
            move(row + extraLinesFromWrapping, lineSize);
            clrtoeol();
        }

        if (row < cursorPos.first - m_linesDown) { extraLinesFromWrappingBeforeCursor += newLinesCreatedByCurrentLine; }

        if (row == cursorPos.first - m_linesDown) { cursorIndexOfFirstNonSpace = indexOfFirstNonSpace; }

    }

    clearRemainingLines(maxRender, extraLinesFromWrapping);

    if (cursorPos.second >= COLS)
    {
        move(cursorPos.first - m_linesDown + extraLinesFromWrappingBeforeCursor + (cursorPos.second - COLS) / (COLS - cursorIndexOfFirstNonSpace) + 1, (cursorPos.second - COLS) % (COLS - cursorIndexOfFirstNonSpace) + cursorIndexOfFirstNonSpace);
    }
    else
    {
        move(cursorPos.first - m_linesDown + extraLinesFromWrappingBeforeCursor + cursorPos.second / COLS, cursorPos.second % COLS);
    }

    refresh();

    curs_set(1);
}

void View::printCharacter(int y, int x, char character)
{
    move(y, x);

    if (mvinch(y, x) != static_cast<size_t>(character))
    {
        addch(character);
    }
}

void View::clearRemainingLines(int maxRender, int extraLinesFromWrapping)
{
    for (int i = maxRender + extraLinesFromWrapping; i < LINES; i++)
    {
        move(i , 0);
        clrtoeol();
    }
}

void View::displayBackend()
{
    if (m_buffer->getFileGapBuffer().bufferSize())
    {
        move(0, 0);

        const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

        const FileGapBuffer& fileGapBuffer = m_buffer->getFileGapBuffer();
        const std::vector<std::shared_ptr<LineGapBuffer>>& fileGapBufferVector = fileGapBuffer.getVectorOfSharedPtrsToLineGapBuffers();

        for (size_t row = 0; row < fileGapBuffer.bufferSize(); row++)
        {
            size_t filePreIndex = fileGapBuffer.preGapIndex();
            size_t filePostIndex = fileGapBuffer.postGapIndex();

            if (row < filePreIndex || row >= filePostIndex)
            {
                const std::shared_ptr<LineGapBuffer>& lineGapBuffer = fileGapBufferVector[row];

                size_t linePreIndex = lineGapBuffer->preGapIndex();
                size_t linePostIndex = lineGapBuffer->postGapIndex();

                const std::vector<char>& lineChars = lineGapBuffer->getLine();

                for (size_t column = 0; column < lineGapBuffer->bufferSize(); column++)
                {
                    if (column < linePreIndex || column >= linePostIndex)
                    {
                        addch(lineChars[column]);
                    }
                    else
                    {
                        addch('_');
                    }
                }
            }
            else
            {
                for (int i = 0; i < 50; i++)
                {
                    addch('|');
                }
            }

            move(row + 1, 0);
        }

        move(cursorPos.first, cursorPos.second);

        refresh();
    }
}

void View::displayCurrentLineGapBuffer(int y)
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    move(40, 0);
    clrtoeol();

    for (size_t column = 0; column < m_buffer->getLineGapBuffer(y)->bufferSize(); column++)
    {
        const std::vector<char>& line = m_buffer->getLineGapBuffer(y)->getLine();
        size_t preIndex = m_buffer->getLineGapBuffer(y)->preGapIndex();
        size_t postIndex = m_buffer->getLineGapBuffer(y)->postGapIndex();

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

void View::displayCurrentFileGapBuffer()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

    move(40, 0);

    const std::vector<std::shared_ptr<LineGapBuffer>>& ptrsToLines = m_buffer->getFileGapBuffer().getVectorOfSharedPtrsToLineGapBuffers();

    for (size_t i = 0; i < ptrsToLines.size(); i++)
    {
        // printw("%p", &ptrsToLines[i]);
        printw(" | ");
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
