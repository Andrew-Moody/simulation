#pragma once

#include <vector>

namespace moodysim
{
    class SurfaceMeshData;

    struct Point3D
    {
        float x{}, y{}, z{};
    };

    SurfaceMeshData generate_sample_mesh();

    SurfaceMeshData generate_delaunay_mesh(const std::vector<Point3D>& points);
}