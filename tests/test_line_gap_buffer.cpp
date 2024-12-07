#include "test_main.cpp"

#include "../src/LineGapBuffer.h"

TEST_CASE("constructors", "[line_gap_buffer]")
{
    LineGapBuffer buffer(0);

    REQUIRE(buffer.bufferSize() == 0);
}
