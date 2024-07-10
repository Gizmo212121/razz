#include "Command.h"
#include "Editor.h"

QuitCommand::QuitCommand(Editor* editor)
    : m_editor(editor)
{

}

void QuitCommand::undo()
{

}

bool QuitCommand::execute()
{
    m_editor->quit();
    return false;
}


SetModeCommand::SetModeCommand(Editor* editor, MODE mode)
    : m_editor(editor), m_mode(mode)
{

}

void SetModeCommand::undo()
{

}

bool SetModeCommand::execute()
{
    m_editor->setMode(m_mode);
    return false;
}
