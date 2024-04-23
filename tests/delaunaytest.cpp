#include <gtest/gtest.h>

#include "mesh.h"
#include "graphics.h"
#include "surfacemeshdata.h"

// Utility function to check if two triangle or neighbor sets are equal
bool array_compare_equal(const std::vector<std::array<int, 3>>& expected, const std::vector<std::array<int, 3>>& result)
{
    size_t array_size{ expected.size() };

    if (array_size != result.size())
    {
        return false;
    }

    for (int i = 0; i < array_size; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (expected[i][j] != result[i][j])
            {
                return false;
            }
        }
    }

    return true;
}

TEST(Delaunay, Normalization)
{
    using namespace moodysim;

    std::vector<Point3D> points{
        { 10.f, -10.f, 0.f },
        { -10.f, 0.f, 0.f },
        { 0.f, 10.f, 0.f }
    };

    DelaunayGenerator delaunay_gen{ points };

    delaunay_gen.normalize_points();

    const std::vector<Point3D>& norm_pts = delaunay_gen.get_points();

    bool pass = true;

    // All points should lie in the range 0 to 1
    for (const auto point : norm_pts)
    {
        if (point.x > 1.f || point.y > 1.f || point.z > 1.f ||
            point.x < 0.f || point.y < 0.f || point.z < 0.f)
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

    std::vector<Point3D> input_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.5f, 0.f },
        { -0.5f, -0.5f, 0.f },
        { 0.f, 0.f, 0.f },
    };

    std::vector<std::array<int, 3>> input_triangles{
        { 0, 1, 2 }
    };

    std::vector<std::array<int, 3>> input_neighbors{};

    DelaunayGenerator delaunay_gen{ input_points, {}, input_triangles, input_neighbors };

    int result = delaunay_gen.find_enclosing_triangle(3);

    EXPECT_EQ(result, 0);
}

TEST(Delaunay, UpdateNeighbors)
{
    using namespace moodysim;

    std::vector<Point3D> input_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.5f, 0.f },
        { -0.5f, -0.5f, 0.f },
        { 0.f, 0.f, 0.f },
        { 0.5f, 0.5f, 0.f },
        { -0.5f, 0.5f, 0.f },
        { 0.f, -0.75f, 0.f }
    };

    std::vector<std::array<int, 3>> input_triangles{
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

    std::vector<std::array<int, 3>> input_neighbors{
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

    std::vector<std::array<int, 3>> expected_neighbors{
        { 1, 2, 3 },
        { -1, 4, -1 },
        { -1, 5, -1 },
        { -1, 6, -1 },
        { 6, 1, 5 },
        { 4, 2, 6 },
        { 5, 3, 4 }
    };

    DelaunayGenerator delaunay_gen{ input_points, {}, input_triangles, input_neighbors };

    int target{};
    int old_neighbor{};
    int new_neighbor{};

    delaunay_gen.update_adjacent(1, 0, 4);

    delaunay_gen.update_adjacent(2, 0, 5);

    delaunay_gen.update_adjacent(3, 0, 6);

    const std::vector<std::array<int, 3>>& neighbors{ delaunay_gen.get_neighbors() };

    EXPECT_TRUE(array_compare_equal(expected_neighbors, delaunay_gen.get_neighbors()));

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

    std::vector<Point3D> input_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.5f, 0.f },
        { -0.5f, -0.5f, 0.f },
        { 0.5f, 0.5f, 0.f },
        { -0.1f, 0.0f, 0.f } // shifted from {-0.5f, 0.5f, 0.f} to result in a thin triangle 1, 4, 2
    };

    std::vector<std::array<int, 3>> input_triangles{
        { 0, 1, 2 },
        { 0, 3, 1 },
        { 1, 4, 2 },
    };

    std::vector<std::array<int, 3>> input_neighbors{
        { 1, 2, -1 },
        { -1, 0, -1 },
        { -1, 0, -1 },
    };

    DelaunayGenerator delaunay_gen{ input_points, {}, input_triangles, input_neighbors };

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

    std::vector<Point3D> input_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.5f, 0.f },
        { -0.5f, -0.5f, 0.f },
        { -0.5f, 0.5f, 0.f }
    };

    std::vector<std::array<int, 3>> input_triangles{
        { 0, 1, 2 },
        { 3, 2, 1 },
    };

    std::vector<std::array<int, 3>> input_neighbors{
        { -1, 1, -1 },
        { -1, 0, -1 }
    };

    std::vector<std::array<int, 3>> expected_triangles{
        { 0, 1, 3 },
        { 0, 3, 2 },
    };

    std::vector<std::array<int, 3>> expected_neighbors{
        { -1, -1, 1 },
        { 0, -1, -1 }
    };

    DelaunayGenerator delaunay_gen{ input_points, {}, input_triangles, input_neighbors };

    int tri_l{ 0 };
    int tri_r{ 1 };

    // Swap the diagonal of a quad
    delaunay_gen.swap_triangles(tri_l, tri_r);

    // Check that the resulting triangles and neighbors match expected
    EXPECT_TRUE(array_compare_equal(expected_triangles, delaunay_gen.get_triangles()));
    EXPECT_TRUE(array_compare_equal(expected_neighbors, delaunay_gen.get_neighbors()));
}


TEST(Delaunay, SwapTrianglePositions)
{
    using namespace moodysim;

    std::vector<Point3D> input_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.5f, 0.f },
        { -0.5f, -0.5f, 0.f },
        { 0.5f, 0.5f, 0.f },
        { -0.5f, 0.5f, 0.f }
    };

    std::vector<std::array<int, 3>> input_triangles{
        { 0, 1, 2 },
        { 4, 2, 1 },
        { 0, 3, 1 }
    };

    std::vector<std::array<int, 3>> input_neighbors{
        { 2, 1, -1 },
        { -1, 0, -1 },
        { -1, 0, -1 }
    };

    std::vector<std::array<int, 3>> expected_triangles{
        { 0, 3, 1 },
        { 4, 2, 1 },
        { 0, 1, 2 }
    };

    std::vector<std::array<int, 3>> expected_neighbors{
        { -1, 2, -1 },
        { -1, 2, -1 },
        { 0, 1, -1 }
    };

    DelaunayGenerator delaunay_gen{ input_points, {}, input_triangles, input_neighbors };

    int tri_a{ 0 };
    int tri_b{ 2 };

    // Swap the positions of two triangles in the triangles list
    delaunay_gen.swap_triangle_positions(tri_a, tri_b);

    // Check that the resulting triangles and neighbors match expected
    EXPECT_TRUE(array_compare_equal(expected_triangles, delaunay_gen.get_triangles()));
    EXPECT_TRUE(array_compare_equal(expected_neighbors, delaunay_gen.get_neighbors()));
}


TEST(Delaunay, PopTriangle)
{
    using namespace moodysim;

    std::vector<Point3D> input_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.5f, 0.f },
        { -0.5f, -0.5f, 0.f },
        { -0.5f, 0.5f, 0.f }
    };

    std::vector<std::array<int, 3>> input_triangles{
        { 0, 1, 2 },
        { 3, 2, 1 },
    };

    std::vector<std::array<int, 3>> input_neighbors{
        { -1, 1, -1 },
        { -1, 0, -1 }
    };

    std::vector<std::array<int, 3>> expected_triangles{
        { 0, 1, 2 }
    };

    std::vector<std::array<int, 3>> expected_neighbors{
        { -1, -1, -1 }
    };

    DelaunayGenerator delaunay_gen{ input_points, {}, input_triangles, input_neighbors };

    // Remove the last triangle from the list
    delaunay_gen.pop_triangle();

    // Check that the resulting triangles and neighbors match expected
    EXPECT_TRUE(array_compare_equal(expected_triangles, delaunay_gen.get_triangles()));
    EXPECT_TRUE(array_compare_equal(expected_neighbors, delaunay_gen.get_neighbors()));
}


TEST(Delaunay, Generation)
{
    using namespace moodysim;

    /* std::vector<Point3D> input_points{
        { 0.5f, -0.5f, 0.f },
        { 0.f, 0.5f, 0.f },
        { -0.5f, -0.5f, 0.f },
        { 0.f, 0.f, 0.f },
        { 0.5f, 0.5f, 0.f },
        { -0.5f, 0.5f, 0.f },
        { 0.f, -0.75f, 0.f }
    }; */

    std::vector<Point3D> input_points{ generate_sample_points(1.0f, 5) };

    DelaunayGenerator delaunay_gen{ input_points };

    SurfaceMeshData mesh_data = delaunay_gen.generate_delaunay_mesh();

    GLFWContext glfw_context{};

    Application application{};

    application.setup();

    application.add_mesh(std::move(mesh_data));

    application.run();
}