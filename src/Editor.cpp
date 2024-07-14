#include "Editor.h"

Editor::Editor(const std::string& fileName)
    : m_commandQueue(this, &m_buffer, &m_view), m_buffer(fileName, &m_view), m_inputController(this), m_view(&m_buffer), m_currentMode(MODE::NORMAL_MODE)
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
        // clear();
        // std::pair<int, int> cursorPos = m_buffer.getCursorPos();
        // move(60, 0);
        // printw("Cursor x: %d", cursorPos.second);
        // move(61, 0);
        // printw("Cursor y: %d", cursorPos.first);
        // move(62, 0);
        // printw("Last Cursor x: %d", m_buffer.cursorXBeforeYMove());
        // move(63, 0);
        // printw("GB PreIndex: %lu", m_buffer.getGapBuffer(cursorPos.first).preGapIndex());
        // move(64, 0);
        // printw("GB PostIndex: %lu", m_buffer.getGapBuffer(cursorPos.first).postGapIndex());
        // m_view.display();
        m_view.displayCurrentLineGapBuffer(m_buffer.getCursorPos().first);
        m_inputController.handleInput();
    }

    refresh();
    endwin();
}

void Editor::quit()
{
    m_running = false;
}
