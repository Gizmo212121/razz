#pragma once

#include <string>
#include <vector>

class Editor;

enum KEYS
{
    CTRL_C = 3,
    ENTER = 10,
    ESCAPE = 27,
    COLON = 58,
    BACKSPACE = 263,
};

class InputController
{

private:

    Editor* m_editor;

    std::string m_commandBuffer;

    void handleNormalModeInput();
    void handleCommandModeInput();
    void handleInsertModeInput();

public:

    InputController(Editor* editor);

    void handleInput();
};
