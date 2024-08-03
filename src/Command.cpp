#include "Command.h"
#include "Buffer.h"
#include "Editor.h"
#include "Includes.h"
#include "InputController.h"
#include "LineGapBuffer.h"

int Command::tabLine(const std::shared_ptr<LineGapBuffer>& line, bool headingRight, int cursorY, int rightwardOffset)
{
    if (headingRight)
    {
        m_buffer->moveCursor(cursorY, 0);

        for (int i = 0; i < abs(rightwardOffset); i++)
        {
            m_buffer->insertCharacter(' ');
        }

        return WHITESPACE_PER_TAB;
    }
    else
    {
        int indexOfFirstNSC = m_buffer->indexOfFirstNonSpaceCharacter(line);

        if (indexOfFirstNSC == 0) { return false; }

        int charactersToDelete = std::min(WHITESPACE_PER_TAB, indexOfFirstNSC);

        m_buffer->moveCursor(cursorY, 0);

        for (int i = 0; i < charactersToDelete; i++)
        {
            m_buffer->removeCharacter(false);
        }

        return charactersToDelete;
    }
}

void Command::removeCharactersInRange(int start, int end, int cursorY) const
{
    assert(cursorY >= 0);
    assert(end - start >= 0);

    m_buffer->moveCursor(cursorY, start);

    for (int i = 0; i < end - start; i++)
    {
        m_buffer->removeCharacter(false);
    }
}

void Command::removeCharactersInRangeAndInsertIntoVector(std::vector<char>& vec, int start, int end, int cursorY) const
{
    assert(cursorY >= 0);
    assert(end - start >= 0);

    vec.reserve(end - start);

    m_buffer->moveCursor(cursorY, start);

    for (int i = 0; i < end - start; i++)
    {
        vec.push_back(m_buffer->removeCharacter(false));
    }
}

void Command::insertCharactersInRangeFromVector(std::vector<char>& vec, int start, int end, int cursorY) const
{
    assert(cursorY >= 0);
    assert(end - start >= 0);

    m_buffer->moveCursor(cursorY, start);

    for (int i = 0; i < end - start; i++)
    {
        m_buffer->insertCharacter(vec[i]);
    }
}

int Command::getXCoordinateFromJumpCode(int jumpCode) const
{
    switch (jumpCode)
    {
        case JUMP_FORWARD | JUMP_BY_WORD | JUMP_TO_END:
            return m_buffer->endNextWordIndex();
            break;
        case JUMP_FORWARD | JUMP_BY_WORD:
            return m_buffer->beginningNextWordIndex();
            break;
        case JUMP_FORWARD | JUMP_TO_END:
            return m_buffer->endNextSymbolIndex();
            break;
        case JUMP_FORWARD:
            return m_buffer->beginningNextSymbolIndex();
            break;
        case JUMP_BY_WORD | JUMP_TO_END:
            return m_buffer->endPreviousWordIndex();
            break;
        case JUMP_BY_WORD:
            return m_buffer->beginningPreviousWordIndex();
            break;
        case JUMP_TO_END:
            return m_buffer->endPreviousSymbolIndex();
            break;
        case 0:
            return m_buffer->beginningPreviousSymbolIndex();
            break;
        default:
            exit_curses(0);
            std::cerr << "Unexpected cursor jump code: " << jumpCode << '\n';
            exit(1);
    }
}

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

    m_view->display();

    return false;
}

void MoveCursorXCommand::redo() {}
void MoveCursorXCommand::undo() {}
bool MoveCursorXCommand::execute()
{
    int prevCursorX = m_buffer->getCursorPos().second;
    m_buffer->shiftCursorX(deltaX);
    int postCursorX = m_buffer->getCursorPos().second;

    if (prevCursorX != postCursorX) { m_view->display(); }

    return false;
}

void MoveCursorYCommand::redo() {}
void MoveCursorYCommand::undo() {}
bool MoveCursorYCommand::execute()
{
    const std::pair<int, int> prevCursorPos = m_buffer->getCursorPos();
    const std::shared_ptr<LineGapBuffer>& lineGapBuffer = m_buffer->getLineGapBuffer(prevCursorPos.first);
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
            m_buffer->moveCursor(prevCursorPos.first, 0);
            for (int i = 0; i < lineSize; i++)
            {
                m_buffer->removeCharacter(false);
            }
        }
    }

    m_buffer->shiftCursorY(deltaY);

    int postCursorY = m_buffer->getCursorPos().first;

    if (postCursorY != prevCursorPos.first) { m_view->display(); }

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
    int prevCursorX = m_buffer->getCursorPos().second;
    m_buffer->shiftCursorFullRight();
    int postCursorX = m_buffer->getCursorPos().second;

    if (prevCursorX != postCursorX) { m_view->display(); }

    return false;
}

void CursorFullLeftCommand::redo() {}
void CursorFullLeftCommand::undo() { }
bool CursorFullLeftCommand::execute()
{
    int prevCursorX = m_buffer->getCursorPos().second;
    m_buffer->shiftCursorFullLeft();
    int postCursorX = m_buffer->getCursorPos().second;

    if (prevCursorX != postCursorX) { m_view->display(); }

    return false;
}

void CursorFullTopCommand::redo() {}
void CursorFullTopCommand::undo() { }
bool CursorFullTopCommand::execute()
{
    int prevCursorY = m_buffer->getCursorPos().first;
    m_buffer->shiftCursorFullTop();
    int postCursorY = m_buffer->getCursorPos().first;

    if (m_renderExecute && prevCursorY != postCursorY) { m_view->display(); }

    return false;
}

void CursorFullBottomCommand::redo() {}
void CursorFullBottomCommand::undo() { }
bool CursorFullBottomCommand::execute()
{
    int prevCursorY = m_buffer->getCursorPos().first;
    m_buffer->shiftCursorFullBottom();
    int postCursorY = m_buffer->getCursorPos().first;

    if (m_renderExecute && prevCursorY != postCursorY) { m_view->display(); }

    return false;
}

void InsertCharacterCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x - 1);
    m_buffer->insertCharacter(m_character);
    m_buffer->shiftCursorX(-1);

    if (m_renderExecute) { m_view->display(); }
}
void InsertCharacterCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->removeCharacter();
    m_buffer->shiftCursorX(-1);

    if (m_renderUndo) { m_view->display(); }
}
bool InsertCharacterCommand::execute()
{
    m_buffer->insertCharacter(m_character);

    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_renderExecute) { m_view->display(); }
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

    if (m_renderExecute) { m_view->display(); }
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

    if (m_renderUndo) { m_view->display(); }
}
bool RemoveCharacterNormalCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_buffer->getLineGapBuffer(m_y)->lineSize() <= 0) { return false; }
    if (m_buffer->getLineGapBuffer(m_y)->bufferSize() <= 0) { return false; }
    if (m_cursorLeft && m_x == 0) { return false; }

    m_character = m_buffer->removeCharacter(m_cursorLeft);

    if (m_renderExecute) { m_view->display(); }

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

        if (m_renderExecute) { m_view->display(); }
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

        if (m_renderExecute) { m_view->display(); }
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

        if (m_renderUndo) { m_view->display(); }
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

        if (m_renderExecute) { m_view->display(); }
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

        if (m_renderExecute) { m_view->display(); }
    }

    return true;
}

void ReplaceCharacterCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->replaceCharacter(m_character);
    if (m_renderExecute) { m_view->display(); }
}
void ReplaceCharacterCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    m_buffer->replaceCharacter(m_replacedCharacter);
    if (m_renderUndo) { m_view->display(); }
}
bool ReplaceCharacterCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    int lineSize = static_cast<int>(m_editor->buffer().getLineGapBuffer(cursorPos.first)->lineSize());

    m_replacedCharacter = m_buffer->replaceCharacter(m_character);

    if (m_headingRight && cursorPos.second < lineSize - 1)
    {
        m_buffer->shiftCursorX(1);
    }

    if (m_character == m_replacedCharacter) { return false; }

    if (m_renderExecute) { m_view->display(); }

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


    removeCharactersInRangeAndInsertIntoVector(m_characters, m_x, lineSize, m_y);

    m_buffer->insertLine(true);

    for (int i = 0; i < m_xPositionOfFirstCharacter; i++)
    {
        m_buffer->insertCharacter(' ');
    }

    insertCharactersInRangeFromVector(m_characters, m_x, lineSize, m_y + 1);

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

    m_buffer->moveCursor(m_y + 1, m_x);

    if (m_deletedOnlyLine == true) { m_buffer->removeLine(); }

    m_buffer->moveCursor(m_y, m_x);

    if (m_renderUndo) { m_view->display(); }
}
bool RemoveLineCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_buffer->getFileGapBuffer().numberOfLines() == 1 && m_buffer->getLineGapBuffer(m_y)->lineSize() == 0) { m_view->display(); return false; }
    else if (m_buffer->getFileGapBuffer().numberOfLines() == 1) { m_deletedOnlyLine = true; }

    m_line = m_buffer->removeLine();
    m_buffer->shiftCursorX(0);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void RemoveLineToInsertCommand::redo()
{
    const std::shared_ptr<LineGapBuffer>& line = m_buffer->getLineGapBuffer(m_y);

    removeCharactersInRange(m_indexOfFirstNonSpaceCharacter, static_cast<int>(line->lineSize()), m_y);

    m_buffer->moveCursor(m_y, m_indexOfFirstNonSpaceCharacter);
    m_buffer->shiftCursorX(0);

    if (m_renderExecute) { m_view->display(); }
}
void RemoveLineToInsertCommand::undo()
{
    m_buffer->moveCursor(m_y, 0);

    // Add back all spaces in beginning, if no spaces were removed, these get overwritten in the next function
    for (int i = 0; i < m_indexOfFirstNonSpaceCharacter; i++)
    {
        m_buffer->insertCharacter(' ');
    }

    insertCharactersInRangeFromVector(m_characters, m_indexOfFirstNonSpaceCharacter, m_indexOfFirstNonSpaceCharacter + m_characters.size(), m_y);

    m_buffer->moveCursor(m_y, m_indexOfFirstNonSpaceCharacter);

    if (m_renderUndo) { m_view->display(); }
}
bool RemoveLineToInsertCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    const std::shared_ptr<LineGapBuffer>& line = m_buffer->getLineGapBuffer(m_y);

    int lineSize = static_cast<int>(line->lineSize());

    if (lineSize == 0) { return false; }

    m_indexOfFirstNonSpaceCharacter = m_buffer->indexOfFirstNonSpaceCharacter(line);

    removeCharactersInRangeAndInsertIntoVector(m_characters, m_indexOfFirstNonSpaceCharacter, static_cast<int>(line->lineSize()), m_y);

    m_editor->setMode(INSERT_MODE);
    m_buffer->moveCursor(m_y, m_indexOfFirstNonSpaceCharacter);
    m_view->insertCursor();

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void TabCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);
    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->insertCharacter(' '); }
    if (m_renderExecute) { m_view->display(); }
}
void TabCommand::undo()
{
    m_buffer->moveCursor(m_y, m_x);
    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->removeCharacter(false); }
    if (m_renderUndo) { m_view->display(); }
}
bool TabCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    for (int i = 0; i < WHITESPACE_PER_TAB; i++) { m_buffer->insertCharacter(' '); }

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void FindCharacterCommand::redo() {}
void FindCharacterCommand::undo() {}
bool FindCharacterCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

    int xPos = m_buffer->findCharacterIndex(m_character, m_searchForward);

    if (xPos == cursorPos.second || xPos < 0) { return false; }

    m_buffer->moveCursor(cursorPos.first, xPos);

    if (m_renderExecute) { m_view->display(); }

    return false;
}

void JumpCursorCommand::redo() {}
void JumpCursorCommand::undo() {}
bool JumpCursorCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();

    int targetX = getXCoordinateFromJumpCode(m_jumpCode);

    if (targetX != cursorPos.second)
    {
        m_buffer->moveCursor(cursorPos.first, targetX);

        m_view->display();
    }

    return false;
}

void JumpCursorDeleteWordCommand::redo()
{
    removeCharactersInRange(m_startX, m_endX, m_y);

    if (m_renderExecute) { m_view->display(); }
}
void JumpCursorDeleteWordCommand::undo()
{
    insertCharactersInRangeFromVector(m_characters, m_startX, m_endX, m_y);

    m_buffer->moveCursor(m_y, m_x);
    m_buffer->shiftCursorX(0);

    if (m_renderUndo) { m_view->display(); }
}
bool JumpCursorDeleteWordCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    int targetX = getXCoordinateFromJumpCode(m_jumpCode);

    int differenceX = targetX - m_x;

    m_startX = std::min(targetX, m_x);
    m_endX = std::max(targetX, m_x);

    if (differenceX == 0) { return false; }

    removeCharactersInRangeAndInsertIntoVector(m_characters, m_startX, m_endX, m_y);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void JumpCursorDeletePreviousWordInsertModeCommand::redo()
{
    m_buffer->moveCursor(m_y, m_x);

    removeCharactersInRange(m_targetX, m_x, m_y);

    if (m_renderExecute) { m_view->display(); }
}
void JumpCursorDeletePreviousWordInsertModeCommand::undo()
{
    insertCharactersInRangeFromVector(m_characters, m_targetX, m_x, m_y);

    m_buffer->moveCursor(m_y, m_x);
    m_buffer->shiftCursorX(0);

    if (m_renderUndo) { m_view->display(); }
}
bool JumpCursorDeletePreviousWordInsertModeCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    if (m_x == 0) { return false; }

    m_targetX = m_buffer->beginningPreviousWordIndex();
    if (m_targetX == m_x) { m_targetX = m_buffer->beginningPreviousSymbolIndex(); }
    if (m_targetX == m_x) { m_targetX = 0; } 

    int differenceX = m_targetX - m_x;

    if (differenceX == 0) { return false; }

    removeCharactersInRangeAndInsertIntoVector(m_characters, m_targetX, m_x, cursorPos.first);

    m_buffer->moveCursor(m_y, m_targetX);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void RemoveLinesVisualLineModeCommand::redo()
{
    m_buffer->moveCursor(m_lowerBoundY, 0);

    for (int i = 0; i <= m_upperBoundY - m_lowerBoundY; i++)
    {
        m_buffer->removeLine();
    }

    if (m_initialY >= m_upperBoundY)
    {
        m_buffer->moveCursor(m_initialY + m_lowerBoundY - m_upperBoundY, m_initialX);
    }
    else
    {
        m_buffer->moveCursor(m_initialY, m_initialX);
    }

    if (m_renderExecute) { m_view->display(); }
}
void RemoveLinesVisualLineModeCommand::undo()
{
    int targetY = std::max(0, m_lowerBoundY - 1);
    m_buffer->moveCursor(targetY, 0);

    for (int i = 0; i <= m_upperBoundY - m_lowerBoundY; i++)
    {
        if (m_lowerBoundY == 0 && i == 0)
        {

            m_buffer->insertLine(m_lines[i], false);
        }
        else
        {
            m_buffer->insertLine(m_lines[i], true);
        }
    }

    m_buffer->moveCursor(m_initialY, m_initialX);

    if (m_renderUndo) { m_view->display(); }
}
bool RemoveLinesVisualLineModeCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_initialX = cursorPos.second;
    m_initialY = cursorPos.first;
    const std::pair<int, int>& previousVisualPos = m_editor->inputController().initialVisualModeCursor();

    m_lowerBoundY = std::min(cursorPos.first, previousVisualPos.first);
    m_upperBoundY = std::max(cursorPos.first, previousVisualPos.first);

    m_buffer->moveCursor(m_lowerBoundY, 0);

    m_lines.reserve(m_upperBoundY - m_lowerBoundY);

    for (int i = 0; i <= m_upperBoundY - m_lowerBoundY; i++)
    {
        m_lines.push_back(m_buffer->removeLine());
    }

    m_buffer->moveCursor(m_lowerBoundY, m_initialX);

    m_editor->setMode(NORMAL_MODE);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void RemoveLinesVisualModeCommand::redo()
{
    m_buffer->moveCursor(m_lowerBoundY, 0);

    for (int i = m_lowerBoundY; i <= m_upperBoundY; i++)
    {
        if (i == m_lowerBoundY)
        {
            if (m_lowerBoundY == m_upperBoundY)
            {
                removeCharactersInRange(m_lowerBoundX, m_upperBoundX + 1, m_lowerBoundY);
            }
            else if (m_lowerBoundY < m_cursorY)
            {
                removeCharactersInRange(m_previousVisualX, m_buffer->getLineGapBuffer(m_lowerBoundY)->lineSize(), m_lowerBoundY);
            }
            else
            {
                removeCharactersInRange(m_cursorX, m_buffer->getLineGapBuffer(m_lowerBoundY)->lineSize(), m_lowerBoundY);
            }
        }
        else
        {
            m_buffer->moveCursor(m_lowerBoundY + 1, 0);
            m_intermediaryLines.push_back(m_buffer->removeLine());
        }
    }

    // Append everything after the cursor on the last line to the first line
    if (m_upperBoundY - m_lowerBoundY > 0)
    {
        const std::shared_ptr<LineGapBuffer>& lastLine = m_intermediaryLines[m_intermediaryLines.size() - 1];

        if (m_lowerBoundY == m_cursorY)
        {
            m_buffer->moveCursor(m_lowerBoundY, m_cursorX);

            for (size_t i = m_previousVisualX + 1; i < lastLine->lineSize(); i++)
            {
                m_buffer->insertCharacter(lastLine->at(i));
            }
        }
        else
        {
            m_buffer->moveCursor(m_lowerBoundY, m_previousVisualX);

            for (size_t i = m_cursorX + 1; i < lastLine->lineSize(); i++)
            {
                m_buffer->insertCharacter(lastLine->at(i));
            }
        }
    }

    m_buffer->moveCursor(m_lowerBoundY, m_upperBoundX);
    m_buffer->shiftCursorX(0);

    if (m_renderExecute) { m_view->display(); }
}
void RemoveLinesVisualModeCommand::undo()
{
    m_buffer->moveCursor(m_lowerBoundY, 0);

    // Append everything after the cursor on the last line to the first line
    if (m_upperBoundY - m_lowerBoundY > 0)
    {
        const std::shared_ptr<LineGapBuffer>& firstLine = m_buffer->getLineGapBuffer(m_lowerBoundY);

        if (m_lowerBoundY == m_cursorY)
        {
            removeCharactersInRange(m_cursorX, static_cast<int>(firstLine->lineSize()), m_lowerBoundY);

        }
        else
        {
            removeCharactersInRange(m_previousVisualX, static_cast<int>(firstLine->lineSize()), m_lowerBoundY);
        }
    }

    for (int i = m_upperBoundY; i >= m_lowerBoundY; i--)
    {
        if (i == m_lowerBoundY)
        {
            if (m_lowerBoundY == m_upperBoundY)
            {
                insertCharactersInRangeFromVector(m_lowerBoundCharacters, m_lowerBoundX, m_upperBoundX + 1, m_lowerBoundY);
            }
            else if (m_lowerBoundY < m_cursorY)
            {
                insertCharactersInRangeFromVector(m_lowerBoundCharacters, m_previousVisualX, m_previousVisualX + static_cast<int>(m_lowerBoundCharacters.size()), m_lowerBoundY);
            }
            else
            {
                insertCharactersInRangeFromVector(m_lowerBoundCharacters, m_cursorX, m_cursorX + static_cast<int>(m_lowerBoundCharacters.size()), m_lowerBoundY);
            }
        }
        else
        {
            m_buffer->moveCursor(m_lowerBoundY, 0);
            m_buffer->insertLine(m_intermediaryLines[i - m_lowerBoundY - 1], true);
        }
    }


    m_buffer->moveCursor(m_lowerBoundY, m_upperBoundX);
    m_buffer->shiftCursorX(0);

    if (m_renderUndo) { m_view->display(); }
}
bool RemoveLinesVisualModeCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_cursorX = cursorPos.second;
    m_cursorY = cursorPos.first;
    const std::pair<int, int>& previousVisualPos = m_editor->inputController().initialVisualModeCursor();
    m_previousVisualX = previousVisualPos.second;
    m_previousVisualY = previousVisualPos.first;

    m_lowerBoundY = std::min(m_cursorY, m_previousVisualY);
    m_upperBoundY = std::max(m_cursorY, m_previousVisualY);

    m_lowerBoundX = std::min(m_cursorX, m_previousVisualX);
    m_upperBoundX = std::max(m_cursorX, m_previousVisualX);

    int differenceY = m_upperBoundY - m_lowerBoundY;

    m_intermediaryLines.reserve(differenceY);

    m_buffer->moveCursor(m_lowerBoundY, 0);

    for (int i = m_lowerBoundY; i <= m_upperBoundY; i++)
    {
        if (i == m_lowerBoundY)
        {
            if (m_lowerBoundY == m_upperBoundY)
            {
                removeCharactersInRangeAndInsertIntoVector(m_lowerBoundCharacters, m_lowerBoundX, m_upperBoundX + 1, m_lowerBoundY);
            }
            else if (m_lowerBoundY < m_cursorY)
            {
                removeCharactersInRangeAndInsertIntoVector(m_lowerBoundCharacters, m_previousVisualX, m_buffer->getLineGapBuffer(m_lowerBoundY)->lineSize(), m_lowerBoundY);
            }
            else
            {
                removeCharactersInRangeAndInsertIntoVector(m_lowerBoundCharacters, m_cursorX, m_buffer->getLineGapBuffer(m_lowerBoundY)->lineSize(), m_lowerBoundY);
            }
        }
        else
        {
            m_buffer->moveCursor(m_lowerBoundY + 1, 0);
            m_intermediaryLines.push_back(m_buffer->removeLine());
        }
    }

    // Append everything after the cursor on the last line to the first line
    if (differenceY > 0)
    {
        const std::shared_ptr<LineGapBuffer>& lastLine = m_intermediaryLines[m_intermediaryLines.size() - 1];

        if (m_lowerBoundY == m_cursorY)
        {
            m_buffer->moveCursor(m_lowerBoundY, m_cursorX);

            for (size_t i = m_previousVisualX + 1; i < lastLine->lineSize(); i++)
            {
                m_buffer->insertCharacter(lastLine->at(i));
            }
        }
        else
        {
            m_buffer->moveCursor(m_lowerBoundY, m_previousVisualX);

            for (size_t i = m_cursorX + 1; i < lastLine->lineSize(); i++)
            {
                m_buffer->insertCharacter(lastLine->at(i));
            }
        }
    }

    m_buffer->moveCursor(m_lowerBoundY, m_upperBoundX);
    m_buffer->shiftCursorX(0);

    m_editor->setMode(NORMAL_MODE);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void RemoveLinesVisualBlockModeCommand::redo()
{
    m_buffer->moveCursor(m_lowerBoundY, 0);

    for (int row = m_lowerBoundY; row <= m_upperBoundY; row++)
    {
        const std::shared_ptr<LineGapBuffer>& line = m_buffer->getLineGapBuffer(row);
        int lineSize = static_cast<int>(line->lineSize());

        if (lineSize == 0) { continue; }

        int endIndex = std::min(m_upperBoundX + 1, lineSize);

        if (m_lowerBoundX > endIndex) { continue; }

        removeCharactersInRange(m_lowerBoundX, endIndex, row);
    }

    m_buffer->moveCursor(m_lowerBoundY, m_lowerBoundX);
    m_buffer->shiftCursorX(0);

    if (m_renderExecute) { m_view->display(); }
}
void RemoveLinesVisualBlockModeCommand::undo()
{
    m_buffer->moveCursor(m_lowerBoundY, 0);

    for (int row = m_lowerBoundY; row <= m_upperBoundY; row++)
    {
        int numberOfCharactersToAdd = static_cast<int>(m_lines[row - m_lowerBoundY].size());

        if (numberOfCharactersToAdd == 0) { continue; }

        insertCharactersInRangeFromVector(m_lines[row - m_lowerBoundY], m_lowerBoundX, m_lowerBoundX + numberOfCharactersToAdd, row);
    }

    m_buffer->moveCursor(m_lowerBoundY, m_lowerBoundX);
    m_buffer->shiftCursorX(0);

    if (m_renderUndo) { m_view->display(); }
}
bool RemoveLinesVisualBlockModeCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_cursorX = cursorPos.second;
    m_cursorY = cursorPos.first;
    const std::pair<int, int>& previousVisualPos = m_editor->inputController().initialVisualModeCursor();
    m_previousVisualX = previousVisualPos.second;
    m_previousVisualY = previousVisualPos.first;

    m_lowerBoundY = std::min(m_cursorY, m_previousVisualY);
    m_upperBoundY = std::max(m_cursorY, m_previousVisualY);

    m_lowerBoundX = std::min(m_cursorX, m_previousVisualX);
    m_upperBoundX = std::max(m_cursorX, m_previousVisualX);

    int differenceY = m_upperBoundY - m_lowerBoundY;

    m_lines.resize(differenceY + 1);

    m_buffer->moveCursor(m_lowerBoundY, 0);

    for (int row = m_lowerBoundY; row <= m_upperBoundY; row++)
    {
        const std::shared_ptr<LineGapBuffer>& line = m_buffer->getLineGapBuffer(row);
        int lineSize = static_cast<int>(line->lineSize());

        if (lineSize == 0) { continue; }

        int endIndex = std::min(m_upperBoundX + 1, lineSize);

        if (m_lowerBoundX > endIndex) { continue; }

        removeCharactersInRangeAndInsertIntoVector(m_lines[row - m_lowerBoundY], m_lowerBoundX, endIndex, row);
    }

    m_buffer->moveCursor(m_lowerBoundY, m_lowerBoundX);
    m_buffer->shiftCursorX(0);

    m_editor->setMode(NORMAL_MODE);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void TabLineCommand::redo()
{
    m_buffer->moveCursor(m_initialY, m_initialX);

    tabLine(m_buffer->getLineGapBuffer(m_initialY), m_headingRight, m_initialY);

    m_buffer->moveCursor(m_initialY, m_initialX);
    m_buffer->shiftCursorX(0);

    if (m_renderExecute) { m_view->display(); }
}
void TabLineCommand::undo()
{
    m_buffer->moveCursor(m_initialY, m_initialX);

    tabLine(m_buffer->getLineGapBuffer(m_initialY), !m_headingRight, m_initialY, m_differenceInCharacters);

    m_buffer->moveCursor(m_initialY, m_initialX);

    if (m_renderUndo) { m_view->display(); }
}
bool TabLineCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_initialX = cursorPos.second;
    m_initialY = cursorPos.first;

    m_differenceInCharacters = tabLine(m_buffer->getLineGapBuffer(m_initialY), m_headingRight, m_initialY);

    if (m_differenceInCharacters == 0) { return false; }

    m_buffer->moveCursor(m_initialY, m_initialX);
    m_buffer->shiftCursorX(0);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void TabLineVisualCommand::redo()
{
    m_buffer->moveCursor(m_initialY, m_initialX);

    for (int i = 0; i <= m_upperBoundY - m_lowerBoundY; i++)
    {
        tabLine(m_buffer->getLineGapBuffer(m_lowerBoundY + i), m_headingRight, m_lowerBoundY + i);
    }

    m_buffer->moveCursor(m_initialY, m_initialX);

    if (m_renderExecute) { m_view->display(); }
}
void TabLineVisualCommand::undo()
{
    m_buffer->moveCursor(m_initialY, m_initialX);

    for (int i = 0; i <= m_upperBoundY - m_lowerBoundY; i++)
    {
        tabLine(m_buffer->getLineGapBuffer(m_lowerBoundY + i), !m_headingRight, m_lowerBoundY + i, m_differenceInCharacters[i]);
    }

    m_buffer->moveCursor(m_initialY, m_initialX);

    if (m_renderUndo) { m_view->display(); }
}
bool TabLineVisualCommand::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_initialX = cursorPos.second;
    m_initialY = cursorPos.first;
    const std::pair<int, int>& previousVisualPos = m_editor->inputController().initialVisualModeCursor();

    m_lowerBoundY = std::min(cursorPos.first, previousVisualPos.first);
    m_upperBoundY = std::max(cursorPos.first, previousVisualPos.first);

    m_differenceInCharacters.reserve(m_upperBoundY - m_lowerBoundY);

    bool madeChange = false;

    for (int i = 0; i <= m_upperBoundY - m_lowerBoundY; i++)
    {
        m_differenceInCharacters.push_back(tabLine(m_buffer->getLineGapBuffer(m_lowerBoundY + i), m_headingRight, m_lowerBoundY + i));

        if (m_differenceInCharacters[i] != 0) { madeChange = true; }
    }

    if (!madeChange) { return false; }

    m_buffer->moveCursor(m_initialY, m_initialX);
    m_buffer->shiftCursorX(0);

    if (m_renderExecute) { m_view->display(); }

    return true;
}

void AutocompletePair::redo()
{

    if (m_renderExecute) { m_view->display(); }
}
void AutocompletePair::undo()
{
    if (m_renderUndo) { m_view->display(); }
}
bool AutocompletePair::execute()
{
    const std::pair<int, int>& cursorPos = m_buffer->getCursorPos();
    m_x = cursorPos.second;
    m_y = cursorPos.first;

    bool autocomplete = true;

    switch (m_leftPair)
    {
        case '(':
            m_rightPair = ')';
            break;
        case '{':
            m_rightPair = '}';
            break;
        case '\'':
            m_rightPair = '\'';
            if (m_x != 0 && m_buffer->getLineGapBuffer(m_y)->at(m_x - 1) != ' ') { autocomplete = false; }
            break;
        case '[':
            m_rightPair = ']';
            break;
        case '"':
            m_rightPair = '"';
            break;
        default:
            endwin();
            std::cerr << "Unexpected input: " << m_leftPair << '\n';
            exit(1);
    }

    m_buffer->insertCharacter(m_leftPair);

    if (autocomplete)
    {
        m_buffer->insertCharacter(m_rightPair);
        m_buffer->shiftCursorX(-1);
    }

    if (m_renderExecute) { m_view->display(); }

    return true;
}
