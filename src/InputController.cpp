#include "InputController.h"
#include "Buffer.h"
#include "Editor.h"
#include "Command.h"
#include "Includes.h"

InputController::InputController(Editor* editor)
    : m_editor(editor), m_commandBuffer(""), m_repetitionBuffer(""), m_circularInputBuffer(INPUT_CONTROLLER_MAX_CIRCULAR_BUFFER_SIZE)
{
    // m_editor->commandQueue().execute<InsertCharacterCommand>(false, 1, ' ');
    // m_editor->commandQueue().execute<RemoveCharacterNormalCommand>(false, 1, true);


    m_keys = {
        CTRL_C, TAB, ENTER, CTRL_R, CTRL_V, CTRL_W, ESCAPE, SPACE, QUOTE, APOSTROPHE,
        LEFT_PARENTHESIS, COMMA, SEMICOLON, LESS_THAN_SIGN, GREATER_THAN_SIGN, A, B, C, D, E, F,
        G, H, I, J, N, O, P, Q, R, S, T, U, V, W, X, LEFT_BRACKET, a, b, d, d, e, f, g, h, i, j, k, k, k, k, k, n,
        o, p, q, r, s, t, u, v, w, x, y, LEFT_BRACE, BACKSPACE };


    // std::random_device rd;

    // unsigned int seed = rd();

    m_distribution = std::uniform_int_distribution<int>(0, static_cast<int>(m_keys.size()));

    m_numberGenerator.seed(m_seed);
}

int InputController::getRandomKey() const
{
    int index = m_distribution(m_numberGenerator);

    MODE currentMode = m_editor->mode();

    do
    {
        index = m_distribution(m_numberGenerator);
    }
    while (currentMode == INSERT_MODE &&
             (m_keys[index] != CTRL_C && (m_keys[index] < 32 || m_keys[index] > 126)));


    return m_keys[index];
}

int InputController::repetitionCount()
{
    int repetitionCount = atoi(m_repetitionBuffer.c_str());

    clearRepetitionBuffer();

    return std::clamp(repetitionCount, 1, MAX_REPETITION_COUNT);
}

void InputController::handleInput()
{
    int input = 0;

    if (m_numberOfRandomInputs-- <= 0)
    {
        // endwin();
        // std::cout << "Last input: " << m_lastInput << '\n';
        // exit(1);
        input = getch();
    }
    else
    {
        if (m_editor->mode() == INSERT_MODE)
        {
            if (m_numberOfInsertModeInserts > 0)
            {
                input = getRandomKey();
                m_lastInput = input;

                m_numberOfInsertModeInserts--;
            }
            else
            {
                input = CTRL_C;
                m_lastInput = input;

                m_numberOfInsertModeInserts = 5;
            }
        }
        else
        {
            input = getRandomKey();
            m_lastInput = input;
        }


        // if (m_numberOfInputRepetitions <= 0)
        // {
        //     input = getRandomKey();
        //     m_lastInput = input;
        //     m_numberOfInputRepetitions = 0;
        // }
        // else
        // {
        //     input = m_lastInput;
        //     m_numberOfInputRepetitions--;
        // }
    }

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
        m_circularInputBuffer.add(input);

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
    else if (currentMode == VISUAL_MODE || currentMode == VISUAL_LINE_MODE || currentMode == VISUAL_BLOCK_MODE)
    {
        m_circularInputBuffer.add(input);
        m_editor->view().displayCircularInputBuffer();

        handleVisualModes(input);
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
        m_editor->view().displayCommandBuffer();
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
    else if (m_commandBuffer == "g")
    {
        handleGoCommands(input);
        return;
    }
    else if (m_commandBuffer == "y")
    {
        handleYankCommands(input);
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
            m_editor->commandQueue().execute<QuickVerticalMovementCommand>(false, 1, true);
            break;
        case P:
            m_editor->commandQueue().execute<QuickVerticalMovementCommand>(false, 1, false);
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
        case g:
            m_commandBuffer.push_back('g');
            break;
        case y:
            m_commandBuffer.push_back('y');
            break;
        case SEMICOLON:
            m_editor->commandQueue().execute<FindCharacterCommand>(false, repetitionCount(), m_findCharacter, m_searchedForward);
            break;
        case COMMA:
            m_editor->commandQueue().execute<FindCharacterCommand>(false, repetitionCount(), m_findCharacter, !m_searchedForward);
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
        case k:
            m_editor->commandQueue().execute<PasteCommand>(false, repetitionCount());
            break;
        case LESS_THAN_SIGN:
        {
            int repetition = repetitionCount();

            if (repetition > 1)
            {
                m_editor->commandQueue().execute<TabLineCommand>(false, repetition, false);
            }
            else
            {
                m_editor->commandQueue().execute<TabLineCommand>(true, 1, false);
            }

            break;
        }
        case GREATER_THAN_SIGN:
        {
            int repetition = repetitionCount();

            if (repetition > 1)
            {
                m_editor->commandQueue().execute<TabLineCommand>(false, repetition, true);
            }
            else
            {
                m_editor->commandQueue().execute<TabLineCommand>(true, 1, true);
            }
        }
            break;
        case v:
        {
            clearRepetitionBuffer();

            const std::pair<int, int>& cursorPos = m_editor->buffer().getCursorPos();
            m_cursorPosOnVisualMode = cursorPos;

            m_editor->commandQueue().execute<SetModeCommand>(false, 1, VISUAL_MODE, 0);
            break;
        }
        case V:
        {
            clearRepetitionBuffer();

            const std::pair<int, int>& cursorPos = m_editor->buffer().getCursorPos();
            m_cursorPosOnVisualMode = cursorPos;

            m_editor->commandQueue().execute<SetModeCommand>(false, 1, VISUAL_LINE_MODE, 0);
            break;
        }
        case CTRL_V:
        {
            clearRepetitionBuffer();

            const std::pair<int, int>& cursorPos = m_editor->buffer().getCursorPos();
            m_cursorPosOnVisualMode = cursorPos;

            m_editor->commandQueue().execute<SetModeCommand>(false, 1, VISUAL_BLOCK_MODE, 0);
            break;
        }
        case t:
            m_editor->commandQueue().execute<ToggleCommentLineCommand>(false, 1);
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
    switch (input)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
            m_commandBuffer.clear();
            m_editor->view().display();
            return;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
            m_commandBuffer.clear();
            break;
        case ENTER:
            handleCommandBufferInput();
            m_editor->view().display();
            return;
        case CTRL_W:
        {
            if (m_commandBuffer == "") { return; }

            bool foundCharacter = false;

            int numCharacters = static_cast<int>(m_commandBuffer.size()) - 1;
            for (int i = numCharacters; i >= -1; i--)
            {
                char currentCharacter = m_commandBuffer[i];

                if ((foundCharacter && currentCharacter == ' ') || (i == -1))
                {
                    m_commandBuffer.erase(i + 1, numCharacters - i);
                    break;
                }
                else if (!foundCharacter && currentCharacter != ' ')
                {
                    foundCharacter = true;
                }
            }
            break;
        }
        case BACKSPACE:
            if (m_commandBuffer == "")
            {
                m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
                m_editor->view().display();
                return;
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

    m_editor->view().displayCommandBuffer();
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
            m_editor->commandQueue().execute<InsertLineInsertCommand>(true, 1);
            break;
        case TAB:
            m_editor->commandQueue().execute<TabCommand>(true, 1);
            break;
        case LEFT_PARENTHESIS:
            m_editor->commandQueue().execute<AutocompletePair>(true, 1, '(');
            break;
        case LEFT_BRACKET:
            m_editor->commandQueue().execute<AutocompletePair>(true, 1, '[');
            break;
        case LEFT_BRACE:
            m_editor->commandQueue().execute<AutocompletePair>(true, 1, '{');
            break;
        case APOSTROPHE:
            m_editor->commandQueue().execute<AutocompletePair>(true, 1, '\'');
            break;
        case QUOTE:
            m_editor->commandQueue().execute<AutocompletePair>(true, 1, '"');
            break;
        case CTRL_W:
        {
            m_editor->commandQueue().execute<JumpCursorDeletePreviousWordInsertModeCommand>(true, 1);
            break;
        }
        default:
        if (input >= 32 && input <= 126)
        {
            m_editor->commandQueue().execute<InsertCharacterCommand>(true, 1, input);
            break;
        }
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
                m_editor->commandQueue().execute<ReplaceCharacterCommand>(false, 1, input, false);
            }
            else
            {
                m_editor->commandQueue().execute<ReplaceCharacterCommand>(false, repetition, input, true);
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
        else if (currentSubstring == "q")
        {
            if (m_lastSavedCommand == m_editor->commandQueue().currentCommandCount())
            {
                m_editor->quit();
            }
            else
            {
                displayErrorMessage("No write since last change");
            }
        }
        else if (currentSubstring == "w")
        {
            if (m_editor->buffer().filePath() != "NO_NAME")
            {
                m_editor->buffer().saveCurrentFile();
                m_lastSavedCommand = m_editor->commandQueue().currentCommandCount();
            }
            else
            {
                displayErrorMessage("No file name. Run :write 'filename'");
            }

            break;
        }
        else if (currentSubstring == "wq")
        {
            if (m_editor->buffer().filePath() != "NO_NAME")
            {
                m_editor->buffer().saveCurrentFile();
            }
            else
            {
                displayErrorMessage("No file name. Run :write 'filename'");
            }

            m_editor->quit();

            break;
        }
        else if (currentSubstring == "write")
        {
            std::string fileName;
            if (!(istream >> fileName))
            {
                displayErrorMessage("Invalid file name");
                break;
            }

            if (m_editor->buffer().filePath() == "NO_NAME") { m_editor->buffer().setFileName(fileName); }

            m_editor->buffer().writeToFile(fileName);

            if (fileName ==  m_editor->buffer().filePath())
            {
                m_lastSavedCommand = m_editor->commandQueue().currentCommandCount();
            }

            break;
        }
        else
        {
            bool isIntegral = true;

            for (size_t i = 0; i < currentSubstring.size(); i++)
            {
                if (!std::isdigit(currentSubstring[i]))
                {
                    isIntegral = false;
                }
            }

            if (isIntegral)
            {
                size_t lineNumber = stol(currentSubstring);

                if (lineNumber)
                {
                    // -1 at end for 1-based indexing on line jumps
                    m_editor->buffer().shiftCursorY(lineNumber - m_editor->buffer().getCursorPos().first - 1);

                    m_editor->view().display();

                    break;
                }
            }

            displayErrorMessage("Not an editor command: " + m_commandBuffer);
            break;
        }
    }

    m_commandBuffer.clear();

    m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);

    return;
}

void InputController::handleDeleteCommands(int input)
{
    int numberOfLines = static_cast<int>(m_editor->buffer().getFileGapBuffer().numberOfLines());
    int rep = std::min(repetitionCount(), numberOfLines);

    switch (input)
    {
        case d:
            m_editor->commandQueue().execute<RemoveLineCommand>(false, rep);
            break;
        case w:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_FORWARD | JUMP_BY_WORD, false);
            break;
        case W:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_FORWARD, false);
            break;
        case s:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_BY_WORD, false);
            break;
        case S:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, 0, false);
            break;
        case e:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END, false);
            break;
        case E:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_FORWARD | JUMP_TO_END, false);
            break;
        case q:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_BY_WORD | JUMP_TO_END, false);
            break;
        case Q:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, rep, JUMP_TO_END, false);
            break;
        case i:
        {
            int modifiedRep = std::min(rep + 1, numberOfLines);

            if (m_editor->buffer().getCursorPos().first == numberOfLines - 1) { break; }

            m_editor->commandQueue().execute<RemoveLineCommand>(false, modifiedRep);
            break;
        }
        case p:
        {
            int modifiedrep = std::min(rep + 1, numberOfLines);

            const std::pair<int, int>& cursorPos = m_editor->buffer().getCursorPos();

            if (cursorPos.first == 0) { break; }

            bool onLastLine = (cursorPos.first == numberOfLines - 1);

            for (int i = 0; i < modifiedrep; i++)
            {
                if (!onLastLine && i > 0) { m_editor->buffer().shiftCursorY(-1); }
                m_editor->commandQueue().execute<RemoveLineCommand>(true, 1);
            }

            break;
        }
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
        case c:
            m_editor->commandQueue().execute<RemoveLineToInsertCommand>(false, 1);
            break;
        case w:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_FORWARD | JUMP_BY_WORD, true);
            break;
        case W:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_FORWARD, true);
            break;
        case s:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_BY_WORD, true);
            break;
        case S:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, 0, true);
            break;
        case e:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END, true);
            break;
        case E:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_FORWARD | JUMP_TO_END, true);
            break;
        case q:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_BY_WORD | JUMP_TO_END, true);
            break;
        case Q:
            m_editor->commandQueue().execute<JumpCursorDeleteWordCommand>(false, 1, JUMP_TO_END, true);
            break;
        default:
            // TODO: Send signal that command isn't valid
            break;
    }

    m_commandBuffer.clear();
}

void InputController::displayErrorMessage(const std::string& message)
{
    m_commandBuffer = message;

    m_editor->view().displayCommandBuffer(COLOR_PAIR(ERROR_MESSAGE_PAIR));

    // getch();
}

void InputController::handleVisualModes(int input)
{
    MODE currentMode = m_editor->mode();

    switch (input)
    {
        case CTRL_C:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
            break;
        case ESCAPE:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, NORMAL_MODE, 0);
            break;
        case COLON:
            m_editor->commandQueue().execute<SetModeCommand>(false, 1, COMMAND_MODE, 0);
            m_editor->view().displayCommandBuffer();
            break;
        case v:
            if (currentMode != VISUAL_MODE)
            {
                m_editor->commandQueue().execute<SetModeCommand>(false, 1, VISUAL_MODE, 0);
            }
            break;
        case V:
            if (currentMode != VISUAL_LINE_MODE)
            {
                m_editor->commandQueue().execute<SetModeCommand>(false, 1, VISUAL_LINE_MODE, 0);
            }
            break;
        case CTRL_V:
            if (currentMode != VISUAL_BLOCK_MODE)
            {
                m_editor->commandQueue().execute<SetModeCommand>(false, 1, VISUAL_BLOCK_MODE, 0);
            }
            break;
        case y:
            switch (currentMode)
            {
                case VISUAL_MODE:
                    m_editor->commandQueue().execute<VisualYankCommand>(false, 1, VISUAL_YANK);
                    break;
                case VISUAL_LINE_MODE:
                    m_editor->commandQueue().execute<VisualYankCommand>(false, 1, LINE_YANK);
                    break;
                case VISUAL_BLOCK_MODE:
                    m_editor->commandQueue().execute<VisualYankCommand>(false, 1, BLOCK_YANK);
                    break;
                default:
                    endwin();
                    std::cerr << "Unexpected mode: " << currentMode << '\n';
                    exit(1);
            }
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
            m_editor->commandQueue().execute<QuickVerticalMovementCommand>(false, 1, true);
            break;
        case P:
            m_editor->commandQueue().execute<QuickVerticalMovementCommand>(false, 1, false);
            break;
        case d:
            switch (currentMode)
            {
                case VISUAL_LINE_MODE:
                    m_editor->commandQueue().execute<RemoveLinesVisualLineModeCommand>(false, 1);
                    break;
                case VISUAL_MODE:
                    m_editor->commandQueue().execute<RemoveLinesVisualModeCommand>(false, 1);
                    break;
                case VISUAL_BLOCK_MODE:
                    m_editor->commandQueue().execute<RemoveLinesVisualBlockModeCommand>(false, 1);
                    break;
                default:
                    endwin();
                    std::cerr << "Unexpected mode: " << currentMode << '\n';
                    exit(1);
            }
            break;
        case c:
            switch (currentMode)
            {
                case VISUAL_LINE_MODE:
                    m_editor->commandQueue().execute<RemoveLinesVisualLineModeCommand>(false, 1);
                    m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
                    break;
                case VISUAL_MODE:
                    m_editor->commandQueue().execute<RemoveLinesVisualModeCommand>(false, 1);
                    m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
                    break;
                case VISUAL_BLOCK_MODE:
                    m_editor->commandQueue().execute<RemoveLinesVisualBlockModeCommand>(false, 1);
                    m_editor->commandQueue().execute<SetModeCommand>(false, 1, INSERT_MODE, 0);
                    break;
                default:
                    endwin();
                    std::cerr << "Unexpected mode: " << currentMode << '\n';
                    exit(1);
            }
            break;
        case LESS_THAN_SIGN:
        {
            int repetition = repetitionCount();

            if (repetition > 1)
            {
                m_editor->commandQueue().execute<TabLineVisualCommand>(false, repetition, false);
            }
            else
            {
                m_editor->commandQueue().execute<TabLineVisualCommand>(true, 1, false);
            }

            break;
        }
        case GREATER_THAN_SIGN:
        {
            int repetition = repetitionCount();

            if (repetition > 1)
            {
                m_editor->commandQueue().execute<TabLineVisualCommand>(false, repetition, true);
            }
            else
            {
                m_editor->commandQueue().execute<TabLineVisualCommand>(true, 1, true);
            }
            break;
        }
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
        case g:
            m_editor->commandQueue().execute<CursorFullTopCommand>(false, 1);
            break;
        case G:
            m_editor->commandQueue().execute<CursorFullBottomCommand>(false, 1);
            break;
        case t:
            m_editor->commandQueue().execute<ToggleCommentLinesVisualCommand>(false, 1);
            break;
    }
}

void InputController::handleGoCommands(int input)
{
    clearRepetitionBuffer();

    switch (input)
    {
        case p:
            m_editor->commandQueue().execute<CursorFullTopCommand>(false, 1);
            break;
        case i:
            m_editor->commandQueue().execute<CursorFullBottomCommand>(false, 1);
            break;
    }

    m_commandBuffer.clear();
}

void InputController::handleYankCommands(int input)
{
    switch (input)
    {
        case y:
            m_editor->commandQueue().execute<NormalYankLineCommand>(false, 1, 0, repetitionCount());
            break;
        case p:
            m_editor->commandQueue().execute<NormalYankLineCommand>(false, 1, -1, repetitionCount());
            break;
        case i:
            m_editor->commandQueue().execute<NormalYankLineCommand>(false, 1, 1, repetitionCount());
            break;
        case w:
            m_editor->commandQueue().execute<JumpCursorYankWordCommand>(false, 1, JUMP_FORWARD | JUMP_BY_WORD, repetitionCount());
            break;
        case W:
            m_editor->commandQueue().execute<JumpCursorYankWordCommand>(false, 1, JUMP_FORWARD, repetitionCount());
            break;
        case s:
            m_editor->commandQueue().execute<JumpCursorYankWordCommand>(false, 1, JUMP_BY_WORD, repetitionCount());
            break;
        case S:
            m_editor->commandQueue().execute<JumpCursorYankWordCommand>(false, 1, 0, repetitionCount());
            break;
        case e:
            m_editor->commandQueue().execute<JumpCursorYankWordCommand>(false, 1, JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END, repetitionCount());
            break;
        case E:
            m_editor->commandQueue().execute<JumpCursorYankWordCommand>(false, 1, JUMP_FORWARD | JUMP_TO_END, repetitionCount());
            break;
        case q:
            m_editor->commandQueue().execute<JumpCursorYankWordCommand>(false, 1, JUMP_BY_WORD | JUMP_TO_END, repetitionCount());
            break;
        case Q:
            m_editor->commandQueue().execute<JumpCursorYankWordCommand>(false, 1, JUMP_TO_END, repetitionCount());
            break;
        case QUOTE:
            m_editor->commandQueue().execute<JumpCursorYankEndlineCommand>(false, 1, true);
            clearRepetitionBuffer();
            break;
        case H:
            m_editor->commandQueue().execute<JumpCursorYankEndlineCommand>(false, 1, false);
            clearRepetitionBuffer();
            break;

    }

    m_commandBuffer.clear();
}
