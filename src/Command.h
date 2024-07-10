#pragma once

class Editor;

enum MODE
{
    INSERT_MODE,
    NORMAL_MODE,
    COMMAND_MODE
};


class Command
{

public:

    virtual void undo() = 0;
    virtual bool execute() = 0;

};

class QuitCommand : public Command
{

private:

    Editor* m_editor;

    void undo() override;
    bool execute() override;

public:

    QuitCommand(Editor* editor);

};

class SetModeCommand : public Command
{

private:

    Editor* m_editor;
    MODE m_mode;

    void undo() override;
    bool execute() override;

public:

    SetModeCommand(Editor* editor, MODE mode);
};
