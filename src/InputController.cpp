#include "InputController.h"
#include "Editor.h"
#include "Command.h"

#include <ncurses.h>
#include <string>

InputController::InputController(Editor* editor)
    : m_editor(editor), m_commandBuffer(""), m_repetitionBuffer("1"), m_typingIntoRepetitionBuffer(false)
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
            m_editor->commandQueue().execute<QuitCommand>(1);
            break;
        case COLON:
            clearRepetitionBuffer();
            m_editor->commandQueue().execute<SetModeCommand>(1, COMMAND_MODE);
            break;
        case j:
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE);
            break;
        case h:
            m_editor->commandQueue().execute<MoveCursorXCommand>(atoi(m_repetitionBuffer.c_str()), -1);
            clearRepetitionBuffer();
            break;
        case i:
            m_editor->commandQueue().execute<MoveCursorYCommand>(atoi(m_repetitionBuffer.c_str()), 1);
            clearRepetitionBuffer();
            break;
        case p:
            m_editor->commandQueue().execute<MoveCursorYCommand>(atoi(m_repetitionBuffer.c_str()), -1);
            clearRepetitionBuffer();
            break;
        case APOSTROPHE:
            m_editor->commandQueue().execute<MoveCursorXCommand>(atoi(m_repetitionBuffer.c_str()), 1);
            clearRepetitionBuffer();
            break;
        case H:
            m_editor->commandQueue().execute<CursorFullLeftCommand>(1);
            break;
        case QUOTE:
            m_editor->commandQueue().execute<CursorFullRightCommand>(1);
            break;
        case I:
            m_editor->commandQueue().execute<CursorFullBottomCommand>(1);
            break;
        case P:
            m_editor->commandQueue().execute<CursorFullTopCommand>(1);
            break;
        case u:
            m_editor->commandQueue().execute<UndoCommand>(1);
            break;
        case CTRL_R:
            m_editor->commandQueue().execute<RedoCommand>(1);
            break;
        default:
        {
            if (getch >= '0' && getch <= '9')
            {
                if (!m_typingIntoRepetitionBuffer && getch == '0' && m_repetitionBuffer == "0") { clearRepetitionBuffer(); break; }
                else if (!m_typingIntoRepetitionBuffer) { m_repetitionBuffer[0] = static_cast<char>(getch); }
                else { m_repetitionBuffer.push_back(static_cast<char>(getch)); }

                m_typingIntoRepetitionBuffer = true;
            }
            else
            {
                // clear();
                // move(0, 0);
                // printw("You printed: %c with integer code: %d", getch, getch);
            }

            break;
        }
    }
}

void InputController::handleCommandModeInput()
{
    int getch = getch();

    if (getch == CTRL_C)
    {
        m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE);
    }

    switch (getch)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE);
            m_commandBuffer.clear();
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE);
            m_commandBuffer.clear();
            break;
        case ENTER:
        {
            if (m_commandBuffer == "q!")
            {
                m_editor->commandQueue().execute<QuitCommand>(1);
            }
            else
            {
                printw("Not an editor command: %s", m_commandBuffer.c_str());
                m_commandBuffer.clear();
                m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE);
            }

            break;
        }
        case BACKSPACE:
        {
            if (m_commandBuffer == "")
            {
                m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE);
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
    int getch = getch();

    switch (getch)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE);
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE);
            break;
        case BACKSPACE:
            // m_editor->commandQueue().execute<RemoveCharacterSendCursorLeftCommand>(1);
            break;
        default:
            m_editor->commandQueue().execute<InsertCharacterCommand>(1, getch);
            break;

    }
}
