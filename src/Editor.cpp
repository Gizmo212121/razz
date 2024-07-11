#include "Editor.h"

Editor::Editor(const std::string& fileName)
    : m_buffer(fileName), m_inputController(this), m_view(&m_buffer), m_currentMode(MODE::NORMAL_MODE)
{
    initNcurses();
}

void Editor::initNcurses()
{
    initscr();
    noecho();
    raw();
    // cbreak();
    keypad(stdscr, true);
    // nodelay(stdscr, TRUE);
}

void Editor::run()
{
    while (m_running)
    {
        m_view.display();
        m_inputController.handleInput();
    }
}

void Editor::quit()
{
    refresh();
    endwin();
    m_running = false;
}
