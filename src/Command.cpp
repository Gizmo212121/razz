#include "Command.h"
#include "Editor.h"

void QuitCommand::undo() { }
bool QuitCommand::execute()
{
    m_editor->quit();
    return false;
}


void SetModeCommand::undo() { }
bool SetModeCommand::execute()
{
    m_editor->setMode(m_mode);
    return false;
}


void MoveCursorXCommand::undo() {}
bool MoveCursorXCommand::execute()
{
    m_buffer->shiftCursorX(deltaX);
    return false;
}

void MoveCursorYCommand::undo() {}
bool MoveCursorYCommand::execute()
{
    m_buffer->shiftCursorY(deltaY);
    return false;
}

void UndoCommand::undo() { }
bool UndoCommand::execute()
{
    m_commandQueue->undo();
    return false;
}

void RedoCommand::undo() { }
bool RedoCommand::execute()
{
    m_commandQueue->redo();
    return false;
}

void CursorFullRightCommand::undo() { }
bool CursorFullRightCommand::execute()
{
    m_buffer->shiftCursorFullRight();
    return false;
}

void CursorFullLeftCommand::undo() { }
bool CursorFullLeftCommand::execute()
{
    m_buffer->shiftCursorFullLeft();
    return false;
}

void CursorFullTopCommand::undo() { }
bool CursorFullTopCommand::execute()
{
    m_buffer->shiftCursorFullTop();
    return false;
}

void CursorFullBottomCommand::undo() { }
bool CursorFullBottomCommand::execute()
{
    m_buffer->shiftCursorFullBottom();
    return false;
}
