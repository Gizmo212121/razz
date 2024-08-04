#include "CommandQueue.h"

// TODO: In future, construct with save file to fill command queue

void CommandQueue::undo()
{
    try
    {
        if (m_currentCommandCount <= 2) // Temp fix here for the bullshit error happening with character-deleting in a new buffer.
        {
            // std::cout << "Already at oldest change!\n";
            // TODO: Send signal to command buffer. Maybe have this function return a bool to say if it worked or not? Then we can contextualize the return statement, giving a proper warning
            return;
        }

        size_t repetitionNumber = m_commandRepetitions[m_currentCommandCount - 1];

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

            m_undoesSinceChange++;

            m_currentCommandCount--;
        }
    }
    catch (const std::out_of_range& e)
    {
        endwin();
        std::cerr << "Out of range error: " << e.what() << '\n';
        exit(1);
    }
    catch (const std::exception& e)
    {
        endwin();
        std::cerr << "Exception: " << e.what() << '\n';
        exit(1);
    }
}

void CommandQueue::redo()
{
    if (m_undoesSinceChange > 0)
    {
        size_t repetitionNumber = m_commandRepetitions[m_currentCommandCount];

        while (m_undoesSinceChange > 0 && m_commandRepetitions[m_currentCommandCount] == repetitionNumber)
        {
            m_undoesSinceChange--;
            m_commands[m_currentCommandCount++]->redo();
        }
    }
    else
    {
        // std::cout << "Already at newest change!\n";
        // TODO: Same thing here
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
