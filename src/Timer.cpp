#include "Timer.h"

std::vector<std::string> Timer::m_timerMessages;

void Timer::print()
{
    for (const std::string& message : m_timerMessages)
    {
        std::cout << message << '\n';
    }
}

Timer::Timer(const std::string& functionName)
{
    start = std::chrono::high_resolution_clock::now();
    m_name = functionName;
}

Timer::~Timer()
{
    end = std::chrono::high_resolution_clock::now();

    duration = end - start;

    m_timerMessages.push_back(m_name + " elapsed time: " + std::to_string(duration.count()));
}
