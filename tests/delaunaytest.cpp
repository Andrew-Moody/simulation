#include <gtest/gtest.h>

#include "mesh.h"


TEST(Delaunay, Normalization)
{
    using namespace moodysim;

    std::vector<Point3D> points{
        { 10.f, 10.f, 10.f }
    };

    DelaunayGenerator delaunay_gen{ points };

    delaunay_gen.normalize_points();

    const std::vector<Point3D>& norm_pts = delaunay_gen.get_points();

    bool pass = true;

    // All points should lie in the range 0 to 1
    for (const auto point : norm_pts)
    {
        if (point.x > 1.f || point.y > 1.f || point.z > 1.f ||
            point.x < 0.f || point.y > 0.f || point.z > 0.f)
        {
            pass = false;
        }
    }

    EXPECT_TRUE(pass);
}

TEST(Delaunay, SortPoints)
{
    using namespace moodysim;
    //sort_points();
}

TEST(Delaunay, FindEnclosingTriangle)
{
    using namespace moodysim;

    std::vector<Point3D> test_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 5.f, 0.f },
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.f, 0.f },
    };

    std::vector<std::array<int, 3>> test_triangles{
        { 0, 1, 2 }
    };

    std::vector<std::array<int, 3>> test_neighbors{};

    DelaunayGenerator delaunay_gen{ test_points, {}, test_triangles, test_neighbors };
    int result = delaunay_gen.find_enclosing_triangle();

    EXPECT_EQ(result, 0);
}

TEST(Delaunay, UpdateNeighbors)
{
    using namespace moodysim;

    std::vector<Point3D> test_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 5.f, 0.f },
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.f, 0.f },
        { 0.5f, 0.5f, 0.f },
        { -0.5f, 0.5f, 0.f },
        { 0.f, -0.75f, 0.f }
    };

    std::vector<std::array<int, 3>> test_triangles{
        { 0, 1, 2 },
        // Neighbors of triangle zero that need to be updated as a result of adding point 3
        { 0, 4, 1 },
        { 1, 5, 2 },
        { 2, 6, 0 },
        // "New" triangles being inserted as a result of adding point 3 inside triangle 0
        { 3, 0, 1 },
        { 3, 1, 2 },
        { 3, 2, 0 }
    };

    std::vector<std::array<int, 3>> test_neighbors{
        { 1, 2, 3 },
        // Neighbors of the original triangle
        { -1, 0, -1 },
        { -1, 0, -1 },
        { -1, 0, -1 },
        // The new triangles are given the correct neighbors when added
        { 6, 1, 5 },
        { 4, 2, 6 },
        { 5, 3, 4 }
    };

    DelaunayGenerator delaunay_gen{ test_points, {}, test_triangles, test_neighbors };

    int target{};
    int old_neighbor{};
    int new_neighbor{};

    delaunay_gen.update_adjacent(1, 0, 4);

    delaunay_gen.update_adjacent(2, 0, 5);

    delaunay_gen.update_adjacent(3, 0, 6);

    const std::vector<std::array<int, 3>>& neighbors{ delaunay_gen.get_neighbors() };

    EXPECT_EQ(neighbors[1][0], -1);
    EXPECT_EQ(neighbors[1][1], 4);
    EXPECT_EQ(neighbors[1][2], -1);

    EXPECT_EQ(neighbors[2][0], -1);
    EXPECT_EQ(neighbors[2][1], 5);
    EXPECT_EQ(neighbors[2][2], -1);

    EXPECT_EQ(neighbors[3][0], -1);
    EXPECT_EQ(neighbors[3][1], 6);
    EXPECT_EQ(neighbors[3][2], -1);
}

TEST(Delaunay, CheckDelaunay)
{
    using namespace moodysim;

    std::vector<Point3D> test_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 5.f, 0.f },
        { 0.5f, -0.5f, 0.f },
        { 0.5f, 0.5f, 0.f },
        { -0.5f, 0.5f, 0.f },
    };

    std::vector<std::array<int, 3>> test_triangles{
        { 0, 1, 2 },
        { 0, 3, 1 },
        { 1, 4, 2 },
    };

    std::vector<std::array<int, 3>> test_neighbors{
        { 1, 2, -1 },
        { -1, 0, -1 },
        { -1, 0, -1 },
    };

    DelaunayGenerator delaunay_gen{ test_points, {}, test_triangles, test_neighbors };

    int tri_l{ 0 };
    int tri_r1{ 1 };
    int tri_r2{ 2 };

    // Check if a quad needs to have its diagonal swapped (result should be false)
    EXPECT_FALSE(delaunay_gen.check_delaunay(tri_l, tri_r1));

    // Check if a quad needs to have its diagonal swapped (result should be true)
    EXPECT_TRUE(delaunay_gen.check_delaunay(tri_l, tri_r2));
}

TEST(Delaunay, SwapTriangles)
{
    using namespace moodysim;

    std::vector<Point3D> test_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 5.f, 0.f },
        { 0.5f, -0.5f, 0.f },
        { -0.5f, 0.5f, 0.f }
    };

    std::vector<std::array<int, 3>> test_triangles{
        { 0, 1, 2 },
        { 3, 2, 1 },
    };

    std::vector<std::array<int, 3>> test_neighbors{
        { -1, 1, -1 },
        { -1, 0, -1 }
    };

    DelaunayGenerator delaunay_gen{ test_points, {}, test_triangles, test_neighbors };

    int tri_l{ 0 };
    int tri_r{ 1 };

    // Swap the diagonal of a quad
    delaunay_gen.swap_triangles(tri_l, tri_r);

    const std::vector<std::array<int, 3>>& triangles{ delaunay_gen.get_triangles() };

    const std::vector<std::array<int, 3>>& neighbors{ delaunay_gen.get_neighbors() };

    std::vector<std::array<int, 3>> triangles_solution{
        { 0, 1, 3 },
        { 0, 3, 2 },
    };

    std::vector<std::array<int, 3>> neighbors_solution{
        { -1, -1, 1 },
        { 0, -1, -1 }
    };

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            EXPECT_EQ(triangles[i][j], triangles_solution[i][j]);
            EXPECT_EQ(neighbors[i][j], neighbors_solution[i][j]);
        }
    }
}