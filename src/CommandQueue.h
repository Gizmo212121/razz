#pragma once

#include "Command.h"

class Buffer;
class View;

class CommandQueue
{

private:

    const size_t m_maxCommandHistory = 50000;
    size_t m_currentCommandCount = 0;
    size_t m_undoesSinceChange = 0;
    std::deque<std::unique_ptr<Command>> m_commands;

    size_t m_repetitionCounter = 0;
    std::deque<size_t> m_commandRepetitions;

    size_t m_consecutiveBatchCommands = 0;

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

    void undo();
    void redo();

    size_t currentCommandCount() const { return m_currentCommandCount; }

    template <typename CommandType, typename ... CommandArgs>
    void execute(bool batch, int repetition, CommandArgs&&... commandArgs)
    {
        assert(repetition >= 0);

        if (repetition == 0) { return; }

        bool modifiesBuffer;

        for (int repeat = 0; repeat < repetition; repeat++)
        {
            std::unique_ptr<Command> command;

            if (batch)
            {
                repetition = 1;

                if (m_consecutiveBatchCommands == 0)
                {
                    command = std::make_unique<CommandType>(
                            m_editor, m_buffer, m_view, m_commandQueue, true, true,
                            std::forward<CommandArgs>(commandArgs)...);
                }
                else
                {
                    command = std::make_unique<CommandType>(
                            m_editor, m_buffer, m_view, m_commandQueue, true, false,
                            // m_editor, m_buffer, m_view, m_commandQueue, true, true,
                            std::forward<CommandArgs>(commandArgs)...);

                    m_commands[m_currentCommandCount - 1]->m_renderExecute = false;
                }

                m_commandRepetitions[m_currentCommandCount] = --m_repetitionCounter;

                m_consecutiveBatchCommands++;
            }
            else
            {
                m_consecutiveBatchCommands = 0;

                command = std::make_unique<CommandType>(
                        m_editor, m_buffer, m_view, m_commandQueue, (repeat == repetition - 1), (repeat == 0),
                        std::forward<CommandArgs>(commandArgs)...);
            }

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
            else
            {
                if (m_consecutiveBatchCommands > 0)
                {
                    m_commands[m_currentCommandCount - 1]->m_renderExecute = true;
                }
            }

            // std::cout << "Current command count: " << m_currentCommandCount << '\n';
            // std::cout << "SIZE OF DEQUE: " << m_commands.size() << '\n';
        }

        m_repetitionCounter++;
    }

};
