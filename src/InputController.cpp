#include "InputController.h"
#include "Buffer.h"
#include "Editor.h"
#include "Command.h"

#include <ncurses.h>
#include <string>
#include <sstream>
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
    if (input == COLON)
    {
        clearRepetitionBuffer();
        m_editor->commandQueue().execute<SetModeCommand>(1, COMMAND_MODE, 0);
        return;
    }

    if (m_commandBuffer == "d")
    {
        handleDeleteCommands(input);
        return;
    }
    else if (m_commandBuffer == "f")
    {
        handleFindCommand(input);
        return;
    }

    switch (input)
    {
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
            break;
        case CTRL_R:
            m_editor->commandQueue().execute<RedoCommand>(1);
            break;
        case x:
            if (repeatedInput(input)) { m_editor->commandQueue().overrideRepetitionQueue(); }
            m_editor->commandQueue().execute<RemoveCharacterNormalCommand>(repetitionCount(), false);
            break;
        case X:
            if (repeatedInput(input)) { m_editor->commandQueue().overrideRepetitionQueue(); }
            m_editor->commandQueue().execute<RemoveCharacterNormalCommand>(repetitionCount(), true);
            break;
        case A:
            m_editor->commandQueue().execute<CursorFullRightCommand>(1);
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE, 1);
            break;
        case r:
            m_editor->commandQueue().execute<SetModeCommand>(1, REPLACE_CHAR_MODE, 0);
            break;
        case o:
            m_editor->commandQueue().execute<InsertLineNormalCommand>(1, true);
            m_editor->commandQueue().overrideRepetitionQueue();
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE, 0);
            break;
        case O:
            m_editor->commandQueue().execute<InsertLineNormalCommand>(1, false);
            m_editor->commandQueue().overrideRepetitionQueue();
            m_editor->commandQueue().execute<SetModeCommand>(1, INSERT_MODE, 0);
            break;
        case d:
            m_commandBuffer.push_back('d');
            break;
        case f:
            m_commandBuffer.push_back('f');
            break;
        case SEMICOLON:
            m_editor->commandQueue().execute<FindCharacterCommand>(repetitionCount(), m_findCharacter);
            break;
        case w:
            m_editor->commandQueue().execute<JumpWordCommand>(repetitionCount());
            break;
        case W:
            m_editor->commandQueue().execute<JumpSymbolCommand>(repetitionCount());
            break;
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
                // clear();
                // move(0, 0);
                // printw("You printed: %c with integer code: %d", input, input);
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
            handleCommandBufferInput();
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
            m_editor->commandQueue().execute<RemoveCharacterInsertCommand>(1);
            break;
        case ENTER:
            m_editor->commandQueue().overrideOverrideRepetitionBuffer();
            m_editor->commandQueue().overrideRepetitionQueue();
            m_editor->commandQueue().execute<InsertLineInsertCommand>(1);
            break;
        case TAB:
            m_editor->commandQueue().overrideRepetitionQueue();
            m_editor->commandQueue().execute<TabCommand>(1);
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

void InputController::handleCommandBufferInput()
{
    std::istringstream istream(m_commandBuffer);

    std::string currentSubstring;

    while (istream >> currentSubstring)
    {
        if (currentSubstring == "q!")
        {
            m_editor->quit();
            break;
        }
        else if (currentSubstring == "w")
        {
            if (m_editor->buffer().fileName() != "NO_NAME") { m_editor->buffer().saveCurrentFile(); }
            else { /* TODO: Send signal that no file is set; */ }

            break;
        }
        else if (currentSubstring == "wq")
        {
            if (m_editor->buffer().fileName() != "NO_NAME") { m_editor->buffer().saveCurrentFile(); }
            else { /* TODO: Send signal that no file is set; */ }

            m_editor->quit();

            break;
        }
        else if (currentSubstring == "write")
        {
            std::string fileName;
            istream >> fileName;

            if (m_editor->buffer().fileName() == "NO_NAME") { m_editor->buffer().setFileName(fileName); }

            m_editor->buffer().writeToFile(fileName);

            break;
        }
        else
        {
            printw("Not an editor command: %s", m_commandBuffer.c_str());
            m_commandBuffer.clear();
            break;
        }
    }

    m_commandBuffer.clear();

    m_editor->commandQueue().execute<SetModeCommand>(1, NORMAL_MODE, 0);

    return;
}

void InputController::handleDeleteCommands(int input)
{
    size_t rep = std::min(repetitionCount(), static_cast<int>(m_editor->buffer().getFileGapBuffer().numberOfLines()));

    switch (input)
    {
        case d:
            m_editor->commandQueue().execute<RemoveLineCommand>(rep);
            break;
        default:
            // TODO: Send signal that command isn't valid
            break;
    }

    m_commandBuffer.clear();
}

void InputController::handleFindCommand(int input)
{
    size_t rep = std::min(repetitionCount(), static_cast<int>(m_editor->buffer().getFileGapBuffer().numberOfLines()));

    m_editor->commandQueue().execute<FindCharacterCommand>(rep, static_cast<char>(input));

    m_findCharacter = static_cast<char>(input);

    m_commandBuffer.clear();
}
