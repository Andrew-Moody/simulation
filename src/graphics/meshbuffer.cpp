#include "meshbuffer.h"

#include <vector>
#include <utility>

// glad must be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "surfacemeshdata.h"

namespace moodysim
{
    MeshBuffer::MeshBuffer(const SurfaceMeshData& mesh_data)
    {
        vert_count_ = mesh_data.get_vertices().size();
        idx_count_ = mesh_data.get_indices().size();

        // OpenGL resources are identified by id number

        // Generate and bind a Vertex Array Object (VAO) to store attributes
        // While bound, Vertex Array Object stores information about the bound Vertex Buffer Object (VBO)
        // as calls are made to modify properties on the VBO
        glGenVertexArrays(1, &vertex_array_id_);
        glBindVertexArray(vertex_array_id_);

        // Generate and bind a vertex buffer object (to store actual vertex data)
        // any calls targeting GL_ARRAY_BUFFER will effect this vbo until it is unbound
        glGenBuffers(1, &vertex_buffer_id_);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id_);

        // Copy vertex data to gpu memory
        glBufferData(GL_ARRAY_BUFFER, vert_count_ * sizeof(SMVertex), mesh_data.get_vertices().data(), GL_STATIC_DRAW);

        // Tell the gpu how the vertex buffer data is laid out (vbo is still bound) this can be done manually
        // each time the vbo is bound or use the vao to store this info and use it when the vao is bound
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SMVertex), (void*)0);
        glEnableVertexAttribArray(0);

        // Attribute layout for vertex color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SMVertex), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Generate and bind an element buffer object (to store the list of indices that describe the triangles to draw)
        glGenBuffers(1, &element_buffer_id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_id_);

        // Copy index data to gpu memory
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_count_ * sizeof(unsigned int), mesh_data.get_indices().data(), GL_STATIC_DRAW);

        // vao is unbound here but remembers info about the vbo
        // bind the vao again before drawing
        glBindVertexArray(0);
    }

    // An OpenGL object id with value of zero signifies no object
    // Passing zero to glDeleteBuffers() does nothing so it makes a good value for "empty"
    MeshBuffer::MeshBuffer(MeshBuffer&& rhs) noexcept
        : vertex_array_id_(std::exchange(rhs.vertex_array_id_, 0)),
        vertex_buffer_id_(std::exchange(rhs.vertex_buffer_id_, 0)),
        element_buffer_id_(std::exchange(rhs.element_buffer_id_, 0)),
        vert_count_(std::exchange(rhs.vert_count_, 0)),
        idx_count_(std::exchange(rhs.idx_count_, 0))
    {}

    MeshBuffer& MeshBuffer::operator=(MeshBuffer&& rhs) noexcept
    {
        MeshBuffer tmp{ std::move(rhs) };
        swap(*this, tmp);
        return *this;
    }


    MeshBuffer::~MeshBuffer()
    {
        glDeleteBuffers(1, &element_buffer_id_);
        glDeleteBuffers(1, &vertex_buffer_id_);
        glDeleteBuffers(1, &vertex_array_id_);
    }


    void MeshBuffer::draw() const
    {
        // Bind the vertex array object (implicitly binds vertex buffer object)
        glBindVertexArray(vertex_array_id_);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_id_);
        glDrawElements(GL_TRIANGLES, idx_count_, GL_UNSIGNED_INT, 0);
    }
}
