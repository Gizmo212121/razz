#include "MacroRegisters.h"

bool MacroRegisters::isValidRegisterKey(const int key)
{
    if (key >= 48 && key <= 57)
    {
        return true;
    }
    else if (key >= 65 && key <= 90)
    {
        return true;
    }
    else if (key >= 97 && key <= 122)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int MacroRegisters::getIntegerIndexFromRegisterKey(const int key) const
{
    if (key >= 48 && key <= 57)
    {
        return key - 48;
    }
    else if (key >= 65 && key <= 90)
    {
        return key - 55;
    }
    else if (key >= 97 && key <= 122)
    {
        return key - 61;
    }
    else
    {
        endwin();
        std::cerr << "Unexpected key into macro register: " << key << '\n';
        abort();
    }
}

const std::vector<int>& MacroRegisters::operator [] (const int key) const
{
    int index = getIntegerIndexFromRegisterKey(key);

    return m_registers[index];
}

void MacroRegisters::add(const int key, const int input)
{
    int index = getIntegerIndexFromRegisterKey(key);

    m_registers[index].push_back(input);
}

void MacroRegisters::clear(const int key)
{
    int index = getIntegerIndexFromRegisterKey(key);

    m_registers[index].clear();
}
