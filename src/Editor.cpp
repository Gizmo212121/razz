#include "Editor.h"

Editor::Editor()
    : m_inputController(this), m_currentMode(MODE::NORMAL_MODE)
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
        m_inputController.handleInput();
    }
}

void Editor::quit()
{
    refresh();
    endwin();
    m_running = false;
}
