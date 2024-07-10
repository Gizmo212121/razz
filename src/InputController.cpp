#include "InputController.h"
#include "Editor.h"
#include "Command.h"

#include <ncurses.h>
#include <string>

InputController::InputController(Editor* editor)
    : m_editor(editor), m_commandBuffer("")
{
}

void InputController::handleInput()
{
    if (m_editor->mode() == NORMAL_MODE)
    {
        handleNormalModeInput();
    }
    else if (m_editor->mode() == COMMAND_MODE)
    {
        handleCommandModeInput();
    }
    else if (m_editor->mode() == INSERT_MODE)
    {
        handleInsertModeInput();
    }
    else
    {
        std::cerr << "Unexpected mode: " << m_editor->mode() << std::endl;
        exit(1);
    }
}

void InputController::handleNormalModeInput()
{
    int getch = getch();

    switch (getch)
    {
        case ESCAPE:
            printw("bozo");
            refresh();
            m_editor->commandQueue().execute<QuitCommand>(1, m_editor);
            break;
        case COLON:
            m_editor->commandQueue().execute<SetModeCommand>(1, m_editor, COMMAND_MODE);
            break;
        default:
            clear();
            move(0, 0);
            printw("You printed: %c with integer code: %d", getch, getch);
            break;
    }
}

void InputController::handleCommandModeInput()
{
    int getch = getch();

    if (getch == CTRL_C)
    {
        m_editor->commandQueue().execute<SetModeCommand>(1, m_editor, NORMAL_MODE);
    }

    switch (getch)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(1, m_editor, NORMAL_MODE);
            m_commandBuffer.clear();
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(1, m_editor, NORMAL_MODE);
            m_commandBuffer.clear();
            break;
        case ENTER:
        {
            if (m_commandBuffer == "q!")
            {
                m_editor->commandQueue().execute<QuitCommand>(1, m_editor);
            }
            else
            {
                printw("Not an editor command: %s", m_commandBuffer.c_str());
                m_commandBuffer.clear();
                m_editor->commandQueue().execute<SetModeCommand>(1, m_editor, NORMAL_MODE);
            }

            break;
        }
        case BACKSPACE:
        {
            if (m_commandBuffer == "")
            {
                m_editor->commandQueue().execute<SetModeCommand>(1, m_editor, NORMAL_MODE);
            }
            else
            {
                m_commandBuffer.pop_back();
            }

            break;
        }
        default:
            m_commandBuffer.push_back(static_cast<char>(getch));
            break;
    }
}

void InputController::handleInsertModeInput()
{

}
