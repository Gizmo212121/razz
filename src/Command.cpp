#include "Command.h"
#include <iostream>

void Test1::undo()
{
    std::cout << "UNDO COMMAND 1" << std::endl;
}

bool Test1::execute()
{
    std::cout << "COMMAND 1" << std::endl;
    return true;
}

void Test2::undo()
{
    std::cout << "UNDO COMMAND 2" << std::endl;
}

bool Test2::execute()
{
    std::cout << "COMMAND 2" << std::endl;
    return true;
}

void Test3::undo()
{
    std::cout << "UNDO COMMAND 3" << std::endl;
}

bool Test3::execute()
{
    std::cout << "COMMAND 3" << std::endl;
    return true;
}

void Test4::undo()
{
    std::cout << "UNDO COMMAND 4" << std::endl;
}

bool Test4::execute()
{
    std::cout << "COMMAND 4" << std::endl;
    return true;
}
