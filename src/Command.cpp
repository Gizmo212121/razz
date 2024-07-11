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
    m_buffer->insertCharacter(m_character, m_y, m_x);

    m_view->display();
}
void InsertCharacterCommand::undo()
{
    m_buffer->removeCharacter(m_y, m_x);

    m_view->display();
}
bool InsertCharacterCommand::execute()
{
    m_buffer->insertCharacter(m_character);

    std::pair<int, int> cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    m_view->display();
    return true;
}
