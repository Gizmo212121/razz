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

void View::display()
{
    if (m_buffer->getFileGapBuffer().bufferSize())
    {
        clear();

        move(0, 0);

        const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

        for (size_t row = 0; row < m_buffer->getFileGapBuffer().numberOfLines(); row++)
        {
            if (m_buffer->getLineGapBuffer(row))
            {
                for (size_t column = 0; column < m_buffer->getLineGapBuffer(row)->lineSize(); column++)
                {
                    move(row, column);
                    addch(m_buffer->getLineGapBuffer(row)->at(column));
                }
            }
        }

        move(60, 0);
        printw("Size: %lu", m_buffer->getFileGapBuffer().bufferSize());
        move(61, 0);
        printw("Pre Index: %lu", m_buffer->getFileGapBuffer().preGapIndex());
        move(62, 0);
        printw("Post Index: %lu", m_buffer->getFileGapBuffer().postGapIndex());

        move(cursorPos.first, cursorPos.second);

        refresh();
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

        move(cursorPos.first + m_buffer->getFileGapBuffer().preGapIndex(), cursorPos.second);

        refresh();
    }
}

void View::displayCurrentLine(int y)
{
    // std::pair<int, int> cursorPos = m_buffer->getCursorPos();
    // move(y, 0);
    // clrtoeol();
    //
    // for (size_t column = 0; column < m_buffer->getLineGapBuffer(y)->lineSize(); column++)
    // {
    //     addch(m_buffer->getLineGapBuffer(y)->at(column));
    // }
    //
    // move(cursorPos.first, cursorPos.second);
    //
    // refresh();
}

void View::displayCurrentLineGapBuffer(int y)
{
    std::pair<int, int> cursorPos = m_buffer->getCursorPos();
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
