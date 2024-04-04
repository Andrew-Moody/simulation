#include "mesh.h"

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
}