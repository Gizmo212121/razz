#pragma once

#include "Includes.h"

class Timer
{

public:

    Timer(const std::string& functionName);
    ~Timer();

    static void print();

private:

    std::string m_name;

    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;

    std::chrono::duration<double> duration;

    static std::vector<std::string> m_timerMessages;

};
