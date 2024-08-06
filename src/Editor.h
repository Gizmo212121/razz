#pragma once

#include "CommandQueue.h"
#include "Buffer.h"
#include "InputController.h"
#include "View.h"
#include "Clipboard.h"
#include <string>

class Editor
{

private:

    bool m_running = true;

    MODE m_currentMode;
    Buffer m_buffer;
    View m_view;
    CommandQueue m_commandQueue;
    InputController m_inputController;
    Clipboard m_clipBoard;

    std::vector<std::string> m_benchmarkMessages;

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
    const InputController& inputController() { return m_inputController; }
    View& view() { return m_view; }
    MODE mode() const { return m_currentMode ; }
    Clipboard& clipBoard() { return m_clipBoard; }

};
