#pragma once

#include "Command.h"

#include <deque>
#include <memory>
#include <stdio.h>
#include <iostream>

class CommandQueue
{

private:

    const size_t m_maxCommandHistory = 100;
    size_t m_currentCommandCount = 0;
    size_t m_undoesSinceChange = 0;
    std::deque<std::unique_ptr<Command>> m_commands;

    size_t m_repetitionCounter = 0;
    std::deque<size_t> m_commandRepetitions;

public:

    template <typename ClassType>
    void execute(const int repetition = 1);
    void undo();
    void redo();

    void printRepetitionQueue() const;

};

extern template void CommandQueue::execute<Test1>(int repetitions);
extern template void CommandQueue::execute<Test2>(int repetitions);
extern template void CommandQueue::execute<Test3>(int repetitions);
extern template void CommandQueue::execute<Test4>(int repetitions);
