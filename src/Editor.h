#pragma once

#include "CommandQueue.h"
#include "Buffer.h"
#include "InputController.h"
#include "View.h"

#include <ncurses.h>

class Editor
{

private:

    bool m_running = true;

    CommandQueue m_commandHistory;
    Buffer m_buffer;
    InputController m_inputController;
    View m_view;
    MODE m_currentMode;

private:

    void initNcurses();

public:

    Editor();

    void run();
    void quit();

    // SETTERS
    void setMode(const MODE mode) { m_currentMode = mode ; }

    // GETTERS
    MODE mode() const { return m_currentMode ; }
    CommandQueue& commandQueue() { return m_commandHistory ; }

};
