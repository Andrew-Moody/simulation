#include "mesh.h"

#include <iostream>
#include <vector>
#include <array>
#include <stack>

#include "surfacemeshdata.h"

namespace moodysim
{
    // Repurpose to generate point cloud
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

    SurfaceMeshData DelaunayGenerator::generate_delaunay_mesh()
    {
        triangulate();

        std::vector<SMVertex> vertices{};
        std::vector<unsigned int> indices{};
        return SurfaceMeshData{ vertices, indices };
    }

    void DelaunayGenerator::triangulate()
    {
        // Indices into the point vector that form each triangle
        std::vector<std::array<int, 3>> triangles;

        // Indices into the triangles vector that denote adjacent triangles
        std::vector<std::array<int, 3>> neighbors;

        // number of points not counting the super triangle
        int num_pts{ static_cast<int>(points_.size()) };

        // Normalize the points vector
        normalize_points();

        // If sorting into bins, do it here (would need to map sorted points back to the original points)

        // Add super triangle (-1 denotes no neighbor for that edge)
        points_.push_back({ -100.f, -100.f, 0.f });
        points_.push_back({ 100.f, -100.f, 0.f });
        points_.push_back({ 0.f, 100.f, 0.f });
        triangles.push_back({ num_pts, num_pts + 1, num_pts + 2 });
        neighbors.push_back({ -1, -1, -1 });


        // A stack is used to track triangles that need to be checked
        std::stack<int> tri_stack{};

        // Add each point one at a time fixing any triangles that violate the delaunay condition
        for (int p = 0; p < num_pts; ++p)
        {
            // First determine which triangle the new point is inside
            int enclosing_tri_idx{ find_enclosing_triangle(p) };

            std::array<int, 3> enclosing_tri{ triangles_[enclosing_tri_idx] };
            std::array<int, 3> enclosing_adj{ neighbors_[enclosing_tri_idx] };

            // Delete the enclosing triangle and create 3 new triangles between
            // the enclosing vertices and the new vertex p (Always make p the first vertex)
            // replace enclosing triangle with the first new one and add the other two to the vector
            // Keep vertex indices ordered counter-clockwise for each triangle 
            triangles_[enclosing_tri_idx] = { p, enclosing_tri[0], enclosing_tri[1] };
            triangles_.push_back({ p, enclosing_tri[1], enclosing_tri[2] });
            triangles_.push_back({ p, enclosing_tri[2], enclosing_tri[0] });

            // Indices for the new triangles
            int tri_0{ enclosing_tri_idx };
            int tri_1{ static_cast<int>(triangles.size()) - 2 };
            int tri_2{ static_cast<int>(triangles.size()) - 1 };

            // The triangles adjacent to the original triangle become the opposite adjacent neighbor
            // to the new triangles i.e. they share the edge that does not include the new point
            int opp_adj_0 = enclosing_adj[0];;
            int opp_adj_1 = enclosing_adj[1];
            int opp_adj_2 = enclosing_adj[2];

            // When adding to the adjacency list make the opposite adjacent triangle to p
            // the middle adjacency entry so we know when popping a triangle t from the stack
            // that the opposite adjacent edge to point triangles[t][0] is adjacency[t][1] (not 2)

            // The statement "In general for element I in the stack, the opposite adjacent triangle
            // is given by E(2, I)" Confused me since fortran starts arrays at 1 not 0 and the paper
            // didn't specify a relative position i.e. middle vs last but it turns out to be middle
            // so actually we need E(1, I) to be opposite adjacent of p not E(2, I)

            // Update existing adjanceny entry
            neighbors_[tri_0][0] = tri_2;
            neighbors_[tri_0][1] = opp_adj_0;
            neighbors_[tri_0][2] = tri_1;

            //adjacency[tri_1][0] = tri_0;
            //adjacency[tri_1][1] = opp_adj_1;
            //adjacency[tri_1][2] = tri_2;

            //adjacency[tri_2][0] = tri_1;
            //adjacency[tri_2][1] = opp_adj_2;
            //adjacency[tri_2][2] = tri_0;

            // Add adjacency entries for the two new triangles
            neighbors_.push_back({ tri_0, opp_adj_1, tri_2 });
            neighbors_.push_back({ tri_1, opp_adj_2, tri_0 });


            // Place the new triangles containing p in the stack as long as the edges opposite
            // p have a neighboring triangle (i.e. is not on a boundary t = -1)
            // While doing this update the adjacency list for the enclosing triangles neighbors
            // as long as they exist (!= -1) The first one enclosing_adj[0] does not need to be 
            // updated since it already points to tri_0 since it was reused but it wont hurt

            if (opp_adj_0 != -1)
            {
                tri_stack.push(tri_0);
            }
            if (opp_adj_1 != -1)
            {
                // Update opposite adjancent 1's neighbor entry that used to point
                // to enclosing triangle to now point toward triangle 1
                update_adjacent(opp_adj_1, enclosing_tri_idx, tri_1);

                tri_stack.push(tri_1);
            }
            if (opp_adj_2 != -1)
            {
                // Update opposite adjancent 2's neighbor entry that used to point
                // to enclosing triangle to now point toward triangle 2
                update_adjacent(opp_adj_2, enclosing_tri_idx, tri_2);

                tri_stack.push(tri_2);
            }


            // Check Delaunay condition and swap as needed propagating via the stack
            while (!tri_stack.empty())
            {
                int tri_l = tri_stack.top();
                tri_stack.pop();

                // the point that was added when tri_l was formed
                int point_p = triangles_[tri_l][0];

                // The triangle opposite adjacent to the point p
                int tri_r = neighbors_[tri_l][1];

                // Check if point p is inside the circumcircle of triangle r
                if (check_delaunay(tri_l, tri_r))
                {
                    // Swap the diagonal edge by updating the points of l and r
                    // then update the adjancies of the effected neighbors
                    swap_triangles(tri_l, tri_r);

                    // There are now potentially two triangles adjacent to l and r (A, B)
                    // that are opposite p. Place these on the stack if they exist
                    if (neighbors_[tri_l][1] != -1)
                    {
                        tri_stack.push(neighbors_[tri_l][1]);
                    }
                    if (neighbors_[tri_r][1] != -1)
                    {
                        tri_stack.push(neighbors_[tri_r][1]);
                    }
                }
            }
        }
    }

    void DelaunayGenerator::normalize_points()
    {
        // Normalize the coordinates of all of the points between 0 and 1

        // Determine the min and max values for x, y, and z
        float xmax{ points_[0].x };
        float ymax{ points_[0].y };
        float zmax{ points_[0].z };

        float xmin{ points_[0].x };
        float ymin{ points_[0].y };
        float zmin{ points_[0].z };

        for (auto point : points_)
        {
            if (point.x > xmax)
            {
                xmax = point.x;
            }
            else if (point.x < xmin)
            {
                xmin = point.x;
            }

            if (point.y > ymax)
            {
                ymax = point.y;
            }
            else if (point.y < ymin)
            {
                ymin = point.y;
            }

            if (point.z > zmax)
            {
                zmax = point.z;
            }
            else if (point.z < zmin)
            {
                zmin = point.z;
            }
        }

        // find the widest span between max and min between x, y, and z
        float xdelta = xmax - xmin;
        float ydelta = ymax - ymin;
        float zdelta = zmax - zmin;
        float dmax = std::max(xdelta, std::max(ydelta, zdelta));

        // shift every point coordinate to be positive and scale by max span
        // to get coordinates ranging from 0 to 1
        for (auto& point : points_)
        {
            point.x = (point.x - xmin) / dmax;
            point.y = (point.y - ymin) / dmax;
            point.z = (point.z - zmin) / dmax;
        }
    }

    void DelaunayGenerator::sort_points()
    {

    }

    int DelaunayGenerator::find_enclosing_triangle(int p)
    {
        int result{ -1 };

        // Naive solution check every triangle
        // this can be greatly improved once sorting is implemented
        for (int t = 0; t < triangles_.size(); ++t)
        {
            // Check if triangle t is enclosing point p by checking
            // if the point falls on or to the left of each edge
            bool enclosing{ true };
            for (int e = 0; e < 3; ++e)
            {
                // end point of the edge with start point e
                int e2{ (e + 1) % 3 };

                Point3D v1{ points_[triangles_[t][e]] }; // point where the edge starts
                Point3D v2{ points_[triangles_[t][e2]] }; // point where the edge stops
                Point3D vp{ points_[p] }; // search point

                // Subtract edge start to get vectors from start to end and from start to point
                v2.x -= v1.x;
                v2.y -= v1.y;
                v2.z -= v1.z;

                vp.x -= v1.x;
                vp.y -= v1.y;
                vp.z -= v1.z;

                // Take the cross product of these two vectors
                Point3D cross_product{
                    v2.y * vp.z - v2.z * vp.y,
                    v2.z * vp.x - v2.x * vp.z,
                    v2.x * vp.y - v2.y * vp.x
                };

                // For 2D the edge normal is always out of plane
                Point3D normal{ 0.f, 0.f, 1.f };

                // Take the dot product between the cross product result and the edge normal vector
                // This can be used to determine if cross product is in the same direction as the normal
                float dot_product{
                    cross_product.x * normal.x +
                    cross_product.y * normal.y +
                    cross_product.z * normal.z
                };

                // If the dot product is zero or positive the point is to the left of the edge
                // because the cross product edge vector x point vector points in the same direction
                // as the normal. If you where to use right hand rule on the two vectors your
                // thumb would point upwards as expected
                if (dot_product < 0.f)
                {
                    enclosing = false;
                    break;
                }
            }

            if (enclosing)
            {
                result = t;
                break;
            }
        }

        return result;
    }

    void DelaunayGenerator::update_adjacent(int target, int old_neighbor, int new_neighbor)
    {
        // Update the adjacency entry such that the entry pointing
        // to old_neighbor now points to new_neighbor

        for (int index = 0; index < 3; ++index)
        {
            if (neighbors_[target][index] == old_neighbor)
            {
                neighbors_[target][index] = new_neighbor;
                return;
            }
        }

        std::cerr << "Triangulation Error: update_adjacent failed to find a match for old_neighbor" << std::endl;
    }

    bool DelaunayGenerator::check_delaunay(int tri_l, int tri_r)
    {
        // Determine points p, v1, v2, and v3 of the quadrilateral
        int p{ triangles_[tri_l][0] };
        int v1{ triangles_[tri_l][2] };
        int v2{ triangles_[tri_l][1] };

        // Find the point v3 in triangle R that is not shared with triangle L
        // it might be the first point in R, but it also might not be
        int v3 = { -1 };
        for (int v3_idx = 0; v3_idx < 3; ++v3_idx)
        {
            if (triangles_[tri_r][v3_idx] != v1 && triangles_[tri_r][v3_idx] != v2)
            {
                v3 = triangles_[tri_r][v3_idx];
                break;
            }
        }

        // Intermediate subtraction results
        float x13{ points_[v1].x - points_[v3].x };
        float x23{ points_[v2].x - points_[v3].x };
        float x1p{ points_[v1].x - points_[p].x };
        float x2p{ points_[v2].x - points_[p].x };

        float y13{ points_[v1].y - points_[v3].y };
        float y23{ points_[v2].y - points_[v3].y };
        float y1p{ points_[v1].y - points_[p].y };
        float y2p{ points_[v2].y - points_[p].y };

        float cos_a = x13 * x23 + y13 * y23;
        float cos_b = x2p * x1p + y1p * y2p;
        float sin_a = x13 * y23 - y13 * x23;
        float sin_b = x2p * y1p - x1p * y2p;
        float sin_ab = sin_a * cos_b + sin_b * cos_a;

        bool swap{ false };

        // Main condition to check, but suffers from round off error near 0
        bool sin_ab_neg = sin_ab < 0.f;

        // a + b == pi (cos_a == 0 && cos_b == 0) is neutral meaning the triangulation
        // is correct regardless of swap

        // Test if cos_a and cos_b are both negative which would prove a + b > pi to be true
        // and covers the case where a and b are both near pi which the sin_ab check does not catch
        bool both_cos_neg = cos_a < 0 && cos_b < 0.f;

        // If cos_a and cos_b are both positive then a + b <= pi
        // and therefore a + b > pi can't be true so there is no need to swap
        // otherwise need to check sin_ab to make final determination
        // if true it also ensures a and b are not both near zero
        bool one_cos_neg = cos_a < 0.f || cos_b < 0.f;

        if (both_cos_neg || (one_cos_neg && sin_ab_neg))
        {
            swap = true;
        }

        return swap;
    }

    void DelaunayGenerator::swap_triangles(int tri_l, int tri_r)
    {
        std::array<int, 3> tri_l_pts{ triangles_[tri_l] };
        std::array<int, 3> tri_r_pts{ triangles_[tri_r] };
        std::array<int, 3> r_neighbors = neighbors_[tri_r];

        // Update triangles

        // Determine points p, v1, v2, and v3 of the quadrilateral
        int p{ tri_l_pts[0] };
        int v1{ tri_l_pts[2] };
        int v2{ tri_l_pts[1] };

        // Find the point v3 in triangle R that is not shared with triangle L
        int v3 = { -1 };
        int v3_idx{ 0 };

        for (; v3_idx < 3; ++v3_idx)
        {
            if (tri_r_pts[v3_idx] != v1 && tri_r_pts[v3_idx] != v2)
            {
                v3 = tri_r_pts[v3_idx];
                break;
            }
        }

        triangles_[tri_l] = { p, v2, v3 };
        triangles_[tri_r] = { p, v3, v1 };

        // Update neighbors

        // Determine the neighbors A, B, and C of the quadrilateral
        int n_a{ -1 };
        int n_b{ -1 };
        int n_c{ neighbors_[tri_l][2] }; // always the last neighbor of L

        // Determine neighbors A and B of triangle R based on the position
        // of v3 in triangle R's points array

        // neighbor B has the same position in neighbors array as point v3 in points array
        n_b = neighbors_[tri_r][v3_idx];

        // neighbor A is shifted two places from neighbor B
        n_a = neighbors_[tri_r][(v3_idx + 2) % 3];

        // Triangle L's first neighbor does not change
        neighbors_[tri_l][1] = n_a;
        neighbors_[tri_l][2] = tri_r;
        neighbors_[tri_r] = { tri_l, n_b, n_c };
    }
}