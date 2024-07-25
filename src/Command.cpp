#include "Command.h"
#include "Buffer.h"
#include "Editor.h"
#include "InputController.h"
#include "LineGapBuffer.h"
#include <ncurses.h>

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
    const std::pair<int, int> cursorPos = m_buffer->getCursorPos();
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(cursorPos.first);
    int lineSize = static_cast<int>(lineGapBuffer->lineSize());

    if (lineSize)
    {
        bool lineIsAllSpaces = true;
        for (int i = lineSize - 1; i >= 0; i--)
        {
            if (lineGapBuffer->at(i) != ' ')
            {
                lineIsAllSpaces = false;
                break;
            }
        }

        if (lineIsAllSpaces)
        {
            m_buffer->moveCursor(cursorPos.first, 0);
            for (int i = 0; i < lineSize; i++)
            {
                m_buffer->removeCharacter(false);
            }
        }
    }

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
    m_buffer->moveCursor(m_y, m_x - 1);
    m_buffer->insertCharacter(m_character);
    m_buffer->shiftCursorX(-1);

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
    // if (m_renderExecute) { m_view->display(); }
}
void InsertCharacterCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->removeCharacter();
    m_buffer->shiftCursorX(-1);

    if (m_renderUndo) { m_view->displayCurrentLine(m_y); }
    // if (m_renderUndo) { m_view->display(); }
}
bool InsertCharacterCommand::execute()
{
    m_buffer->insertCharacter(m_character);

    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
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

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
}
void RemoveCharacterNormalCommand::undo()
{
    if (m_cursorLeft)
    {
        m_buffer->moveCursor(m_y, m_x - 1);
        m_buffer->insertCharacter(m_character);
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x);
        m_buffer->insertCharacter(m_character);

        m_buffer->shiftCursorX(-1);
    }

    if (m_renderUndo) { m_view->displayCurrentLine(m_y); }
}
bool RemoveCharacterNormalCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_buffer->getLineGapBuffer(m_y)->lineSize() <= 0) { return false; }
    if (m_cursorLeft && m_x == 0) { return false; }

    m_character = m_buffer->removeCharacter(m_cursorLeft);

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }

    return true;
}

void RemoveCharacterInsertCommand::redo()
{
    if (m_x == 0)
    {
        m_buffer->moveCursor(m_y, m_x);

        m_buffer->removeLine();

        m_buffer->moveCursor(m_y - 1, m_buffer->getLineGapBuffer(m_y - 1)->lineSize());

        size_t lineSize = m_line->lineSize();
        for (size_t column = 0; column < lineSize; column++)
        { 
            m_buffer->insertCharacter(m_line->at(column));
        }

        if (lineSize > 0)
        {
            m_buffer->shiftCursorX(- lineSize);
        }

        if (m_renderExecute) { m_view->displayFromCurrentLineOnwards(m_y - 1); }
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x);

        m_buffer->removeCharacter(true);

        if (m_character == ' ')
        {
            for (int i = 1; i < m_deletedWhitespaces; i++)
            {
                m_buffer->removeCharacter(true);
            }
        }

        if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
    }
}
void RemoveCharacterInsertCommand::undo()
{
    if (m_x == 0)
    {
        m_buffer->moveCursor(m_y - 1, m_buffer->getLineGapBuffer(m_y - 1)->lineSize() - 1);

        int lineLength = static_cast<int>(m_line->lineSize());

        for (int x = 0; x < lineLength; x++)
        {
            m_buffer->removeCharacter(false);
        }

        m_buffer->insertLine(m_line, true);

        m_buffer->moveCursor(m_y, 0);

        // if (m_renderUndo) { m_view->displayFromCurrentLineOnwards(m_y - 1); }
        if (m_renderUndo) { m_view->display(); }
    }
    else
    {
        for (int i = 0; i < m_deletedWhitespaces; i++)
        {
            m_buffer->moveCursor(m_y, m_x - m_deletedWhitespaces);
            m_buffer->insertCharacter(' ');
        }

        if (m_deletedWhitespaces > 0)
        {
            m_buffer->moveCursor(m_y, m_x);
        }
        else
        {
            m_buffer->moveCursor(m_y, m_x - 1);
        }

        if (!m_deletedWhitespaces) { m_buffer->insertCharacter(m_character); m_buffer->shiftCursorX(0); }

        if (m_renderUndo) { m_view->displayCurrentLine(m_y); }
    }
}
bool RemoveCharacterInsertCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_x == 0)
    {
        if (m_y == 0) { return false; }

        m_line = m_buffer->removeLine();

        m_buffer->moveCursor(m_y - 1, m_buffer->getLineGapBuffer(m_y - 1)->lineSize());

        size_t lineSize = m_line->lineSize();
        for (size_t column = 0; column < lineSize; column++)
        { 
            m_buffer->insertCharacter(m_line->at(column));
        }

        if (lineSize > 0)
        {

            m_buffer->shiftCursorX(- lineSize);
        }

        if (m_renderExecute) { m_view->displayFromCurrentLineOnwards(m_y - 1); }
    }
    else
    {
        m_character = m_buffer->removeCharacter(true);

        if (m_character == ' ')
        {
            m_deletedWhitespaces++;

            const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(m_y);

            for (int i = 1; i < WHITESPACE_PER_TAB; i++)
            {
                int index = std::max(0, m_x - i - 1);
                if (lineGapBuffer->at(index) == ' ')
                {
                    m_deletedWhitespaces++;
                    m_character = m_buffer->removeCharacter(true);
                }
                else { break; }
            }
        }

        if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
    }

    return true;
}

void ReplaceCharacterCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->replaceCharacter(m_character);
    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
}
void ReplaceCharacterCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->replaceCharacter(m_replacedCharacter);
    if (m_renderUndo) { m_view->displayCurrentLine(m_y); }
}
bool ReplaceCharacterCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    m_replacedCharacter = m_buffer->replaceCharacter(m_character);

    if (m_character == m_replacedCharacter) { return false; }

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }

    return true;
}

void InsertLineNormalCommand::redo()
{
    m_buffer->moveCursor(m_y, 0);

    for (int i = 0; i < m_deletedSpaces; i++)
    {
        m_buffer->removeCharacter(false);
    }

    m_buffer->moveCursor(m_y, m_x);

    m_buffer->insertLine(m_down);

    if (m_down)
    {
        for (int i = 0; i < m_buffer->getXPositionOfFirstCharacter(m_y); i++)
        {
            m_buffer->insertCharacter(' ');
        }
    }
    else
    {
        for (int i = 0; i < m_buffer->getXPositionOfFirstCharacter(m_y + 1); i++)
        {
            m_buffer->insertCharacter(' ');
        }
    }

    if (m_renderExecute) { m_view->display(); }
}
void InsertLineNormalCommand::undo()
{
    m_buffer->moveCursor(m_y, 0);
    for (int i = 0; i < m_deletedSpaces; i++)
    {
        m_buffer->insertCharacter(' ');
    }

    m_buffer->moveCursor(m_y + 1 * m_down, m_x);
    m_buffer->removeLine();

    m_buffer->moveCursor(m_y, m_x);

    if (m_renderUndo) { m_view->display(); }
}
bool InsertLineNormalCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(m_y);
    int lineSize = static_cast<int>(lineGapBuffer->lineSize());

    if (lineSize)
    {
        bool lineIsAllSpaces = true;
        for (int i = lineSize - 1; i >= 0; i--)
        {
            if (lineGapBuffer->at(i) != ' ')
            {
                lineIsAllSpaces = false;
                break;
            }
        }

        if (lineIsAllSpaces)
        {
            m_buffer->moveCursor(m_y, 0);
            for (int i = 0; i < lineSize; i++)
            {
                m_deletedSpaces++;
                m_buffer->removeCharacter(false);
            }
        }
    }

    m_buffer->insertLine(m_down);

    if (m_down)
    {
        for (int i = 0; i < m_buffer->getXPositionOfFirstCharacter(m_y); i++)
        {
            m_buffer->insertCharacter(' ');
        }
    }
    else
    {
        for (int i = 0; i < m_buffer->getXPositionOfFirstCharacter(m_y + 1); i++)
        {
            m_buffer->insertCharacter(' ');
        }
    }

    m_editor->setMode(INSERT_MODE);
    m_view->insertCursor();

    // if (m_renderExecute) { m_view->displayFromCurrentLineOnwards(m_y); }
    if (m_renderExecute) { m_view->display(); }

    return true;
}

void InsertLineInsertCommand::redo()
{
    m_buffer->moveCursor(m_y, 0);
    for (int i = 0; i < m_deletedSpaces; i++)
    {
        m_buffer->removeCharacter(false);
    }

    m_buffer->moveCursor(m_y, m_x);

    for (size_t i = 0; i < m_distanceToEndLine; i++)
    {
        m_buffer->removeCharacter(false);
    }

    m_buffer->insertLine(true);

    for (int i = 0; i < m_xPositionOfFirstCharacter; i++)
    {
        m_buffer->insertCharacter(' ');
    }

    for (size_t i = 0; i < m_distanceToEndLine; i++)
    {
        m_buffer->insertCharacter(m_characters[i]);

    }

    if (m_deletedSpaces)
    {
        m_buffer->moveCursor(m_y + 1, m_x - 1);
    }
    else
    {
        m_buffer->shiftCursorFullLeft();
    }

    if (m_renderExecute) { m_view->display(); }
}
void InsertLineInsertCommand::undo()
{
    m_buffer->moveCursor(m_y + 1, 0);

    m_buffer->removeLine();

    m_buffer->moveCursor(m_y, m_x);

    for (int i = 0; i < m_deletedSpaces; i++)
    {
        m_buffer->insertCharacter(' ');
    }

    for (size_t i = 0; i < m_characters.size(); i++)
    {
        m_buffer->insertCharacter(m_characters[i]);
    }

    m_buffer->moveCursor(m_y, m_x);
    m_buffer->shiftCursorX(0);

    if (m_renderUndo) { m_view->display(); }
}
bool InsertLineInsertCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

    m_x = cursorPos.second;
    m_y = cursorPos.first;

    m_xPositionOfFirstCharacter = m_buffer->getXPositionOfFirstCharacter(m_y);

    m_distanceToEndLine = m_buffer->getLineGapBuffer(m_y)->lineSize() - m_x;

    m_buffer->moveCursor(m_y, m_x);

    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(m_y);
    int lineSize = static_cast<int>(lineGapBuffer->lineSize());

    if (lineSize)
    {
        bool lineIsAllSpaces = true;
        for (int i = lineSize - 1; i >= 0; i--)
        {
            if (lineGapBuffer->at(i) != ' ')
            {
                lineIsAllSpaces = false;
                break;
            }
        }

        if (lineIsAllSpaces)
        {
            m_buffer->moveCursor(m_y, 0);
            for (int i = 0; i < lineSize; i++)
            {
                m_deletedSpaces++;
                m_buffer->removeCharacter(false);
            }
        }
    }

    m_characters.resize(m_distanceToEndLine);

    for (size_t i = 0; i < m_distanceToEndLine; i++)
    {
        m_characters[i] = m_buffer->removeCharacter(false);
    }


    m_buffer->insertLine(true);

    for (int i = 0; i < m_xPositionOfFirstCharacter; i++)
    {
        m_buffer->insertCharacter(' ');
    }

    for (size_t i = 0; i < m_distanceToEndLine; i++)
    {
        m_buffer->insertCharacter(m_characters[i]);
    }

    m_buffer->moveCursor(m_y + 1, m_xPositionOfFirstCharacter);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void RemoveLineCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->removeLine();

    if (m_renderExecute) { m_view->display(); }
}
void RemoveLineCommand::undo()
{
    int targetY;
    if (m_y == 0) { targetY = 0; }
    else { targetY = m_y; }

    m_buffer->moveCursor(targetY, m_x);
    m_buffer->insertLine(m_line, false);
    m_buffer->moveCursor(m_y, m_x);

    if (m_renderUndo) { m_view->display(); }
}
bool RemoveLineCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_buffer->getFileGapBuffer().numberOfLines() == 1 && m_buffer->getLineGapBuffer(m_y)->lineSize() == 0) { m_view->display(); return false; }

    m_line = m_buffer->removeLine();
    m_buffer->shiftCursorX(0);

    if (m_renderExecute) { m_view->displayFromCurrentLineOnwards(m_y); }

    return true;
}

void TabCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->insertCharacter(' '); }
    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
}
void TabCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->removeCharacter(false); }
    if (m_renderUndo) { m_view->displayCurrentLine(m_y); }
}
bool TabCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->insertCharacter(' '); }

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }

    return true;
}

void FindCharacterCommand::redo() {}
void FindCharacterCommand::undo() {}
bool FindCharacterCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

    m_buffer->moveCursor(cursorPos.first, m_buffer->findCharacterIndex(m_character, m_searchForward));

    return false;
}

void JumpCursorCommand::redo() {}
void JumpCursorCommand::undo() {}
bool JumpCursorCommand::execute()
{
    int cursorY = m_buffer->getCursorPos().first;

    switch (m_jumpCode)
    {
        case JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END:
            m_buffer->moveCursor(cursorY, m_buffer->endNextWordIndex());
            break;
        case JUMP_FORWARD | JUMP_BY_WORD:
            m_buffer->moveCursor(cursorY, m_buffer->beginningNextWordIndex());
            break;
        case JUMP_FORWARD | JUMP_TO_END:
            m_buffer->moveCursor(cursorY, m_buffer->endNextSymbolIndex());
            break;
        case JUMP_FORWARD:
            m_buffer->moveCursor(cursorY, m_buffer->beginningNextSymbolIndex());
            break;
        case JUMP_BY_WORD | JUMP_TO_END:
            m_buffer->moveCursor(cursorY, m_buffer->endPreviousWordIndex());
            break;
        case JUMP_BY_WORD:
            m_buffer->moveCursor(cursorY, m_buffer->beginningPreviousWordIndex());
            break;
        case JUMP_TO_END:
            m_buffer->moveCursor(cursorY, m_buffer->endPreviousSymbolIndex());
            break;
        case 0:
            m_buffer->moveCursor(cursorY, m_buffer->beginningPreviousSymbolIndex());
            break;
        default:
            exit_curses(0);
            std::cerr << "Unexpected cursor jump code: " << m_jumpCode << '\n';
            exit(1);
    }

    return false;
}

void JumpCursorDeleteWordCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);

    if (m_differenceX >= 0)
    {
        for (int i = 0; i < m_differenceX; i++)
        {
            m_buffer->removeCharacter(false);
        }
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x + m_differenceX);

        for (int i = 0; i < - m_differenceX; i++)
        {
            m_buffer->removeCharacter(false);
        }
    }

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
}
void JumpCursorDeleteWordCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);

    if (m_differenceX >= 0)
    {
        for (int i = 0; i < m_differenceX; i++)
        {
            m_buffer->insertCharacter(m_characters[i]);
        }
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x + m_differenceX);

        for (int i = 0; i < - m_differenceX; i++)
        {
            m_buffer->insertCharacter(m_characters[i]);
        }
    }

    m_buffer->moveCursor(m_y, m_x);
    m_buffer->shiftCursorX(0);

    if (m_renderUndo) { m_view->displayCurrentLine(m_y); }
}
bool JumpCursorDeleteWordCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    int targetX;

    switch (m_jumpCode)
    {
        case JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END:
            targetX = m_buffer->endNextWordIndex();
            break;
        case JUMP_FORWARD | JUMP_BY_WORD:
            targetX = m_buffer->beginningNextWordIndex();
            break;
        case JUMP_FORWARD | JUMP_TO_END:
            targetX = m_buffer->endNextSymbolIndex();
            break;
        case JUMP_FORWARD:
            targetX = m_buffer->beginningNextSymbolIndex();
            break;
        case JUMP_BY_WORD | JUMP_TO_END:
            targetX = m_buffer->endPreviousWordIndex();
            break;
        case JUMP_BY_WORD:
            targetX = m_buffer->beginningPreviousWordIndex();
            break;
        case JUMP_TO_END:
            targetX = m_buffer->endPreviousSymbolIndex();
            break;
        case 0:
            targetX = m_buffer->beginningPreviousSymbolIndex();
            break;
        default:
            exit_curses(0);
            std::cerr << "Unexpected cursor jump code: " << m_jumpCode << '\n';
            exit(1);
    }

    m_differenceX = targetX - m_x;

    if (m_differenceX == 0) { return false; }

    m_characters.reserve(abs(m_differenceX));

    if (m_differenceX >= 0)
    {
        for (int i = 0; i < m_differenceX; i++)
        {
            m_characters[i] = m_buffer->removeCharacter(false);
        }
    }
    else
    {
        m_buffer->moveCursor(m_y, targetX);

        for (int i = 0; i < - m_differenceX; i++)
        {
            m_characters[i] = m_buffer->removeCharacter(false);
        }
    }

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }

    return true;
}

void JumpCursorDeletePreviousWordInsertModeCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);

    if (m_differenceX >= 0)
    {
        for (int i = 0; i < m_differenceX; i++)
        {
            m_buffer->removeCharacter(false);
        }
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x + m_differenceX);

        for (int i = 0; i < - m_differenceX; i++)
        {
            m_buffer->removeCharacter(false);
        }
    }

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }
}
void JumpCursorDeletePreviousWordInsertModeCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);

    if (m_differenceX >= 0)
    {
        for (int i = 0; i < m_differenceX; i++)
        {
            m_buffer->insertCharacter(m_characters[i]);
        }
    }
    else
    {
        m_buffer->moveCursor(m_y, m_x + m_differenceX);

        for (int i = 0; i < - m_differenceX; i++)
        {
            m_buffer->insertCharacter(m_characters[i]);
        }
    }

    m_buffer->moveCursor(m_y, m_x);
    m_buffer->shiftCursorX(0);

    if (m_renderUndo) { m_view->displayCurrentLine(m_y); }
}
bool JumpCursorDeletePreviousWordInsertModeCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_x == 0) { return false; }

    int targetX = m_buffer->beginningPreviousWordIndex();

    if (targetX == m_x) { targetX = m_buffer->beginningPreviousSymbolIndex(); }

    if (targetX == m_x) { targetX = 0; } 

    m_differenceX = targetX - m_x;

    if (m_differenceX == 0) { return false; }

    m_characters.reserve(abs(m_differenceX));

    if (m_differenceX >= 0)
    {
        for (int i = 0; i < m_differenceX; i++)
        {
            m_characters[i] = m_buffer->removeCharacter(false);
        }
    }
    else
    {
        m_buffer->moveCursor(m_y, targetX);

        for (int i = 0; i < - m_differenceX; i++)
        {
            m_characters[i] = m_buffer->removeCharacter(false);
        }
    }

    if (m_renderExecute) { m_view->displayCurrentLine(m_y); }

    return true;
}
