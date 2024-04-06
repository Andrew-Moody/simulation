#include "mesh.h"

#include <vector>
#include <array>
#include <stack>

#include "surfacemeshdata.h"

namespace moodysim
{
    SurfaceMeshData generate_sample_mesh()
    {
        std::vector<SMVertex> vertices{};
        std::vector<unsigned int> indices{};

        // number of grid points or nodes in each dimension
        constexpr int xsize{ 11 };
        constexpr int ysize{ 6 };

        constexpr float scale = 1.0f / (xsize - 1);
        constexpr float xoffset = -0.5f;
        constexpr float yoffset = scale * (ysize - 1) / 2.0f;

        vertices.reserve(xsize * ysize);
        indices.reserve(6 * (xsize - 1) * (ysize - 1));

        for (int j = 0; j < ysize; ++j)
        {
            for (int i = 0; i < xsize; ++i)
            {
                float x = i * scale + xoffset;
                float y = -j * scale + yoffset;
                float z = 0.0f;

                vertices.push_back({ x, y, z, 0.0f, 0.0f, 0.0f });
            }
        }

        for (int j = 0; j < ysize - 1; ++j)
        {
            for (int i = 0; i < xsize - 1; ++i)
            {
                // four points around a quad clockwise from topleft
                int idx0 = i + xsize * j;   // topleft
                int idx1 = idx0 + 1;        // topright
                int idx2 = idx0 + 1 + xsize;    // bottomright
                int idx3 = idx0 + xsize;        // bottomleft

                // Two triangles to make a quad
                indices.push_back(idx0);
                indices.push_back(idx1);
                indices.push_back(idx2);

                indices.push_back(idx0);
                indices.push_back(idx2);
                indices.push_back(idx3);
            }
        }

        return SurfaceMeshData{ std::move(vertices), std::move(indices) };
    }

    std::vector<Point3D> normalize_points(const std::vector<Point3D>& points)
    {
        return std::vector<Point3D>{};
    }

    int find_adjacent(int i, int j, const std::vector<std::array<int, 3>>& neighbors)
    {
        // For two adjacent triangles i and j find the neighbor index for triangle i
        // that matches the triangle j

        return -1;
    }

    SurfaceMeshData generate_delaunay_mesh(const std::vector<Point3D>& points)
    {
        // Indices into the point vector that form each triangle
        std::vector<std::array<int, 3>> triangles;

        // Indices into the triangles vector that denote adjacent triangles
        std::vector<std::array<int, 3>> adjacency;

        // number of points not counting the super triangle
        int num_pts{ static_cast<int>(points.size()) };

        // Create a normalized copy of the points vector (need to keep original values)
        std::vector<Point3D> norm_pts{ normalize_points(points) };

        // If sorting into bins, do it here (would need to map sorted points back to the original points)

        // Add super triangle 
        norm_pts.push_back({ -100.f, -100.f, 0.f });
        norm_pts.push_back({ 100.f, -100.f, 0.f });
        norm_pts.push_back({ 0.f, 100.f, 0.f });
        triangles.push_back({ num_pts, num_pts + 1, num_pts + 2 });
        adjacency.push_back({ -1, -1, -1 });


        // A stack is used to track triangles that need to be checked
        std::stack<int> tri_stack{};

        // Add each point one at a time fixing any triangles that violate the delaunay condition
        for (int p = 0; p < num_pts; ++p)
        {
            // First determine which triangle the new point is inside
            int enclosing_tri_idx{};
            std::array<int, 3> enclosing_tri{ triangles[enclosing_tri_idx] };
            std::array<int, 3> enclosing_adj{ adjacency[enclosing_tri_idx] };

            // Delete the enclosing triangle and create 3 new triangles between
            // the enclosing vertices and the new vertex p (Always make p the first vertex)
            // replace enclosing triangle with the first new one and add the other two to the vector
            // Keep vertex indices ordered counter-clockwise for each triangle 
            triangles[enclosing_tri_idx] = { p, enclosing_tri[0], enclosing_tri[1] };
            triangles.push_back({ p, enclosing_tri[1], enclosing_tri[2] });
            triangles.push_back({ p, enclosing_tri[2], enclosing_tri[0] });

            // Indices for the new triangles
            int tri_0{ enclosing_tri_idx };
            int tri_1{ static_cast<int>(triangles.size()) - 2 };
            int tri_2{ static_cast<int>(triangles.size()) - 1 };

            // When adding to the adjacency list make the opposite adjacent triangle to p
            // the middle adjacency entry so we know when popping a triangle t from the stack
            // that the opposite adjacent edge to point triangles[t][0] is adjacency[t][1] (not 2)

            // The statement "In general for element I in the stack, the opposite adjacent triangle
            // is given by E(2, I)" Confused me since fortran starts arrays at 1 not 0 and the paper
            // didn't specify a relative position i.e. middle vs last but it turns out to be middle
            // so actually we need E(1, I) to be opposite adjacent of p not E(2, I)

            // Update existing adjanceny entry
            adjacency[tri_0][0] = tri_2;
            adjacency[tri_0][1] = enclosing_adj[0];
            adjacency[tri_0][2] = tri_1;

            //adjacency[tri_1][0] = tri_0;
            //adjacency[tri_1][1] = enclosing_adj[1];
            //adjacency[tri_1][2] = tri_2;

            //adjacency[tri_2][0] = tri_1;
            //adjacency[tri_2][1] = enclosing_adj[2];
            //adjacency[tri_2][2] = tri_0;

            // Add adjacency entries for the two new triangles
            adjacency.push_back({ tri_0, enclosing_adj[1], tri_2 });
            adjacency.push_back({ tri_1, enclosing_adj[2], tri_0 });


            // Place the new triangles containing p in the stack as long as the edges opposite
            // p have a neighboring triangle (i.e. is not on a boundary t = -1)
            // While doing this update the adjacency list for the enclosing triangles neighbors
            // as long as they exist (!= -1) The first one enclosing_adj[0] does not need to be 
            // updated since it already points to tri_0 since we reused but it wont hurt

            if (adjacency[tri_0][1] != -1)
            {
                tri_stack.push(tri_0);
            }
            if (adjacency[tri_1][1] != -1)
            {

                tri_stack.push(tri_1);
            }
            if (adjacency[tri_2][1] != -1)
            {

                tri_stack.push(tri_2);
            }


            // Check Delaunay condition and swap as needed propogating via the stack

        }

        std::vector<SMVertex> vertices{};
        std::vector<unsigned int> indices{};
        return SurfaceMeshData{ vertices, indices };
    }
}