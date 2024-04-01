#pragma once

#include <vector>
#include <utility>

namespace graphics
{
    struct Vertex
    {
        float x{}, y{}, z{}; // Position
        float r{}, g{}, b{}; // Vertex Color
    };

    class StaticMesh
    {
    public:

        StaticMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
        ~StaticMesh();

        StaticMesh(const StaticMesh& rhs) = delete;
        StaticMesh& operator=(const StaticMesh& rhs) = delete;

        StaticMesh(StaticMesh&& rhs) noexcept;
        StaticMesh& operator=(StaticMesh&& rhs) noexcept;

        void draw() const;

        friend void swap(StaticMesh& lhs, StaticMesh& rhs) noexcept
        {
            using std::swap;
            swap(lhs.vertices_, rhs.vertices_);
            swap(lhs.indices_, rhs.indices_);
            swap(lhs.vertex_array_id_, rhs.vertex_array_id_);
            swap(lhs.vertex_buffer_id_, rhs.vertex_buffer_id_);
            swap(lhs.element_buffer_id_, rhs.element_buffer_id_);
        }

    private:

        // GPU Resource Ids
        unsigned int vertex_array_id_{};
        unsigned int vertex_buffer_id_{};
        unsigned int element_buffer_id_{};

        // Mesh data
        std::vector<Vertex> vertices_{};
        std::vector<unsigned int> indices_{};
    };
}
