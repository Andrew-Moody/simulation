#pragma once

#include <utility>
#include <cstddef>

namespace moodysim
{
    class SurfaceMeshData;

    class MeshBuffer
    {
    public:

        MeshBuffer(const SurfaceMeshData& data);
        ~MeshBuffer();

        MeshBuffer(const MeshBuffer& rhs) = delete;
        MeshBuffer& operator=(const MeshBuffer& rhs) = delete;

        MeshBuffer(MeshBuffer&& rhs) noexcept;
        MeshBuffer& operator=(MeshBuffer&& rhs) noexcept;

        void draw() const;

        friend void swap(MeshBuffer& lhs, MeshBuffer& rhs) noexcept
        {
            using std::swap;
            swap(lhs.vertex_array_id_, rhs.vertex_array_id_);
            swap(lhs.vertex_buffer_id_, rhs.vertex_buffer_id_);
            swap(lhs.element_buffer_id_, rhs.element_buffer_id_);
        }

    private:

        // GPU Resource Ids
        unsigned int vertex_array_id_{};
        unsigned int vertex_buffer_id_{};
        unsigned int element_buffer_id_{};

        size_t vert_count_{};
        size_t idx_count_{};
    };
}
