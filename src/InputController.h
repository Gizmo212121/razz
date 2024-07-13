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

class InputController
{

private:

    Editor* m_editor;

    std::string m_commandBuffer;
    std::string m_repetitionBuffer;
    bool m_typingIntoRepetitionBuffer;

    void handleNormalModeInput();
    void handleCommandModeInput();
    void handleInsertModeInput();

    void clearRepetitionBuffer() { m_repetitionBuffer = "1"; m_typingIntoRepetitionBuffer = false; }

public:

    InputController(Editor* editor);

    void handleInput();
};
