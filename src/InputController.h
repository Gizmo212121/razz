#pragma once

#include <string>
#include <vector>

class Editor;

enum KEYS
{
    CTRL_C = 3,
    ENTER = 10,
    CTRL_R = 18,
    ESCAPE = 27,
    QUOTE = 34,
    APOSTROPHE = 39,
    COLON = 58,
    A = 65,
    H = 72,
    I = 73,
    P = 80,
    X = 88,
    a = 97,
    h = 104,
    i = 105,
    j = 106,
    p = 112,
    r = 114,
    u = 117,
    x = 120,
    BACKSPACE = 263,
};

const int MAX_REPETITION_COUNT = 1000;

class InputController
{

private:

    Editor* m_editor;

    std::string m_commandBuffer;
    std::string m_repetitionBuffer;

    int m_previousInput = 0;

    void handleNormalModeInput(int input);
    void handleCommandModeInput(int input);
    void handleInsertModeInput(int input);
    void handleReplaceCharMode(int input);

    void clearRepetitionBuffer() { m_repetitionBuffer.clear(); }
    int repetitionCount();

    bool repeatedInput(int input) { return (input == m_previousInput) ;}

public:

    InputController(Editor* editor);

    void handleInput();
};
