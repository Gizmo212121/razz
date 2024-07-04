#include <ncurses.h>

void initNcurses()
{
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
}

int main(int argc, char* argv[])
{
    const char* fileName = "default";
    if (argc > 1) { fileName = argv[1] ; }

    initNcurses();

    printw("%s", fileName);
    refresh();
    getch();
    endwin();

    return 0;
}
