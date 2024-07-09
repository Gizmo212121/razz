class Command
{

public:

    virtual void undo() = 0;
    virtual bool execute() = 0;

};

class Test1 : public Command
{
    void undo() override;
    bool execute() override;
};

class Test2 : public Command
{
    void undo() override;
    bool execute() override;
};

class Test3 : public Command
{
    void undo() override;
    bool execute() override;
};

class Test4 : public Command
{
    void undo() override;
    bool execute() override;
};
