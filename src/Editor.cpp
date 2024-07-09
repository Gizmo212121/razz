#include "Editor.h"

Editor::Editor()
{
    initNcurses();
}

void Editor::initNcurses()
{
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
}

void Editor::run()
{
    while (m_running)
    {
        refresh();
    }
}

void Editor::quit()
{
    endwin();
    m_running = false;
}
