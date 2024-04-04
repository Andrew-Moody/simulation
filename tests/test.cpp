#include <gtest/gtest.h>

#include "graphics.h"

TEST(HelloTest, HelloMeshing)
{
    EXPECT_TRUE(moodysim::hello_simulation());
}