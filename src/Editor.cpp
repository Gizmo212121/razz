#include "Editor.h"

Editor::Editor(const std::string& fileName)
    : m_commandQueue(this, &m_buffer, &m_view), m_buffer(fileName), m_inputController(this), m_view(&m_buffer), m_currentMode(MODE::NORMAL_MODE)
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
    m_view.display();

    while (m_running)
    {
        // std::pair<int, int> cursorPos = m_buffer.getCursorPos();
        // move(10, 0);
        // printw("Cursor x: %d", cursorPos.second);
        // move(11, 0);
        // printw("Cursor y: %d", cursorPos.first);
        // move(12, 0);
        // printw("Last Cursor x: %d", m_buffer.cursorXBeforeYMove());
        m_inputController.handleInput();
    }
}

void Editor::quit()
{
    refresh();
    endwin();
    m_running = false;
}
