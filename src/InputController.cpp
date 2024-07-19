#include "InputController.h"
#include "Editor.h"
#include "Command.h"

#include <ncurses.h>
#include <string>
#include <algorithm>


InputController::InputController(Editor* editor)
    : m_editor(editor), m_commandBuffer(""), m_repetitionBuffer("")
{
}

int InputController::repetitionCount()
{
    int repetitionCount = atoi(m_repetitionBuffer.c_str());

    clearRepetitionBuffer();

    return std::clamp(repetitionCount, 1, MAX_REPETITION_COUNT);
}

void InputController::handleInput()
{
    int input = getch();

    MODE currentMode = m_editor->mode();

    if (currentMode == NORMAL_MODE)
    {
        handleNormalModeInput(input);
    }
    else if (currentMode == COMMAND_MODE)
    {
        handleCommandModeInput(input);
    }
    else if (currentMode == INSERT_MODE)
    {
        handleInsertModeInput(input);
    }
    else if (currentMode == REPLACE_CHAR_MODE)
    {
        handleReplaceCharMode(input);
    }
    else
    {
        std::cerr << "Unexpected mode: " << m_editor->mode() << std::endl;
        exit(1);
    }

    m_previousInput = input;
    m_previousMode = currentMode;
}

void InputController::handleNormalModeInput(int input)
{
    switch (input)
    {
        case ESCAPE:
            printw("bozo");
            refresh();
            m_editor->commandQueue().execute<QuitCommand>(1);
            break;
        case COLON:
            clearRepetitionBuffer();
            m_editor->commandQueue().execute<SetModeCommand>(1, COMMAND_MODE, 0);
            break;
        case j:
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE, 0);
            break;
        case a:
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE, 1);
            break;
        case h:
            m_editor->commandQueue().execute<MoveCursorXCommand>(1, -1 * repetitionCount());
            break;
        case i:
            m_editor->commandQueue().execute<MoveCursorYCommand>(1, 1 * repetitionCount());
            break;
        case p:
            m_editor->commandQueue().execute<MoveCursorYCommand>(1, -1 * repetitionCount());
            break;
        case APOSTROPHE:
            m_editor->commandQueue().execute<MoveCursorXCommand>(1, 1 * repetitionCount());
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

            // endwin();
            // m_editor->commandQueue().printRepetitionQueue();
            // exit(1);
            break;
        case CTRL_R:
            m_editor->commandQueue().execute<RedoCommand>(1);
            break;
        case x:
            if (repeatedInput(input)) { m_editor->commandQueue().overrideRepetitionQueue(); }
            m_editor->commandQueue().execute<RemoveCharacterCommand>(repetitionCount(), false);
            break;
        case X:
            if (repeatedInput(input)) { m_editor->commandQueue().overrideRepetitionQueue(); }
            m_editor->commandQueue().execute<RemoveCharacterCommand>(repetitionCount(), true);
            break;
        case A:
            m_editor->commandQueue().execute<CursorFullRightCommand>(1);
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE, 1);
            break;
        case r:
            m_editor->commandQueue().execute<SetModeCommand>(1, REPLACE_CHAR_MODE, 0);
            break;
        case o:
            m_editor->commandQueue().execute<InsertLineCommand>(1, true);
            m_editor->commandQueue().overrideRepetitionQueue();
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE, 0);
            break;
        case O:
            m_editor->commandQueue().execute<InsertLineCommand>(1, false);
            m_editor->commandQueue().overrideRepetitionQueue();
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE, 0);
            break;
        case d:
        {
            size_t rep = std::min(repetitionCount(), static_cast<int>(m_editor->buffer().getFileGapBuffer().numberOfLines()));
            m_editor->commandQueue().execute<DeleteLineCommand>(rep);
            break;
        }
        default:
        {
            if (input >= '0' && input <= '9')
            {
                m_repetitionBuffer.push_back(static_cast<char>(input));

                if (m_repetitionBuffer == "0")
                {
                    clearRepetitionBuffer();
                    break;
                }
            }
            else
            {
                clear();
                move(0, 0);
                printw("You printed: %c with integer code: %d", input, input);
            }

            break;
        }
    }
}

void InputController::handleCommandModeInput(int input)
{
    if (input == CTRL_C)
    {
        m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);
    }

    switch (input)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);
            m_commandBuffer.clear();
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);
            m_commandBuffer.clear();
            break;
        case ENTER:
            if (m_commandBuffer == "q!")
            {
                m_editor->commandQueue().execute<QuitCommand>(1);
            }
            else
            {
                printw("Not an editor command: %s", m_commandBuffer.c_str());
                m_commandBuffer.clear();
                m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);
            }

            break;
        case BACKSPACE:
            if (m_commandBuffer == "")
            {
                m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);
            }
            else
            {
                m_commandBuffer.pop_back();
            }

            break;
        default:
            m_commandBuffer.push_back(static_cast<char>(input));
            break;
    }
}

void InputController::handleInsertModeInput(int input)
{
    switch (input)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, -1);
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, -1);
            break;
        case BACKSPACE:
            if (repeatedInput(input)) { m_editor->commandQueue().overrideRepetitionQueue(); }
            m_editor->commandQueue().execute<RemoveCharacterCommand>(1, true);
            break;
        case ENTER:
            m_editor->commandQueue().execute<InsertLineCommand>(1, true);
            m_editor->commandQueue().overrideRepetitionQueue();
            break;
        case TAB:
            m_editor->commandQueue().overrideRepetitionQueue();
            m_editor->commandQueue().execute<InsertCharacterCommand>(4, SPACE);
            break;
        default:
            m_editor->commandQueue().overrideRepetitionQueue();
            m_editor->commandQueue().execute<InsertCharacterCommand>(1, input);
            break;

    }
}

void InputController::handleReplaceCharMode(int input)
{
    switch (input)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);
            break;
        default:
            int repetition = repetitionCount();

            if (repetition == 1)
            {
                m_editor->commandQueue().execute<ReplaceCharacterCommand>(1, input);
            }
            else
            {
                for (int i = 0; i < repetition; i++)
                {
                    m_editor->commandQueue().execute<ReplaceCharacterCommand>(1, input);
                    m_editor->commandQueue().overrideRepetitionQueue();

                    m_editor->commandQueue().execute<MoveCursorXCommand>(1, 1);
                    m_editor->commandQueue().overrideRepetitionQueue();

                    const std::pair<int, int>& cursorPos = m_editor->buffer().getCursorPos();

                    if (static_cast<size_t>(cursorPos.second) == m_editor->buffer().getLineGapBuffer(cursorPos.first)->lineSize()) { break; }
                }
            }

            m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);

            break;
    }
}
