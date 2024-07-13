#pragma once

class Editor;
class Buffer;
class View;
class CommandQueue;

enum MODE
{
    INSERT_MODE,
    NORMAL_MODE,
    COMMAND_MODE
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

class QuitCommand : public Command
{
private:
    void redo() override;
    void undo() override;
    bool execute() override;
public:
    QuitCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue)
        : Command(editor, buffer, view, commandQueue) {}
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

class RemoveCharacterCommand : public Command
{
private:
    char m_character;
    int m_x = 0;
    int m_y = 0;

    bool m_cursorLeft;
    int m_cursorDifferential;

    void redo() override;
    void undo() override;
    bool execute() override;
public:
    RemoveCharacterCommand(Editor* editor, Buffer* buffer, View* view, CommandQueue* commandQueue, bool cursorLeft, int cursorDiff)
        : Command(editor, buffer, view, commandQueue), m_cursorLeft(cursorLeft), m_cursorDifferential(cursorDiff) {}
};
