#include "CommandQueue.h"
#include <cassert>
#include <memory>

// In future, construct with save file to fill command queue
/* CommandQueue::CommandQueue()
{

} */

template <typename CommandType>
void CommandQueue::execute(const int repetition)
{
    assert(repetition > 0);

    bool modifiesBuffer;

    for (int repeat = 0; repeat < repetition; repeat++)
    {
        // If executing the command changes any buffers, add it to deque
        std::unique_ptr<Command> command = std::make_unique<CommandType>();

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

void CommandQueue::undo()
{
    try
    {
        if (m_currentCommandCount <= 0)
        {
            std::cout << "Already at oldest change!\n";
            return;
        }

        size_t repetitionNumber = m_commandRepetitions[m_currentCommandCount - 1];

        // std::cout << "REP NUM: " << repetitionNumber << '\n';
        // std::cout << "INSIDE WHILE: " << m_commandRepetitions[m_currentCommandCount - 1] << '\n';

        while (m_currentCommandCount > 0 && m_commandRepetitions[m_currentCommandCount - 1] == repetitionNumber)
        {
            if (m_currentCommandCount <= m_commands.size())
            {
                if (m_commands[m_currentCommandCount - 1])
                {
                    m_commands[m_currentCommandCount - 1]->undo();
                }
                else
                {
                    std::cerr << "Error: Command at index " << m_currentCommandCount - 1 << " is a nullptr.\n";
                }
            }
            else
            {
                std::cerr << "Error: m_currentCommandCount (" << m_currentCommandCount << ") exceeds m_commands size (" << m_commands.size() << ").\n";
            }

            // std::cout << "BOZO2\n";

            m_undoesSinceChange++;
            // std::cout << "BOZO3\n";
            m_currentCommandCount--;
            // std::cout << "BOZO4\n";
        }
    }
    catch (const std::out_of_range& e)
    {
        std::cerr << "Out of range error: " << e.what() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }

    // std::cout << "Current command count: " << m_currentCommandCount << '\n';
    // std::cout << "SIZE OF DEQUE: " << m_commands.size() << '\n';
}

void CommandQueue::redo()
{
    // std::cout << "BEFORE REDO: \n\t UNDOS: " << m_undoesSinceChange << '\n';
    // std::cout << "Current command count: " << m_currentCommandCount << '\n';
    // std::cout << "SIZE OF DEQUE: " << m_commands.size() << '\n';

    if (m_undoesSinceChange > 0)
    {
        size_t repetitionNumber = m_commandRepetitions[m_currentCommandCount];

        while (m_undoesSinceChange > 0 && m_commandRepetitions[m_currentCommandCount] == repetitionNumber)
        {
            m_undoesSinceChange--;
            m_commands[m_currentCommandCount++]->execute();
        }
    }
    else
    {
        std::cout << "Already at newest change!\n";
    }
}

void CommandQueue::printRepetitionQueue() const
{
    for (auto it : m_commandRepetitions)
    {
        std::cout << it << ", ";
    }

    std::cout << std::endl;
}

template void CommandQueue::execute<Test1>(int repetitions);
template void CommandQueue::execute<Test2>(int repetitions);
template void CommandQueue::execute<Test3>(int repetitions);
template void CommandQueue::execute<Test4>(int repetitions);
