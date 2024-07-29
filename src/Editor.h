#pragma once

#include "CommandQueue.h"
#include "Buffer.h"
#include "InputController.h"
#include "View.h"

class Editor
{

private:

    bool m_running = true;

    CommandQueue m_commandQueue;
    Buffer m_buffer;
    InputController m_inputController;
    View m_view;
    MODE m_currentMode;

private:

    void initNcurses();

public:

    Editor(const std::string& fileName);
    ~Editor();

    void run();
    void quit();

    // SETTERS
    void setMode(const MODE mode) { m_currentMode = mode ; }

    // GETTERS
    CommandQueue& commandQueue() { return m_commandQueue ; }
    Buffer& buffer() { return m_buffer ; }
    View& view() { return m_view; }
    MODE mode() const { return m_currentMode ; }

};
