#include "Editor.h"
#include "Includes.h"

Editor::Editor(const std::string& fileName)
    : m_commandQueue(this, &m_buffer, &m_view), m_buffer(fileName), m_inputController(this), m_view((initNcurses(), this), &m_buffer), m_currentMode(MODE::NORMAL_MODE)
{
}

Editor::~Editor()
{
    m_view.normalCursor();
    clear();
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

    const int COLOR_CONVERSION = 1000 / 255;

    if (can_change_color())
    {
        // Modified colors
        init_color(GREY11, 26 * COLOR_CONVERSION, 27 * COLOR_CONVERSION, 38 * COLOR_CONVERSION);
        init_color(SKY_BLUE2, 122 * COLOR_CONVERSION, 162 * COLOR_CONVERSION, 247 * COLOR_CONVERSION);
        init_color(DARK_OLIVE_GREEN3_3, 158 * COLOR_CONVERSION, 206 * COLOR_CONVERSION, 106 * COLOR_CONVERSION);
        init_color(LIGHT_GOLDENROD3, 224 * COLOR_CONVERSION, 175 * COLOR_CONVERSION, 104 * COLOR_CONVERSION);
        init_color(SANDY_BROWN, 255 * COLOR_CONVERSION, 158 * COLOR_CONVERSION, 100 * COLOR_CONVERSION);
        init_color(LIGHT_CORAL, 247 * COLOR_CONVERSION, 118 * COLOR_CONVERSION, 142 * COLOR_CONVERSION);
        init_color(GREY19, 41 * COLOR_CONVERSION, 46 * COLOR_CONVERSION, 66 * COLOR_CONVERSION);
        init_color(GREY30, 59 * COLOR_CONVERSION, 66 * COLOR_CONVERSION, 97 * COLOR_CONVERSION);
        init_color(INDIAN_RED1_1, 255 * COLOR_CONVERSION, 75 * COLOR_CONVERSION, 75 * COLOR_CONVERSION);
        init_color(MEDIUM_PURPLE1_1, 187 * COLOR_CONVERSION, 154 * COLOR_CONVERSION, 247 * COLOR_CONVERSION);
        init_color(MEDIUM_PURPLE4, 57 * COLOR_CONVERSION, 74 * COLOR_CONVERSION, 125 * COLOR_CONVERSION);
    }

    init_pair(BACKGROUND, COLOR_WHITE, GREY11);

    init_pair(NORMAL_MODE_PAIR, GREY11, SKY_BLUE3);
    init_pair(INSERT_MODE_PAIR, GREY11, DARK_OLIVE_GREEN3_3);
    init_pair(COMMAND_MODE_PAIR, GREY11, LIGHT_GOLDENROD3);
    init_pair(REPLACE_CHAR_MODE_PAIR, GREY11, LIGHT_CORAL);
    init_pair(VISUAL_MODE_PAIR, GREY11, MEDIUM_PURPLE1_1);
    init_pair(VISUAL_LINE_MODE_PAIR, GREY11, MEDIUM_PURPLE2_1);
    init_pair(VISUAL_BLOCK_MODE_PAIR, GREY11, MEDIUM_PURPLE3_1);

    init_pair(LINE_NUMBER_ORANGE, SANDY_BROWN, GREY11);
    init_pair(LINE_NUMBER_GREY, GREY30, GREY11);
    init_pair(VISUAL_HIGHLIGHT_PAIR, COLOR_WHITE, MEDIUM_PURPLE4);

    init_pair(PATH_COLOR_PAIR, COLOR_WHITE, GREY19);
    init_pair(ERROR_MESSAGE_PAIR, INDIAN_RED1_1, GREY11);

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
