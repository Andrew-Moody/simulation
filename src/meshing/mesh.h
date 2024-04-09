#pragma once

#include <vector>
#include <array>

namespace moodysim
{
    class SurfaceMeshData;

    SurfaceMeshData generate_sample_mesh();

    struct Point3D
    {
        float x{}, y{}, z{};
    };


    // This class is not intended to be a public interface
    // it exposes more inner functionality for testing but is intended
    // to be wrapped in a more restricted class
    class DelaunayGenerator
    {
    public:

        DelaunayGenerator(std::vector<Point3D> points)
            : points_(std::move(points))
        {}

        // Allow for state injection for testing purposes
        DelaunayGenerator(
            std::vector<Point3D> points,
            std::vector<int> point_ordering,
            std::vector<std::array<int, 3>> triangles,
            std::vector<std::array<int, 3>> neighbors
        )
            : points_(std::move(points)), point_ordering_(std::move(point_ordering)),
            triangles_(std::move(triangles)), neighbors_(std::move(neighbors))
        {}

        SurfaceMeshData generate_delaunay_mesh();

        void triangulate();

        void normalize_points();

        // Optionally sort into bins to improve efficiency
        void sort_points();

        int find_enclosing_triangle();

        // Update the adjacency entry such that the entry pointing
        // to old_neighbor now points to new_neighbor
        void update_adjacent(int target, int old_neighbor, int new_neighbor);

        // Check if the diagonal of a quad needs to be swapped (Delaunay condition)
        bool check_delaunay(int tri_l, int tri_r);

        // Swap the diagonal of a quad
        void swap_triangles(int tri_l, int tri_r);

        // Used by tests to check internal state
        const std::vector<Point3D>& get_points() const { return points_; }
        const std::vector<int>& get_point_ordering() const { return point_ordering_; }
        const std::vector<std::array<int, 3>>& get_triangles() const { return triangles_; }
        const std::vector<std::array<int, 3>>& get_neighbors() const { return neighbors_; }

    private:

        // The point cloud to triangulate
        // Must copy since they will get normalized and reordered
        std::vector<Point3D> points_{};

        // The location each point has been moved to as a result of sorting
        std::vector<int> point_ordering_{};

        // Each triangle is defined by three indices into the points vector
        std::vector<std::array<int, 3>> triangles_;

        // Each triangle has up to three neighbors that share an edge
        // each entry is an index into the triangle vector (-1 denotes no neighbor)
        std::vector<std::array<int, 3>> neighbors_;

    };
}