#pragma once

#include <string>
#include <vector>

class Editor;

enum KEYS
{
    CTRL_C = 3,
    TAB = 9,
    ENTER = 10,
    CTRL_R = 18,
    ESCAPE = 27,
    SPACE = 32,
    QUOTE = 34,
    APOSTROPHE = 39,
    COLON = 58,
    SEMICOLON = 59,
    A = 65,
    H = 72,
    I = 73,
    O = 79,
    P = 80,
    W = 87,
    X = 88,
    a = 97,
    b = 98,
    d = 100,
    f = 102,
    h = 104,
    i = 105,
    j = 106,
    o = 111,
    p = 112,
    r = 114,
    u = 117,
    w = 119,
    x = 120,
    SPACE_DOT = 250,
    BACKSPACE = 263,
};

const int MAX_REPETITION_COUNT = 1000;

#include "Command.h"

class InputController
{

private:

    Editor* m_editor;

    std::string m_commandBuffer;
    std::string m_repetitionBuffer;

    char m_findCharacter = '\0';

    int m_previousInput = 0;
    MODE m_previousMode = NORMAL_MODE;

    void handleNormalModeInput(int input);
    void handleCommandModeInput(int input);
    void handleInsertModeInput(int input);
    void handleReplaceCharMode(int input);

    void handleCommandBufferInput();
    void handleDeleteCommands(int input);
    void handleFindCommand(int input);

    void clearRepetitionBuffer() { m_repetitionBuffer.clear(); }
    int repetitionCount();

    bool repeatedInput(int input) { return (input == m_previousInput) ;}

public:

    InputController(Editor* editor);

    void handleInput();
};
