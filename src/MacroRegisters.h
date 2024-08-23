#pragma once

#include "Includes.h"

class MacroRegisters
{

private:

    std::vector<int> m_registers[62];

    int getIntegerIndexFromRegisterKey(const int key) const;

public:

    // Given a register key, index into the registers and push back the input
    void add(const int key, const int input);
    // Clears a register given a key
    void clear(const int key);
    const std::vector<int>& operator [] (const int key) const;

    static bool isValidRegisterKey(const int key);

};
