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

void View::moveCursor(int y, int x)
{
    move(y, x);
}

void View::displayWithDoubleBuffer()
{
    if (m_buffer->getFileGapBuffer().bufferSize())
    {
        WINDOW* virtualWindow = newwin(LINES, COLS, 0, 0);

        curs_set(0);

        move(0, 0);

        const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

        int numLines = static_cast<int>(m_buffer->getFileGapBuffer().numberOfLines());

        int extraLinesFromWrapping = 0;

        int maxRender = std::min(LINES, numLines);
        for (int row = 0; row < maxRender; row++)
        {
            wmove(virtualWindow, row + extraLinesFromWrapping, 0);

            wclrtoeol(virtualWindow);

            const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(row);

            if (!lineGapBuffer) { break; }

            size_t lineSize = lineGapBuffer->lineSize();

            int indexOfFirstNonSpace = 0;

            for (size_t column = 0; column < lineSize; column++)
            {
                wmove(virtualWindow, row + extraLinesFromWrapping, column);

                char character = lineGapBuffer->at(column);

                if (indexOfFirstNonSpace == 0 && character != ' ') { indexOfFirstNonSpace = column; }

                waddch(virtualWindow, character);
            }

            int newLines = lineSize / COLS;
            extraLinesFromWrapping += lineSize / COLS;
            maxRender -= newLines;

        }

        for (int i = maxRender; i < LINES; i++)
        {
            wmove(virtualWindow, maxRender, 0);
            clrtoeol();
        }

        overwrite(virtualWindow, stdscr);

        wmove(stdscr, cursorPos.first, cursorPos.second);

        curs_set(1);

        wrefresh(stdscr);

        delwin(virtualWindow);
    }
}

void View::display()
{
    if (m_buffer->getFileGapBuffer().bufferSize())
    {
        move(0, 0);

        curs_set(0);

        const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

        int numLines = static_cast<int>(m_buffer->getFileGapBuffer().numberOfLines());

        int extraLinesFromWrapping = 0;

        int maxRender = std::min(LINES, numLines);
        for (int row = 0; row < maxRender; row++)
        {
            move(row + extraLinesFromWrapping, 0);

            const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(row);

            if (!lineGapBuffer) { break; }

            size_t lineSize = lineGapBuffer->lineSize();

            int indexOfFirstNonSpace = 0;

            for (size_t column = 0; column < lineSize; column++)
            {
                move(row + extraLinesFromWrapping, column);

                char character = lineGapBuffer->at(column);

                if (mvinch(row + extraLinesFromWrapping, column) != static_cast<size_t>(character))
                {
                    addch(character);
                }

                // if (indexOfFirstNonSpace == 0 && character != ' ') { indexOfFirstNonSpace = column; }
            }

            move(row + extraLinesFromWrapping, lineSize);

            int newLines = lineSize / COLS;

            if (!newLines) { clrtoeol(); }

            extraLinesFromWrapping += lineSize / COLS;
            maxRender -= newLines;

        }

        for (int i = maxRender; i < LINES; i++)
        {
            move(i, 0);
            clrtoeol();
        }

        move(cursorPos.first, cursorPos.second);

        refresh();

        curs_set(1);
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
