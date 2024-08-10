#include "View.h"
#include "Buffer.h"
#include "Command.h"
#include "Includes.h"
#include "Editor.h"
#include <chrono>
#include <ncurses.h>
#include <thread>

View::View(Editor* editor, Buffer* buffer)
    : m_editor(editor), m_buffer(buffer)
{
}

void View::yankHighlightTimer(int milliseconds, YANK_TYPE yankType)
{
    m_previousYankType = yankType;

    std::thread(&View::yankHighlight, this, milliseconds).detach();
}

void View::yankHighlight(int milliseconds)
{
    m_displayHighlight.store(true);

    display();

    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));

    m_displayHighlight.store(false);

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

        int indexOfFirstNonSpace = m_buffer->indexOfFirstNonSpaceCharacter(lineGapBuffer);

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
    std::lock_guard<std::mutex> lock(displayMutex);

    // Timer timer("display");


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
    int maxRender = std::clamp(numLines - m_linesDown, 0, LINES - 2);

    int maxRenderCopy = maxRender;
    int cursorIndexOfFirstNonSpace = 0;
    bool scrolledBuffer = (m_prevLinesDown != m_linesDown);

    for (int row = 0; row < maxRender; row++)
    {
        const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(row + m_linesDown);
        if (!lineGapBuffer)
            break;

        int indexOfFirstNonSpace = m_buffer->indexOfFirstNonSpaceCharacter(lineGapBuffer);

        if (indexOfFirstNonSpace >= COLS - 1)
            continue;

        int relativeY = cursorPos.first - m_linesDown;

        move(row + extraLinesFromWrapping, 0);
        if (scrolledBuffer)
            clrtoeol();

        int newLinesCreatedByCurrentLine = printLine(lineGapBuffer, row, indexOfFirstNonSpace, extraLinesFromWrapping, relativeY);

        extraLinesFromWrapping += newLinesCreatedByCurrentLine;

        if (row < relativeY) { extraLinesFromWrappingBeforeCursor += newLinesCreatedByCurrentLine; }
        if (row == relativeY) { cursorIndexOfFirstNonSpace = indexOfFirstNonSpace; }

        if (numLines - m_linesDown >= LINES - 2)
        {
            maxRender = maxRenderCopy - extraLinesFromWrapping;
        }
    }

    clearRemainingLines(maxRender, extraLinesFromWrapping);

    printBufferInformationLine(cursorPos);
    displayCircularInputBuffer();

    moveCursor(cursorPos, cursorIndexOfFirstNonSpace, extraLinesFromWrappingBeforeCursor);

    refresh();
    curs_set(1);
}

void View::printBufferInformationLine(const std::pair<int, int>& cursorPos)
{
    MODE currentMode = m_editor->mode();
    move(LINES - 2, 0);

    int xPos = 0;
    std::string modeString;
    int colorPair = 0;

    switch (currentMode)
    {
        case NORMAL_MODE:
            modeString = " Normal ";
            colorPair = NORMAL_MODE_PAIR;
            xPos += 8;
            break;
        case INSERT_MODE:
            modeString = " Insert ";
            colorPair = INSERT_MODE_PAIR;
            xPos += 8;
            break;
        case COMMAND_MODE:
            modeString = " Command ";
            colorPair = COMMAND_MODE_PAIR;
            xPos += 9;
            break;
        case REPLACE_CHAR_MODE:
            modeString = " Replace ";
            colorPair = REPLACE_CHAR_MODE_PAIR;
            xPos += 9;
            break;
        case VISUAL_MODE:
            modeString = " Visual ";
            colorPair = VISUAL_MODE_PAIR;
            xPos += 8;
            break;
        case VISUAL_LINE_MODE:
            modeString = " V-Line ";
            colorPair = VISUAL_LINE_MODE_PAIR;
            xPos += 8;
            break;
        case VISUAL_BLOCK_MODE:
            modeString = " V-Block ";
            colorPair = VISUAL_BLOCK_MODE_PAIR;
            xPos += 9;
            break;
    }

    // Draw mode
    attron(COLOR_PAIR(colorPair));
    addstr(modeString.c_str());
    attroff(COLOR_PAIR(colorPair));


    // Draw path and extra spaces
    attron(COLOR_PAIR(PATH_COLOR_PAIR));

    const std::filesystem::path& filePath = m_buffer->filePath();
    std::string fileName;

    if (filePath == "NO_NAME")
    {
        fileName = " NO_NAME ";
    }
    else
    {
        fileName = " " + std::filesystem::absolute(m_buffer->filePath()).string() + " ";
    }

    addstr(fileName.c_str());

    int maxCursorIndicatorSize = numberOfDigits(cursorPos.first + 1) + 3 + numberOfDigits(cursorPos.second + 1);
    for (int i = xPos; i < COLS - static_cast<int>(fileName.size()) - maxCursorIndicatorSize; i++)
    {
        addch(' ');
    }

    attroff(COLOR_PAIR(PATH_COLOR_PAIR));

    // Draw cursor coordinates

    std::string cursorPosition = " " + std::to_string(cursorPos.first + 1) + ":" + std::to_string(cursorPos.second + 1) + " ";

    attron(COLOR_PAIR(colorPair));
    addstr(cursorPosition.c_str());
    attroff(COLOR_PAIR(colorPair));
}

void View::displayCommandBuffer(const int colorPair)
{
    move(LINES - 1, 0);
    clrtoeol();

    const std::string& commandBuffer = m_editor->inputController().commandBuffer();

    attron(colorPair);
    addch(':');
    addstr(commandBuffer.c_str());
    attroff(colorPair);


    refresh();
}

void View::displayCircularInputBuffer()
{
    curs_set(0);

    attron(COLOR_PAIR(BACKGROUND));

    for (size_t i = 0; i < INPUT_CONTROLLER_MAX_CIRCULAR_BUFFER_SIZE; i++)
    {
        move(LINES - 1, COLS - i - 1);
        int input = m_editor->inputController().circularBuffer()[i];
        char character = ' ';
        if (input != -1) { character = static_cast<char>(input); }

        addch(character);
    }

    attroff(COLOR_PAIR(BACKGROUND));

    move(m_previousCursorY, m_previousCursorX);

    refresh();

    curs_set(1);
}

int View::printLine(const std::shared_ptr<LineGapBuffer>& lineGapBuffer, int row, int indexOfFirstNonSpace, int extraLinesFromWrapping, int relativeCursorY)
{
    MODE currentMode = m_editor->mode();
    bool inVisualMode = (currentMode == VISUAL_MODE || currentMode == VISUAL_LINE_MODE || currentMode == VISUAL_BLOCK_MODE);
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

    int lineSize = static_cast<int>(lineGapBuffer->lineSize());
    int newLinesCreatedByCurrentLine = 0;

    for (int column = 0; column < lineSize; column++)
    {
        char character = lineGapBuffer->at(column);

        // Line colorings in visual modes
        int colorPair = getColorPair(currentMode, row, column, cursorPos, relativeCursorY);

        attron(COLOR_PAIR(colorPair));

        if (column + m_reservedColumnsForLineNumbering >= COLS)
        {
            if (COLS - indexOfFirstNonSpace - m_reservedColumnsForLineNumbering == 0) { return newLinesCreatedByCurrentLine; }

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

        attroff(COLOR_PAIR(colorPair));
    }

    if (!newLinesCreatedByCurrentLine)
    {
        move(row + extraLinesFromWrapping, lineSize + m_reservedColumnsForLineNumbering);
        clrtoeol();
    }

    if (lineSize == 0)
    {
        if (currentMode == VISUAL_MODE || currentMode == VISUAL_LINE_MODE || currentMode == VISUAL_BLOCK_MODE)
        {
            const std::pair<int, int>& previousVisualPos = m_editor->inputController().initialVisualModeCursor();

            if (row + extraLinesFromWrapping + m_linesDown >= std::min(previousVisualPos.first, cursorPos.first) && row + extraLinesFromWrapping + m_linesDown <= std::max(previousVisualPos.first, cursorPos.first))
            {
                attron(COLOR_PAIR(VISUAL_HIGHLIGHT_PAIR));
                printCharacter(row + extraLinesFromWrapping, m_reservedColumnsForLineNumbering, ' ');
                attroff(COLOR_PAIR(VISUAL_HIGHLIGHT_PAIR));
            }
        }
    }

    // Color the rest of the line the cursor is at

    if (row == relativeCursorY && !inVisualMode)
    {
        attron(COLOR_PAIR(PATH_COLOR_PAIR));

        int newCursorY = row + extraLinesFromWrapping + newLinesCreatedByCurrentLine;
        int newCursorX = 0;

        if (newLinesCreatedByCurrentLine)
        {
            newCursorX = (lineSize + m_reservedColumnsForLineNumbering - COLS) % (COLS - indexOfFirstNonSpace - m_reservedColumnsForLineNumbering) + m_reservedColumnsForLineNumbering + indexOfFirstNonSpace;
        }
        else { newCursorX = lineSize + m_reservedColumnsForLineNumbering; }

        for (int i = newCursorX; i < COLS; i++)
        {
            move(newCursorY, i);
            addch(' ');
        }

        attroff(COLOR_PAIR(PATH_COLOR_PAIR));
    }

    // Draw line number
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
                    attron(COLOR_PAIR(LINE_NUMBER_ORANGE));
                    attron(A_BOLD);

                    addch(lineNumber[i + numberDigits - m_reservedColumnsForLineNumbering + 1]);

                    attroff(COLOR_PAIR(LINE_NUMBER_ORANGE));
                    attroff(A_BOLD);
                }
                else
                {
                    attron(COLOR_PAIR(LINE_NUMBER_GREY));

                    addch(lineNumber[i + numberDigits - m_reservedColumnsForLineNumbering + 1]);

                    // FIXME: Stupid fix; probably makes the rendering go slower
                    //        it fixes the mouse flickering
                    curs_set(0);
                    refresh();

                    attroff(COLOR_PAIR(LINE_NUMBER_GREY));
                }
            }
            else
            {
                addch(' ');
            }
        }
    }

    refresh();

    return newLinesCreatedByCurrentLine;
}

int View::getColorPair(MODE currentMode, int row, int column, const std::pair<int, int>& cursorPos, int relativeCursorY) const
{
    if (m_displayHighlight)
    {
        const std::pair<int, int>& initialYankPos = m_buffer->lastYankInitialCursor();
        const std::pair<int, int>& finalYankPos = m_buffer->lastYankFinalCursor();

        switch (m_previousYankType)
        {
            case VISUAL_YANK:
            {
                int lowerBoundY = std::min(initialYankPos.first, finalYankPos.first);
                int upperBoundY = std::max(initialYankPos.first, finalYankPos.first);

                int lowerBoundX = std::min(initialYankPos.second, finalYankPos.second);
                int upperBoundX = std::max(initialYankPos.second, finalYankPos.second);

                bool isWithinXBounds = (column >= lowerBoundX && column <= upperBoundX);

                if (row + m_linesDown == lowerBoundY)
                {
                    if (lowerBoundY == upperBoundY)
                    {
                        if (isWithinXBounds) { return YANK_HIGHLIGHT_PAIR; }
                    }
                    else if ((lowerBoundY < initialYankPos.first && column >= finalYankPos.second) ||
                             (lowerBoundY > initialYankPos.first && column >= initialYankPos.second) ||
                             (lowerBoundY == initialYankPos.first && column >= initialYankPos.second))
                    {
                        return YANK_HIGHLIGHT_PAIR;
                    }
                }
                else if (row + m_linesDown == upperBoundY)
                {
                    if ((upperBoundY > initialYankPos.first && column <= finalYankPos.second) ||
                        (upperBoundY <= initialYankPos.first && column <= initialYankPos.second))
                    {
                        return YANK_HIGHLIGHT_PAIR;
                    }
                }
                else if (row + m_linesDown > lowerBoundY && row + m_linesDown < upperBoundY)
                {
                    return YANK_HIGHLIGHT_PAIR;
                }

                break;
            }
            case LINE_YANK:
            {
                int lowerBound = std::min(initialYankPos.first, finalYankPos.first);
                int upperBound = std::max(initialYankPos.first, finalYankPos.first);

                if (row + m_linesDown >= lowerBound && row + m_linesDown <= upperBound)
                {
                    return YANK_HIGHLIGHT_PAIR;
                }
                break;
            }
            case BLOCK_YANK:
            {
                int lowerBoundY = std::min(initialYankPos.first, finalYankPos.first);
                int upperBoundY = std::max(initialYankPos.first, finalYankPos.first);

                int lowerBoundX = std::min(initialYankPos.second, finalYankPos.second);
                int upperBoundX = std::max(initialYankPos.second, finalYankPos.second);

                if (column >= lowerBoundX && column <= upperBoundX && row + m_linesDown >= lowerBoundY && row + m_linesDown <= upperBoundY)
                {
                    return YANK_HIGHLIGHT_PAIR;
                }
                break;
            }
            default:
                return BACKGROUND;
        }
    }
    else
    {
        const std::pair<int, int>& visualModeInitialCursor = m_editor->inputController().initialVisualModeCursor();

        switch (currentMode)
        {
            case VISUAL_MODE:
            {
                int relativePreviousVisualY = visualModeInitialCursor.first - m_linesDown;
                int lowerBoundY = std::min(relativeCursorY, relativePreviousVisualY);
                int upperBoundY = std::max(relativeCursorY, relativePreviousVisualY);

                int lowerBoundX = std::min(cursorPos.second, visualModeInitialCursor.second);
                int upperBoundX = std::max(cursorPos.second, visualModeInitialCursor.second);

                bool isWithinXBounds = (column >= lowerBoundX && column <= upperBoundX);

                if (row == lowerBoundY)
                {
                    if (lowerBoundY == upperBoundY)
                    {
                        if (isWithinXBounds) { return VISUAL_HIGHLIGHT_PAIR; }
                    }
                    else if ((lowerBoundY < relativeCursorY && column >= visualModeInitialCursor.second) ||
                             (lowerBoundY > relativeCursorY && column >= cursorPos.second) ||
                             (lowerBoundY == relativeCursorY && column >= cursorPos.second))
                    {
                        return VISUAL_HIGHLIGHT_PAIR;
                    }
                }
                else if (row == upperBoundY)
                {
                    if ((upperBoundY > relativeCursorY && column <= visualModeInitialCursor.second) ||
                        (upperBoundY <= relativeCursorY && column <= cursorPos.second))
                    {
                        return VISUAL_HIGHLIGHT_PAIR;
                    }
                }
                else if (row > lowerBoundY && row < upperBoundY)
                {
                    return VISUAL_HIGHLIGHT_PAIR;
                }

                break;
            }
            case VISUAL_LINE_MODE:
            {
                int relativePreviousVisualY = visualModeInitialCursor.first - m_linesDown;

                int lowerBound = std::min(relativeCursorY, relativePreviousVisualY);
                int upperBound = std::max(relativeCursorY, relativePreviousVisualY);

                if (row >= lowerBound && row <= upperBound)
                {
                    return VISUAL_HIGHLIGHT_PAIR;
                }
                break;
            }
            case VISUAL_BLOCK_MODE:
            {
                int relativePreviousVisualY = visualModeInitialCursor.first - m_linesDown;
                int lowerBoundY = std::min(relativeCursorY, relativePreviousVisualY);
                int upperBoundY = std::max(relativeCursorY, relativePreviousVisualY);

                int lowerBoundX = std::min(cursorPos.second, visualModeInitialCursor.second);
                int upperBoundX = std::max(cursorPos.second, visualModeInitialCursor.second);

                if (column >= lowerBoundX && column <= upperBoundX && row >= lowerBoundY && row <= upperBoundY)
                {
                    return VISUAL_HIGHLIGHT_PAIR;
                }
                break;
            }
            default:
                if (row == relativeCursorY)
                {
                    return PATH_COLOR_PAIR;
                }
                else
                {
                    return BACKGROUND;
                }
                break;
        }
    }


    return BACKGROUND;
}

void View::moveCursor(const std::pair<int, int>& cursorPos, int cursorIndexOfFirstNonSpace, int extraLinesFromWrappingBeforeCursor)
{
    if (cursorPos.second + m_reservedColumnsForLineNumbering >= COLS)
    {
        int relativeXAfterWrap = cursorPos.second + m_reservedColumnsForLineNumbering - COLS;
        int lineSizeWithoutIndent = COLS - cursorIndexOfFirstNonSpace - m_reservedColumnsForLineNumbering;

        if (lineSizeWithoutIndent == 0) { return; }

        int cursorY = cursorPos.first - m_linesDown + extraLinesFromWrappingBeforeCursor + relativeXAfterWrap / lineSizeWithoutIndent + 1;
        int cursorX = relativeXAfterWrap % lineSizeWithoutIndent + cursorIndexOfFirstNonSpace + m_reservedColumnsForLineNumbering;

        move(cursorY, cursorX);

        m_previousCursorY = cursorY;
        m_previousCursorX = cursorX;
    }
    else
    {
        int cursorY = cursorPos.first - m_linesDown + extraLinesFromWrappingBeforeCursor + (cursorPos.second + m_reservedColumnsForLineNumbering) / COLS;
        int cursorX = (cursorPos.second + m_reservedColumnsForLineNumbering) % COLS;
        move(cursorY, cursorX);

        m_previousCursorY = cursorY;
        m_previousCursorX = cursorX;
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
