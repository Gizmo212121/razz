#include "Command.h"
#include "Buffer.h"
#include "Editor.h"
#include "LineGapBuffer.h"

void SetModeCommand::redo() {}
void SetModeCommand::undo() {}
bool SetModeCommand::execute()
{
    m_editor->setMode(m_mode);

    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_buffer->moveCursor(cursorPos.first, cursorPos.second + m_cursorOffset);

    if (m_mode == INSERT_MODE)
    {
        m_view->insertCursor();
    }
    else if (m_mode == REPLACE_CHAR_MODE)
    {
        m_view->replaceCursor();
    }
    else
    {
        m_view->normalCursor();
    }

    return false;
}

void MoveCursorXCommand::redo() {}
void MoveCursorXCommand::undo() {}
bool MoveCursorXCommand::execute()
{
    m_buffer->shiftCursorX(deltaX);
    return false;
}

void MoveCursorYCommand::redo() {}
void MoveCursorYCommand::undo() {}
bool MoveCursorYCommand::execute()
{
    m_buffer->shiftCursorY(deltaY);
    return false;
}

void UndoCommand::redo() {}
void UndoCommand::undo() { }
bool UndoCommand::execute()
{
    m_commandQueue->undo();
    return false;
}

void RedoCommand::redo() {}
void RedoCommand::undo() { }
bool RedoCommand::execute()
{
    m_commandQueue->redo();
    return false;
}

void CursorFullRightCommand::redo() {}
void CursorFullRightCommand::undo() { }
bool CursorFullRightCommand::execute()
{
    m_buffer->shiftCursorFullRight();
    return false;
}

void CursorFullLeftCommand::redo() {}
void CursorFullLeftCommand::undo() { }
bool CursorFullLeftCommand::execute()
{
    m_buffer->shiftCursorFullLeft();
    return false;
}

void CursorFullTopCommand::redo() {}
void CursorFullTopCommand::undo() { }
bool CursorFullTopCommand::execute()
{
    m_buffer->shiftCursorFullTop();
    return false;
}

void CursorFullBottomCommand::redo() {}
void CursorFullBottomCommand::undo() { }
bool CursorFullBottomCommand::execute()
{
    m_buffer->shiftCursorFullBottom();
    return false;
}

void InsertCharacterCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x - 1);
    m_buffer->insertCharacter(m_character);
    m_buffer->shiftCursorX(-1);

    m_view->displayCurrentLine(m_y);
}
void InsertCharacterCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->removeCharacter();
    m_buffer->shiftCursorX(-1);

    m_view->displayCurrentLine(m_y);
}
bool InsertCharacterCommand::execute()
{
    m_buffer->insertCharacter(m_character);

    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    m_view->displayCurrentLine(m_y);
    return true;
}

void RemoveCharacterNormalCommand::redo()
{
    if (m_cursorLeft)
    {
        m_buffer->moveCursor(m_y, m_x);
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x);
    }

    m_buffer->removeCharacter(m_cursorLeft);
}
void RemoveCharacterNormalCommand::undo()
{
    if (m_cursorLeft)
    {
        m_buffer->moveCursor(m_y, m_x - 1);
        m_buffer->insertCharacter(m_character);
        m_view->displayCurrentLine(m_y);
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x);
        m_buffer->insertCharacter(m_character);

        m_buffer->shiftCursorX(-1);

        m_view->displayCurrentLine(m_y);
    }
}
bool RemoveCharacterNormalCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_buffer->getLineGapBuffer(m_y)->lineSize() <= 0) { return false; }
    if (m_cursorLeft && m_x == 0) { return false; }

    m_character = m_buffer->removeCharacter(m_cursorLeft);
    return true;
}

void RemoveCharacterInsertCommand::redo()
{
    if (m_x == 0)
    {
        m_buffer->moveCursor(m_y, m_x);

        m_buffer->deleteLine();

        m_buffer->moveCursor(m_y - 1, m_buffer->getLineGapBuffer(m_y - 1)->lineSize());

        size_t lineSize = m_line->lineSize();
        for (size_t column = 0; column < lineSize; column++)
        { 
            m_buffer->insertCharacter(m_line->at(column));
        }

        if (lineSize > 0)
        {
            m_buffer->shiftCursorX(- lineSize);
        }

        m_view->displayFromCurrentLineOnwards(m_y - 1);
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x);

        m_buffer->removeCharacter(true);

        if (m_character == ' ')
        {
            for (int i = 1; i < m_deletedWhitespaces; i++)
            {
                m_buffer->removeCharacter(true);
            }
        }

        m_view->displayCurrentLine(m_y);
    }
}
void RemoveCharacterInsertCommand::undo()
{
    if (m_x == 0)
    {
        m_buffer->moveCursor(m_y - 1, m_buffer->getLineGapBuffer(m_y - 1)->lineSize() - 1);

        int lineLength = static_cast<int>(m_line->lineSize());

        for (int x = 0; x < lineLength; x++)
        {
            m_buffer->removeCharacter(false);
        }

        m_buffer->insertLine(m_line, true);

        m_buffer->moveCursor(m_y, 0);

        m_view->displayFromCurrentLineOnwards(m_y - 1);
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x);

        for (int i = 0; i < m_deletedWhitespaces; i++)
        {
            m_buffer->insertCharacter(' ');
        }

        if (!m_deletedWhitespaces) { m_buffer->insertCharacter(m_character); m_buffer->shiftCursorX(0); }

        m_view->displayCurrentLine(m_y);
    }
}
bool RemoveCharacterInsertCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_x == 0)
    {
        if (m_y == 0) { return false; }

        m_line = m_buffer->deleteLine();

        m_buffer->moveCursor(m_y - 1, m_buffer->getLineGapBuffer(m_y - 1)->lineSize());

        size_t lineSize = m_line->lineSize();
        for (size_t column = 0; column < lineSize; column++)
        { 
            m_buffer->insertCharacter(m_line->at(column));
        }

        if (lineSize > 0)
        {

            m_buffer->shiftCursorX(- lineSize);
        }

        m_view->displayFromCurrentLineOnwards(m_y - 1);
    }
    else
    {
        m_character = m_buffer->removeCharacter(true);

        if (m_character == ' ')
        {
            m_deletedWhitespaces++;

            for (int i = 1; i < WHITESPACE_PER_TAB; i++)
            {
                const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(m_y);

                int index = std::max(0, m_x - i - 1);
                if (lineGapBuffer->at(index) == ' ')
                {
                    m_deletedWhitespaces++;
                    m_character = m_buffer->removeCharacter(true);
                }
                else { break; }
            }
        }

        m_view->displayCurrentLine(m_y);
    }

    return true;
}

void ReplaceCharacterCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->replaceCharacter(m_character);
    m_view->displayCurrentLine(m_y);
}
void ReplaceCharacterCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->replaceCharacter(m_replacedCharacter);
    m_view->displayCurrentLine(m_y);
}
bool ReplaceCharacterCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    m_replacedCharacter = m_buffer->replaceCharacter(m_character);

    if (m_character == m_replacedCharacter) { return false; }

    m_view->displayCurrentLine(m_y);

    return true;
}

void InsertLineNormalCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->insertLine(true);

    m_view->displayFromCurrentLineOnwards(m_y);
}
void InsertLineNormalCommand::undo()
{
    m_buffer->moveCursor(m_y + 1 * m_down, m_x);
    m_buffer->deleteLine();
    m_buffer->moveCursor(m_y, m_x);

    m_view->displayFromCurrentLineOnwards(m_y);
}
bool InsertLineNormalCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    m_buffer->insertLine(m_down);

    for (int i = 0; i < m_buffer->getXPositionOfFirstCharacter(m_y); i++)
    {
        m_buffer->insertCharacter(' ');
    }

    m_view->displayFromCurrentLineOnwards(m_y);

    return true;
}

void InsertLineInsertCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->insertLine(true);
    m_view->displayFromCurrentLineOnwards(m_y);
}
void InsertLineInsertCommand::undo()
{
    // m_buffer->moveCursor(m_y + 1 * m_down, m_x, false);
    m_buffer->deleteLine();
    m_buffer->moveCursor(m_y, m_x);
    m_view->displayFromCurrentLineOnwards(m_y);
}
bool InsertLineInsertCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    m_buffer->insertLine(true);

    for (int i = 0; i < m_buffer->getXPositionOfFirstCharacter(m_y); i++)
    {
        m_buffer->insertCharacter(' ');
    }

    m_view->displayFromCurrentLineOnwards(m_y);

    return true;
}

void DeleteLineCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->deleteLine();

    m_view->displayFromCurrentLineOnwards(m_y);
}
void DeleteLineCommand::undo()
{
    int targetY;
    if (m_y == 0) { targetY = 0; }
    else { targetY = m_y; }

    m_buffer->moveCursor(targetY, m_x);
    m_buffer->insertLine(m_line, false);
    m_buffer->moveCursor(m_y, m_x);

    m_view->displayFromCurrentLineOnwards(targetY);
}
bool DeleteLineCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_buffer->getFileGapBuffer().numberOfLines() == 1 && m_buffer->getLineGapBuffer(m_y)->lineSize() == 0) { return false; }

    m_line = m_buffer->deleteLine();

    m_view->displayFromCurrentLineOnwards(m_y);

    return true;
}

void TabCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->insertCharacter(' '); }
    m_view->displayCurrentLine(m_y);
}
void TabCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->removeCharacter(false); }
    m_view->displayCurrentLine(m_y);
}
bool TabCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->insertCharacter(' '); }

    m_view->displayCurrentLine(m_y);

    return true;
}
