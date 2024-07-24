#pragma once

#include <memory>
#include <vector>

class Editor;
class Buffer;
class View;
class CommandQueue;
class LineGapBuffer;

enum MODE
{
    INSERT_MODE,
    NORMAL_MODE,
    COMMAND_MODE,
    REPLACE_CHAR_MODE,
};


class Command
{

protected:

    Editor* m_editor;
    Buffer* m_buffer;
    View* m_view;
    CommandQueue* m_commandQueue;

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

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    ReplaceCharacterCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo, char character)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo), m_character(character) {}
};

class InsertLineNormalCommand : public Command
{
private:
    int m_x = 0;
    int m_y = 0;

    bool m_down;

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

    std::vector<char> m_characters;
    size_t m_distanceToEndLine;
    int m_xPositionOfFirstCharacter;

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

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    RemoveLineCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
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

    int m_differenceX;

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

    int m_differenceX;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    JumpCursorDeletePreviousWordInsertModeCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool renderExecute, bool renderUndo)
        : Command(editor, buffer, view, commandQueue, renderExecute, renderUndo) {}
};
