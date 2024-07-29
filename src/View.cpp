#include "View.h"
#include "Buffer.h"
#include "Command.h"
#include "Includes.h"

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

int View::numberOfDigits(int x)
{
    int count = 0;

    do
    {
        x /= 10;
        count++;
    }
    while (x >= 1);

    return count;
}

int View::wrappedLinesBeforeCursor(const FileGapBuffer& fileGapBuffer, int numLines, int relativeCursorY)
{
    int extraLinesFromWrapping = 0;
    int maxRender = std::min(LINES, numLines - m_linesDown);
    int maxRenderCopy = maxRender;

    for (int row = 0; row < maxRender; row++)
    {
        if (row >= relativeCursorY) { return extraLinesFromWrapping; }

        const std::shared_ptr<LineGapBuffer>& lineGapBuffer = fileGapBuffer[row + m_linesDown];
        if (!lineGapBuffer)
            break;

        maxRender = maxRenderCopy - extraLinesFromWrapping;

        int indexOfFirstNonSpace = indexOfFirstNonSpaceCharacter(lineGapBuffer);

        if (indexOfFirstNonSpace >= COLS)
            continue;

        int lineSize = lineGapBuffer->lineSize();

        if (lineSize + m_reservedColumnsForLineNumbering >= COLS)
        {
            extraLinesFromWrapping += (lineSize + m_reservedColumnsForLineNumbering - COLS) / (COLS - indexOfFirstNonSpace - m_reservedColumnsForLineNumbering) + 1;
        }
    }

    return extraLinesFromWrapping;
}

void View::display()
{
    const FileGapBuffer& fileGapBuffer = m_buffer->getFileGapBuffer();
    if (!fileGapBuffer.bufferSize())
        return;

    int numLines = static_cast<int>(fileGapBuffer.numberOfLines());
    m_reservedColumnsForLineNumbering = numberOfDigits(numLines) + 1;

    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    const int relativeCursorPosY = cursorPos.first - m_linesDown;

    const int upperLineMoveThreshold = LINES / 4;
    const int lowerLineMoveThreshold = upperLineMoveThreshold * 3;

    int preCursorWrappedLines = wrappedLinesBeforeCursor(fileGapBuffer, numLines, relativeCursorPosY);
    adjustLinesAfterScrolling(relativeCursorPosY, upperLineMoveThreshold - preCursorWrappedLines, lowerLineMoveThreshold - preCursorWrappedLines);

    move(0, 0);
    curs_set(0);


    int extraLinesFromWrapping = 0;
    int extraLinesFromWrappingBeforeCursor = 0;
    int maxRender = std::min(LINES, numLines - m_linesDown);
    int maxRenderCopy = std::min(LINES, numLines - m_linesDown);
    int cursorIndexOfFirstNonSpace = 0;
    bool scrolledBuffer = (m_prevLinesDown != m_linesDown);

    for (int row = 0; row < maxRender; row++)
    {
        const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(row + m_linesDown);
        if (!lineGapBuffer)
            break;

        int indexOfFirstNonSpace = indexOfFirstNonSpaceCharacter(lineGapBuffer);

        if (indexOfFirstNonSpace >= COLS)
            continue;

        int relativeY = cursorPos.first - m_linesDown;

        move(row + extraLinesFromWrapping, 0);
        if (scrolledBuffer)
            clrtoeol();

        int newLinesCreatedByCurrentLine = printLine(lineGapBuffer, row, indexOfFirstNonSpace, extraLinesFromWrapping, relativeY);

        extraLinesFromWrapping += newLinesCreatedByCurrentLine;

        if (row < relativeY) { extraLinesFromWrappingBeforeCursor += newLinesCreatedByCurrentLine; }
        if (row == relativeY) { cursorIndexOfFirstNonSpace = indexOfFirstNonSpace; }

        maxRender = maxRenderCopy - extraLinesFromWrapping;
    }

    clearRemainingLines(maxRender, extraLinesFromWrapping);
    moveCursor(cursorPos, cursorIndexOfFirstNonSpace, extraLinesFromWrappingBeforeCursor);

    refresh();
    curs_set(1);
}

int View::printLine(const std::shared_ptr<LineGapBuffer>& lineGapBuffer, int row, int indexOfFirstNonSpace, int extraLinesFromWrapping, int relativeCursorY)
{
    int lineSize = static_cast<int>(lineGapBuffer->lineSize());
    int newLinesCreatedByCurrentLine = 0;

    for (int column = 0; column < lineSize; column++)
    {
        char character = lineGapBuffer->at(column);

        if (column + m_reservedColumnsForLineNumbering >= COLS)
        {
            newLinesCreatedByCurrentLine = (column  + m_reservedColumnsForLineNumbering - COLS) / (COLS - indexOfFirstNonSpace - m_reservedColumnsForLineNumbering) + 1;

            int newCursorY = row + extraLinesFromWrapping + newLinesCreatedByCurrentLine;
            int newCursorXWithoutOffset = (column + m_reservedColumnsForLineNumbering - COLS) % (COLS - indexOfFirstNonSpace - m_reservedColumnsForLineNumbering);

            if (newCursorXWithoutOffset == 0)
            {
                move(newCursorY, 0);
                clrtoeol();
            }

            printCharacter(newCursorY, newCursorXWithoutOffset + m_reservedColumnsForLineNumbering + indexOfFirstNonSpace, character);
        }
        else
        {
            printCharacter(row + extraLinesFromWrapping, column + m_reservedColumnsForLineNumbering, character);
        }
    }

    if (!newLinesCreatedByCurrentLine)
    {
        move(row + extraLinesFromWrapping, lineSize + m_reservedColumnsForLineNumbering);
        clrtoeol();
    }
    // Draw line numbers
    for (int i = 0; i < m_reservedColumnsForLineNumbering; i++)
    {
        std::string lineNumber;
        int numberDigits;

        if (relativeCursorY == row)
        {
            move(relativeCursorY + extraLinesFromWrapping, i);
            lineNumber = std::to_string(row + m_linesDown + 1);
            numberDigits = numberOfDigits(row + m_linesDown + 1);
        }
        else
        {
            move(row + extraLinesFromWrapping, i);
            lineNumber = std::to_string(abs(row - relativeCursorY));
            numberDigits = numberOfDigits(abs(row - relativeCursorY));
        }

        if (i == m_reservedColumnsForLineNumbering - 1)
        {
            addch(' ');
        }
        else
        {
            if (i >= m_reservedColumnsForLineNumbering - numberDigits - 1)
            {
                if (relativeCursorY == row)
                {
                    attron(A_BOLD);
                    attron(COLOR_PAIR(LINE_NUMBER_ORANGE));

                    addch(lineNumber[i + numberDigits - m_reservedColumnsForLineNumbering + 1]);

                    attroff(COLOR_PAIR(LINE_NUMBER_ORANGE));
                    attroff(A_BOLD);
                }
                else
                {
                    addch(lineNumber[i + numberDigits - m_reservedColumnsForLineNumbering + 1]);
                }
            }
            else
            {
                addch(' ');
            }
        }
    }

    return newLinesCreatedByCurrentLine;
}

void View::moveCursor(const std::pair<int, int>& cursorPos, int cursorIndexOfFirstNonSpace, int extraLinesFromWrappingBeforeCursor)
{
    if (cursorPos.second + m_reservedColumnsForLineNumbering >= COLS)
    {
        int relativeXAfterWrap = cursorPos.second + m_reservedColumnsForLineNumbering - COLS;
        int lineSizeWithoutIndent = COLS - cursorIndexOfFirstNonSpace - m_reservedColumnsForLineNumbering;

        move(cursorPos.first - m_linesDown + extraLinesFromWrappingBeforeCursor + relativeXAfterWrap / lineSizeWithoutIndent + 1, relativeXAfterWrap % lineSizeWithoutIndent + cursorIndexOfFirstNonSpace + m_reservedColumnsForLineNumbering);
    }
    else
    {
        move(cursorPos.first - m_linesDown + extraLinesFromWrappingBeforeCursor + (cursorPos.second + m_reservedColumnsForLineNumbering) / COLS, (cursorPos.second + m_reservedColumnsForLineNumbering) % COLS);
    }
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
