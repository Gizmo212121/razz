#pragma once

#include "Includes.h"

class Editor;
class Buffer;
class View;
class CommandQueue;
class LineGapBuffer;

class Command
{

protected:

    Editor* m_editor;
    Buffer* m_buffer;
    View* m_view;
    CommandQueue* m_commandQueue;

    // Removes characters in a line gap buffer from start to end inclusive
    void removeCharactersInRange(int start, int end, int cursorY) const;
    // Removes characters in a range from start to end inclusive, then inserts the removed characters into a vector.
    // Pre: This vector is assumed to be empty, as it is resized inside the function, end - start > 0
    void removeCharactersInRangeAndInsertIntoVector(std::vector<char>& vec, int start, int end, int cursorY) const;
    // Takes characters from a char vector and inserts them one by one into a line gap buffer from start to end inclusive
    void insertCharactersInRangeFromVector(std::vector<char>& vec, int start, int end, int cursorY) const;
    // Takes a jump code and returns the absolute x coordinate of the resulting jump
    int getXCoordinateFromJumpCode(int jumpCode) const;
    // Tabs a line left or right, returns the directional-displacement in characters
    int tabLine(const std::shared_ptr<LineGapBuffer>& line, bool headingRight, int cursorY, int rightwardOffset = WHITESPACE_PER_TAB);
    // Determins if two characters match as a pair, ex: '(' and ')' returns true. 'a' and 'b' returns false
    bool isMatchingPair(char leftPair, char rightPair) const;
    // Returns true if left pair is a valid pair starter. '(' returns treu, 'a' returns false
    bool isValidLeftPair(char character) const;
    // Returns the rightPair component of a given left pair
    char leftPairToRightPair(char leftPair) const;


public:

    bool m_renderExecute;
    bool m_renderUndo;

    Command(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : m_editor(editor), m_buffer(buffer), m_view(view), m_commandQueue(commandQueue), m_renderExecute(renderExecute), m_renderUndo(renderUndo) {}

    virtual void redo() = 0;
    virtual void undo() = 0;
    virtual bool execute() = 0;

};

class SetModeCommand : public Command
{
private:
    MODE m_mode;
    int m_cursorOffset;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    SetModeCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, MODE mode, int offset)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_mode(mode), m_cursorOffset(offset) {}
};

class MoveCursorXCommand : public Command
{
private:
    int deltaX = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    MoveCursorXCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, int x)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), deltaX(x) {}
};

class MoveCursorYCommand : public Command
{
private:
    int deltaY = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    MoveCursorYCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, int y)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), deltaY(y) {}
};

class UndoCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    UndoCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class RedoCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    RedoCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class CursorFullRightCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    CursorFullRightCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class CursorFullLeftCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    CursorFullLeftCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class CursorFullTopCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    CursorFullTopCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class CursorFullBottomCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    CursorFullBottomCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class InsertCharacterCommand : public Command
{
private:
    char m_character;
    int m_x = 0;
    int m_y = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    InsertCharacterCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, char character)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_character(character) {}
};

class RemoveCharacterNormalCommand : public Command
{
private:
    char m_character;
    int m_x = 0;
    int m_y = 0;

    bool m_cursorLeft;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    RemoveCharacterNormalCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, bool cursorLeft)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_cursorLeft(cursorLeft) {}
};

class RemoveCharacterInsertCommand : public Command
{
private:
    char m_character;
    int m_x = 0;
    int m_y = 0;

    std::shared_ptr<LineGapBuffer> m_line = nullptr;
    int m_deletedWhitespaces = 0;

    bool m_autoDeletedPair = false;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    RemoveCharacterInsertCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class ReplaceCharacterCommand : public Command
{
private:
    char m_character;
    char m_replacedCharacter = '\0';
    int m_x = 0;
    int m_y = 0;

    bool m_headingRight = false;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    ReplaceCharacterCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, char character, bool headingRight)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_character(character), m_headingRight(headingRight) {}
};

class InsertLineNormalCommand : public Command
{
private:
    int m_x = 0;
    int m_y = 0;

    bool m_down;

    int m_deletedSpaces = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    InsertLineNormalCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, bool down)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_down(down) {}
};

class InsertLineInsertCommand : public Command
{
private:
    int m_x = 0;
    int m_y = 0;

    int m_deletedSpaces = 0;

    std::vector<char> m_characters;
    size_t m_distanceToEndLine;
    int m_xPositionOfFirstCharacter;

    bool m_insidePair = false;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    InsertLineInsertCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class RemoveLineCommand : public Command
{
private:
    std::shared_ptr<LineGapBuffer> m_line;

    int m_x = 0;
    int m_y = 0;

    bool m_deletedOnlyLine = false;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    RemoveLineCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class RemoveLineToInsertCommand : public Command
{
private:
    std::vector<char> m_characters;

    int m_x = 0;
    int m_y = 0;

    int m_indexOfFirstNonSpaceCharacter = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    RemoveLineToInsertCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class TabCommand : public Command
{
private:
    int m_x = 0;
    int m_y = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    TabCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class FindCharacterCommand : public Command
{
private:
    char m_character;
    bool m_searchForward;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    FindCharacterCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, char character, bool forward)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_character(character), m_searchForward(forward) {}
};

class JumpCursorCommand : public Command
{
private:
    int m_jumpCode;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    JumpCursorCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, int code)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_jumpCode(code) {}
};

class JumpCursorDeleteWordCommand : public Command
{
private:
    int m_x = 0;
    int m_y = 0;

    std::vector<char> m_characters;
    int m_jumpCode;

    // int m_differenceX;
    int m_startX;
    int m_endX;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    JumpCursorDeleteWordCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, int code)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_jumpCode(code) {}
};

class JumpCursorDeletePreviousWordInsertModeCommand : public Command
{
private:
    int m_x = 0;
    int m_y = 0;

    std::vector<char> m_characters;

    int m_targetX;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    JumpCursorDeletePreviousWordInsertModeCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class RemoveLinesVisualLineModeCommand : public Command
{
private:
    int m_initialX = 0;
    int m_initialY = 0;
    int m_lowerBoundY = 0;
    int m_upperBoundY = 0;

    std::vector<std::shared_ptr<LineGapBuffer>> m_lines;

    void redo() override;
    void undo() override;
    bool execute() override;

public:
    RemoveLinesVisualLineModeCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class RemoveLinesVisualModeCommand : public Command
{
private:
    int m_cursorX = 0;
    int m_cursorY = 0;
    int m_previousVisualX = 0;
    int m_previousVisualY = 0;
    int m_lowerBoundY = 0;
    int m_upperBoundY = 0;
    int m_lowerBoundX = 0;
    int m_upperBoundX = 0;

    std::vector<char> m_lowerBoundCharacters;
    std::vector<std::shared_ptr<LineGapBuffer>> m_intermediaryLines;

    void redo() override;
    void undo() override;
    bool execute() override;

public:
    RemoveLinesVisualModeCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class RemoveLinesVisualBlockModeCommand : public Command
{
private:
    int m_cursorX = 0;
    int m_cursorY = 0;

    int m_previousVisualX = 0;
    int m_previousVisualY = 0;

    int m_lowerBoundY = 0;
    int m_upperBoundY = 0;

    int m_lowerBoundX = 0;
    int m_upperBoundX = 0;

    std::vector<std::vector<char>> m_lines;

    void redo() override;
    void undo() override;
    bool execute() override;

public:
    RemoveLinesVisualBlockModeCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};

class TabLineCommand : public Command
{
private:
    int m_initialX = 0;
    int m_initialY = 0;

    bool m_headingRight;

    int m_differenceInCharacters = 0;

    void redo() override;
    void undo() override;
    bool execute() override;

public:
    TabLineCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, bool headingRight)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_headingRight(headingRight) {}
};

class TabLineVisualCommand : public Command
{
private:
    int m_initialX = 0;
    int m_initialY = 0;

    int m_lowerBoundY = 0;
    int m_upperBoundY = 0;

    bool m_headingRight;

    std::vector<int> m_differenceInCharacters;

    void redo() override;
    void undo() override;
    bool execute() override;

public:
    TabLineVisualCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, bool headingRight)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_headingRight(headingRight) {}
};

class AutocompletePair : public Command
{
private:
    int m_y = 0;
    int m_x = 0;

    char m_leftPair;
    char m_rightPair;

    bool m_autocomplete = true;

    void redo() override;
    void undo() override;
    bool execute() override;

public:
    AutocompletePair(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, char leftPair)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_leftPair(leftPair) {}
};

class PasteCommand : public Command
{
private:
    int m_cursorX = 0;
    int m_cursorY = 0;

    int m_previousVisualX = 0;
    int m_previousVisualY = 0;

    int m_lowerBoundY = 0;
    int m_upperBoundY = 0;

    int m_lowerBoundX = 0;
    int m_upperBoundX = 0;

    std::vector<std::vector<char>> m_lines;

    void redo() override;
    void undo() override;
    bool execute() override;

public:
    PasteCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};
