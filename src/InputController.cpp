#include "InputController.h"
#include "Buffer.h"
#include "Editor.h"
#include "Command.h"

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

    // Global input
    switch (input)
    {
        case KEY_RESIZE:
            m_editor->view().display();
            return;
    }


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
        exit_curses(-1);
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
        m_editor->commandQueue().execute<SetModeCommand>(false, 1, COMMAND_MODE, 0);
        return;
    }

    if (m_commandBuffer == "d")
    {
        handleDeleteCommands(input);
        return;
    }
    else if (m_commandBuffer == "f" || m_commandBuffer == "F")
    {
        handleFindCommand(input);
        return;
    }
    else if (m_commandBuffer == "c")
    {
        handleDeleteToInsertCommands(input);
        return;
    }

    switch (input)
    {
        case CTRL_C:
            clearRepetitionBuffer();
            break;
        case j:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case a:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 1);
            break;
        case h:
            m_editor->commandQueue().execute<MoveCursorXCommand>(false, 1, -1 * repetitionCount());
            break;
        case i:
            m_editor->commandQueue().execute<MoveCursorYCommand>(false, 1, 1 * repetitionCount());
            break;
        case p:
            m_editor->commandQueue().execute<MoveCursorYCommand>(false, 1, -1 * repetitionCount());
            break;
        case APOSTROPHE:
            m_editor->commandQueue().execute<MoveCursorXCommand>(false, 1, 1 * repetitionCount());
            break;
        case H:
            m_editor->commandQueue().execute<CursorFullLeftCommand>(false, 1);
            break;
        case QUOTE:
            m_editor->commandQueue().execute<CursorFullRightCommand>(false, 1);
            break;
        case I:
            m_editor->commandQueue().execute<CursorFullBottomCommand>(false, 1);
            break;
        case P:
            m_editor->commandQueue().execute<CursorFullTopCommand>(false, 1);
            break;
        case u:
            m_editor->commandQueue().execute<UndoCommand>(false, 1);
            break;
        case CTRL_R:
            m_editor->commandQueue().execute<RedoCommand>(false, 1);
            break;
        case x:
            m_editor->commandQueue().execute<RemoveCharacterNormalCommand>(true, repetitionCount(), false);
            break;
        case X:
            m_editor->commandQueue().execute<RemoveCharacterNormalCommand>(true, repetitionCount(), true);
            break;
        case A:
            m_editor->commandQueue().execute<CursorFullRightCommand>(false, 1);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 1);
            break;
        case J:
            m_editor->commandQueue().execute<CursorFullLeftCommand>(false, 1);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case r:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, REPLACE_CHAR_MODE, 0);
            break;
        case o:
            m_editor->commandQueue().execute<InsertLineNormalCommand>(true, 1, true);
            // m_editor->commandQueue().execute<InsertLineNormalCommand>(false, 1, true);
            break;
        case O:
            m_editor->commandQueue().execute<InsertLineNormalCommand>(true, 1, false);
            break;
        case d:
            m_commandBuffer.push_back('d');
            break;
        case f:
            m_commandBuffer.push_back('f');
            break;
        case F:
            m_commandBuffer.push_back('F');
            break;
        case SEMICOLON:
            m_editor->commandQueue().execute<FindCharacterCommand>(false, repetitionCount(), m_findCharacter, m_searchedForward);
            break;
        case w:
            m_editor->commandQueue().execute<JumpCursorCommand>(false, repetitionCount(), JUMP_FORWARD | JUMP_BY_WORD);
            break;
        case W:
            m_editor->commandQueue().execute<JumpCursorCommand>(false, repetitionCount(), JUMP_FORWARD);
            break;
        case s:
            m_editor->commandQueue().execute<JumpCursorCommand>(false, repetitionCount(), JUMP_BY_WORD);
            break;
        case S:
            m_editor->commandQueue().execute<JumpCursorCommand>(false, repetitionCount(), 0);
            break;
        case e:
            m_editor->commandQueue().execute<JumpCursorCommand>(false, repetitionCount(), JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END);
            break;
        case E:
            m_editor->commandQueue().execute<JumpCursorCommand>(false, repetitionCount(), JUMP_FORWARD | JUMP_TO_END);
            break;
        case q:
            m_editor->commandQueue().execute<JumpCursorCommand>(false, repetitionCount(), JUMP_BY_WORD | JUMP_TO_END);
            break;
        case Q:
            m_editor->commandQueue().execute<JumpCursorCommand>(false, repetitionCount(), JUMP_TO_END);
            break;
        case c:
            m_commandBuffer.push_back('c');
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
        m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
    }

    switch (input)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
            m_commandBuffer.clear();
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
            m_commandBuffer.clear();
            break;
        case ENTER:
            handleCommandBufferInput();
            break;
        case BACKSPACE:
            if (m_commandBuffer == "")
            {
                m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
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
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, -1);
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, -1);
            break;
        case BACKSPACE:
            m_editor->commandQueue().execute<RemoveCharacterInsertCommand>(repeatedInput(input), 1);
            break;
        case ENTER:
            // m_editor->commandQueue().execute<InsertLineInsertCommand>(repeatedInput(input), 1);
            m_editor->commandQueue().execute<InsertLineInsertCommand>(true, 1);
            break;
        case TAB:
            m_editor->commandQueue().execute<TabCommand>(true, 1);
            break;
        case CTRL_W:
        {
            m_editor->commandQueue().execute<JumpCursorDeletePreviousWordInsertModeCommand>(true, 1);

            const std::pair<int, int> cursorPos = m_editor->buffer().getCursorPos();
            m_editor->buffer().moveCursor(cursorPos.first, cursorPos.second + 1);
            break;
        }
        default:
            m_editor->commandQueue().execute<InsertCharacterCommand>(true, 1, input);
            break;

    }
}

void InputController::handleReplaceCharMode(int input)
{
    switch (input)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
            break;
        default:
        {
            const std::pair<int, int>& cursorPos = m_editor->buffer().getCursorPos();
            int lineSize = static_cast<int>(m_editor->buffer().getLineGapBuffer(cursorPos.first)->lineSize());
            int repetition = std::min(repetitionCount(), lineSize - cursorPos.second);

            if (repetition == 1)
            {
                m_editor->commandQueue().execute<ReplaceCharacterCommand>(false, 1, input);
            }
            else
            {
                for (int i = 0; i < repetition; i++)
                {
                    m_editor->commandQueue().execute<ReplaceCharacterCommand>(true, 1, input);

                    if (cursorPos.second == lineSize - 1) { break; }

                    m_editor->buffer().shiftCursorX(1);
                }
            }

            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);

            break;
        }
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
            int lineNumber = atoi(currentSubstring.c_str());

            if (lineNumber)
            {
                // -1 at end for 1-based indexing on line jumps
                m_editor->buffer().shiftCursorY(lineNumber - m_editor->buffer().getCursorPos().first - 1);

                m_editor->view().display();

                break;
            }

            printw("Not an editor command: %s", m_commandBuffer.c_str());
            m_commandBuffer.clear();
            break;
        }
    }

    m_commandBuffer.clear();

    m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);

    return;
}

void InputController::handleDeleteCommands(int input)
{
    size_t rep = std::min(repetitionCount(), static_cast<int>(m_editor->buffer().getFileGapBuffer().numberOfLines()));

    switch (input)
    {
        case d:
            m_editor->commandQueue().execute<RemoveLineCommand>(false, rep);
            break;
        case w:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_FORWARD | JUMP_BY_WORD);
            break;
        case W:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_FORWARD);
            break;
        case s:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_BY_WORD);
            break;
        case S:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, 0);
            break;
        case e:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END);
            break;
        case E:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_FORWARD | JUMP_TO_END);
            break;
        case q:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_BY_WORD | JUMP_TO_END);
            break;
        case Q:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_TO_END);
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

    if (m_commandBuffer == "f")
    {
        m_editor->commandQueue().execute<FindCharacterCommand>(false, rep, static_cast<char>(input), true);

        m_searchedForward = true;
    }
    else
    {
        m_editor->commandQueue().execute<FindCharacterCommand>(false, rep, static_cast<char>(input), false);

        m_searchedForward = false;
    }

    m_findCharacter = static_cast<char>(input);

    m_commandBuffer.clear();
}

void InputController::handleDeleteToInsertCommands(int input)
{
    clearRepetitionBuffer();

    switch (input)
    {
        case w:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_FORWARD | JUMP_BY_WORD);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case W:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_FORWARD);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case s:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_BY_WORD);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case S:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, 0);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case e:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case E:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_FORWARD | JUMP_TO_END);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case q:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_BY_WORD | JUMP_TO_END);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        case Q:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_TO_END);
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
            break;
        default:
            // TODO: Send signal that command isn't valid
            break;
    }

    m_commandBuffer.clear();
}
