#include "Editor.h"

int main(int argc, char* argv[])
{
    const char* fileName = "default";
    if (argc > 1) { fileName = argv[1] ; }

    Editor editor;

    editor.run();

    return 0;
}
