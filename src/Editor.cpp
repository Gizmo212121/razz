#include "Editor.h"
#include "Includes.h"

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

    start_color();

    if (!has_colors() || COLORS < 255)
    {
        endwin();
        std::cerr << "Your terminal does not support colors!\n";
        exit(1);
    }

    // if (!can_change_color())
    // {
    //     endwin();
    //     std::cerr << "Your terminal does not support colors!\n";
    //     exit(1);
    // }

    // Highlighter orange
    // init_color(MEDIUM_PURPLE4, 999, 619, 392);
    init_color(MEDIUM_PURPLE4, 101, 106, 149);

    init_pair(LINE_NUMBER_ORANGE, ORANGE1, MEDIUM_PURPLE4);
    // init_pair(BACKGROUND, COLOR_WHITE, PALE_TURQUOISE4);
    init_pair(BACKGROUND, COLOR_WHITE, MEDIUM_PURPLE4);

    bkgd(COLOR_PAIR(BACKGROUND));

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
