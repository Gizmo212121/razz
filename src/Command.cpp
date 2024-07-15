#include "Command.h"
#include "Editor.h"

void QuitCommand::redo() {}
void QuitCommand::undo() {}
bool QuitCommand::execute()
{
    m_editor->quit();
    return false;
}

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
    m_buffer->moveCursor(m_y, m_x - 1, false);
    m_buffer->insertCharacter(m_character);
    m_buffer->shiftCursorX(-1, false);

    m_view->displayCurrentLine(m_y);
}
void InsertCharacterCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x, false);
    m_buffer->removeCharacter();
    m_buffer->shiftCursorX(-1, false);

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
        m_buffer->moveCursor(m_y, m_x - 1, false);
        m_buffer->insertCharacter(m_character);
        m_view->displayCurrentLine(m_y);
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x, false);
        m_buffer->insertCharacter(m_character);

        if (m_x != static_cast<int>(m_buffer->getGapBuffer(m_y).lineSize()) - 1)
        {
            m_buffer->shiftCursorX(-1, false);
        }

        m_view->displayCurrentLine(m_y);
    }
}
bool RemoveCharacterNormalCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_buffer->getGapBuffer(m_y).lineSize() <= 0) { return false; }
    if (m_cursorLeft && m_x == 0) { return false; }

    m_character = m_buffer->removeCharacter(m_cursorLeft);
    return true;
}

void ReplaceCharacterCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->replaceCharacter(m_character);
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

    move(60, 0);
    printw("REPLACED CHAR: %c", m_replacedCharacter);
    refresh();

    return true;
}
