#pragma once

#include <ncurses.h>

class Editor
{
    
private:

    bool m_running = true;

private:

    void initNcurses();
    void quit();

public:

    Editor();

    void run();

};
