#pragma once

#include "Command.h"

#include <deque>
#include <memory>
#include <stdio.h>
#include <iostream>
#include <cassert>

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

    void printRepetitionQueue() const;

    void undo();
    void redo();

    template <typename CommandType, typename ... CommandArgs>
    void execute(const int repetition, CommandArgs&&... commandArgs)
    {
        assert(repetition > 0);

        bool modifiesBuffer;

        for (int repeat = 0; repeat < repetition; repeat++)
        {
            // If executing the command changes any buffers, add it to deque
            // std::unique_ptr<Command> command = std::make_unique<CommandType>();
            std::unique_ptr<Command> command = std::make_unique<CommandType>(std::forward<CommandArgs>(commandArgs)...);

            modifiesBuffer = command->execute();

            if (modifiesBuffer)
            {
                if (m_currentCommandCount < m_maxCommandHistory)
                {
                    if (m_currentCommandCount < m_commands.size())
                    {
                        m_commands[m_currentCommandCount] = std::move(command);
                        m_commandRepetitions[m_currentCommandCount] = m_repetitionCounter;
                    }
                    else
                    {
                        m_commands.push_back(std::move(command));
                        m_commandRepetitions.push_back(m_repetitionCounter);
                    }

                    m_currentCommandCount++;
                }
                else
                {
                    m_commands.push_back(std::move(command));
                    m_commands.pop_front();

                    m_commandRepetitions.push_back(m_repetitionCounter);
                    m_commandRepetitions.pop_front();
                }

                m_undoesSinceChange = 0;
            }

            // std::cout << "Current command count: " << m_currentCommandCount << '\n';
            // std::cout << "SIZE OF DEQUE: " << m_commands.size() << '\n';
        }

        m_repetitionCounter++;
    }

};
