#include <gtest/gtest.h>

#include "graphics.h"

TEST(HelloTest, HelloMeshing)
{
    EXPECT_TRUE(graphics::hello_meshing());
}