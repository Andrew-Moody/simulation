#pragma once

#include <vector>
#include <array>

namespace moodysim
{

    struct Point3D
    {
        float x{}, y{}, z{};
    };

    struct Edge
    {
        int n1{}, n2{};
    };

    class SurfaceMeshData;

    SurfaceMeshData generate_sample_mesh();

    std::vector<Point3D> generate_sample_points(float radius, int density);


    // This class is not intended to be a public interface
    // it exposes more inner functionality for testing but is intended
    // to be wrapped in a more restricted class
    class DelaunayGenerator
    {
    public:

        DelaunayGenerator(std::vector<Point3D> points, std::vector<Edge> edges)
            : points_(std::move(points)), edges_(std::move(edges))
        {}

        // Allow for state injection for testing purposes
        DelaunayGenerator(
            std::vector<Point3D> points,
            std::vector<int> point_ordering,
            std::vector<Edge> edges,
            std::vector<std::array<int, 3>> triangles,
            std::vector<std::array<int, 3>> neighbors
        )
            : points_(std::move(points)), point_ordering_(std::move(point_ordering)),
            edges_(std::move(edges)), triangles_(std::move(triangles)), neighbors_(std::move(neighbors))
        {}

        SurfaceMeshData generate_delaunay_mesh();

        void triangulate();

        void apply_constraint();

        void normalize_points();

        // Optionally sort into bins to improve efficiency
        void sort_points();

        // Find the triangle that encloses the point p
        int find_enclosing_triangle(int p);

        // Update the adjacency entry such that the entry pointing
        // to old_neighbor now points to new_neighbor
        void update_adjacent(int target, int old_neighbor, int new_neighbor);

        // Check if the diagonal of a quad needs to be swapped (Delaunay condition)
        bool check_delaunay(int tri_l, int tri_r);

        // Swap the diagonal of a quad
        void swap_triangles(int tri_l, int tri_r);

        // Swap the position of two triangles in the triangles list and update neighbors 
        void swap_triangle_positions(int tri_a, int tri_b);

        // Remove the last triangle from triangles list and remove references from neighbors
        void pop_triangle();


        // Check if the point p lies "to the right" of edge e1-e2
        bool check_outward(Point3D p, Point3D e1, Point3D e2);

        // Check if the line segments a1-a2 and b1-b2 intersect
        bool check_intersection(Point3D a1, Point3D a2, Point3D b1, Point3D b2);

        // Check if two triangles form a convex quadrilateral
        bool check_convex(int t1, int t2);

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

        std::vector<Edge> edges_{};

        // Each triangle is defined by three indices into the points vector
        std::vector<std::array<int, 3>> triangles_;

        // Each triangle has up to three neighbors that share an edge
        // each entry is an index into the triangle vector (-1 denotes no neighbor)
        std::vector<std::array<int, 3>> neighbors_;

    };


    Point3D subtract(Point3D a, Point3D b)
    {
        return Point3D{
            (a.x - b.x),
            (a.y - b.y),
            (a.z - b.z)
        };
    }

    float dot_product(Point3D a, Point3D b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Point3D cross_product(Point3D a, Point3D b)
    {
        return Point3D{
            (a.y * b.z - a.z * b.y),
            (a.z * b.x - a.x * b.z),
            (a.x * b.y - a.y * b.x)
        };
    }
}