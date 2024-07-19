#pragma once

#include <memory>

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

    Command(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : m_editor(editor), m_buffer(buffer), m_view(view), m_commandQueue(commandQueue) {}

    // virtual ~Command() = default;
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
    SetModeCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, MODE mode, int offset)
        : Command(editor, buffer, view, commandQueue), m_mode(mode), m_cursorOffset(offset) {}
};

class MoveCursorXCommand : public Command
{
private:
    int deltaX = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    MoveCursorXCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, int x)
        : Command(editor, buffer, view, commandQueue), deltaX(x) {}
};

class MoveCursorYCommand : public Command
{
private:
    int deltaY = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    MoveCursorYCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, int y)
        : Command(editor, buffer, view, commandQueue), deltaY(y) {}
};

class UndoCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    UndoCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
};

class RedoCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    RedoCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
};

class CursorFullRightCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    CursorFullRightCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
};

class CursorFullLeftCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    CursorFullLeftCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
};

class CursorFullTopCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    CursorFullTopCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
};

class CursorFullBottomCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    CursorFullBottomCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
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
    InsertCharacterCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, char character)
        : Command(editor, buffer, view, commandQueue), m_character(character) {}
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
    RemoveCharacterNormalCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool cursorLeft)
        : Command(editor, buffer, view, commandQueue), m_cursorLeft(cursorLeft) {}
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
    RemoveCharacterInsertCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
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
    ReplaceCharacterCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, char character)
        : Command(editor, buffer, view, commandQueue), m_character(character) {}
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
    InsertLineNormalCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool down)
        : Command(editor, buffer, view, commandQueue), m_down(down) {}
};

class InsertLineInsertCommand : public Command
{
private:
    int m_x = 0;
    int m_y = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    InsertLineInsertCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
};

class DeleteLineCommand : public Command
{
private:
    std::shared_ptr<LineGapBuffer> m_line;

    int m_x = 0;
    int m_y = 0;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    DeleteLineCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
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
    TabCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
};
