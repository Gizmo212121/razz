#include "Editor.h"

int main(int argc, char* argv[])
{
    Editor editor((argc > 1) ? argv[1] : "NO_NAME");

    editor.run();

    return 0;
}
