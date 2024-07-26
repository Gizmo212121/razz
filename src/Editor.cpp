#include "Editor.h"

Editor::Editor(const std::string& fileName)
    : m_commandQueue(this, &m_buffer, &m_view), m_buffer(fileName, &m_view), m_inputController(this), m_view((initNcurses(), &m_buffer)), m_currentMode(MODE::NORMAL_MODE)
{
}

Editor::~Editor()
{
    m_view.normalCursor();
    refresh();
    endwin();
}

void Editor::initNcurses()
{
    initscr();
    noecho();
    raw();
    // cbreak();
    keypad(stdscr, true);
}

void Editor::run()
{
    m_view.display();

    while (m_running)
    {
        m_inputController.handleInput();
    }
}

void Editor::quit()
{
    m_running = false;
}
