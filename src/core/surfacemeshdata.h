#pragma once

#include <vector>

namespace moodysim
{
    struct SMVertex
    {
        float x{}, y{}, z{}; // Position
        float r{}, g{}, b{}; // Vertex Color
    };

    class SurfaceMeshData
    {
    public:

        SurfaceMeshData(std::vector<SMVertex> vertices, std::vector<unsigned int> indices)
            : vertices_(std::move(vertices)), indices_(std::move(indices))
        {}

        const std::vector<SMVertex>& get_vertices() const { return vertices_; }
        const std::vector<unsigned int>& get_indices() const { return indices_; }

    private:

        std::vector<SMVertex> vertices_;
        std::vector<unsigned int> indices_;
    };
}
