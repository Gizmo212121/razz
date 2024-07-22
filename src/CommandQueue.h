#pragma once

#include "Command.h"

#include <deque>
#include <memory>
#include <stdio.h>
#include <iostream>
#include <cassert>

class Buffer;
class View;

class CommandQueue
{

private:

    const size_t m_maxCommandHistory = 1000;
    size_t m_currentCommandCount = 0;
    size_t m_undoesSinceChange = 0;
    std::deque<std::unique_ptr<Command>> m_commands;

    size_t m_repetitionCounter = 0;
    std::deque<size_t> m_commandRepetitions;

    // POINTERS TO OBJECTS
    Editor* m_editor;
    Buffer* m_buffer;
    View* m_view;
    CommandQueue* m_commandQueue;


public:

    CommandQueue(Editor* editor, Buffer* buffer, View* view)
        : m_editor(editor), m_buffer(buffer), m_view(view), m_commandQueue(this)
    {
    }

    void printRepetitionQueue() const;


    void overrideRepetitionQueue() { m_commandRepetitions[m_currentCommandCount] = --m_repetitionCounter; };
    void overrideOverrideRepetitionBuffer() { m_repetitionCounter++; }

    void undo();
    void redo();

    template <typename CommandType, typename ... CommandArgs>
    void execute(const int repetition, CommandArgs&&... commandArgs)
    {
        assert(repetition >= 0);

        if (repetition == 0) { return; }

        bool modifiesBuffer;

        for (int repeat = 0; repeat < repetition; repeat++)
        {
            std::unique_ptr<Command> command = std::make_unique<CommandType>(
                    m_editor, m_buffer, m_view, m_commandQueue,
                    std::forward<CommandArgs>(commandArgs)...);

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
