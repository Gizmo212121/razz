#pragma once

class Buffer;

class View
{

private:

    Buffer* m_buffer;

public:

    View(Buffer* buffer);

    void display();

};
