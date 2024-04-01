#include "staticmesh.h"

// glad must be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace graphics
{
    StaticMesh::StaticMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
        : vertices_(std::move(vertices)), indices_(std::move(indices))
    {
        // OpenGL resources are identified by id number

        unsigned int test{};

        glGenVertexArrays(1, &test);
        glBindVertexArray(test);

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
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);

        // Tell the gpu how the vertex buffer data is laid out (vbo is still bound) this can be done manually
        // each time the vbo is bound or use the vao to store this info and use it when the vao is bound
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        // Attribute layout for vertex color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Generate and bind an element buffer object (to store the list of indices that describe the triangles to draw)
        glGenBuffers(1, &element_buffer_id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_id_);

        // Copy index data to gpu memory
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);

        // vao is unbound here but remembers info about the vbo
        // bind the vao again before drawing
        glBindVertexArray(0);
    }

    // An OpenGL object id with value of zero signifies no object
    // Passing zero to glDeleteBuffers() does nothing so it makes a good value for "empty"
    StaticMesh::StaticMesh(StaticMesh&& rhs) noexcept
        : vertices_(std::move(rhs.vertices_)),
        indices_(std::move(rhs.indices_)),
        vertex_array_id_(std::exchange(rhs.vertex_array_id_, 0)),
        vertex_buffer_id_(std::exchange(rhs.vertex_buffer_id_, 0)),
        element_buffer_id_(std::exchange(rhs.element_buffer_id_, 0))
    {}

    StaticMesh& StaticMesh::operator=(StaticMesh&& rhs) noexcept
    {
        StaticMesh tmp{ std::move(rhs) };
        swap(*this, tmp);
        return *this;
    }


    StaticMesh::~StaticMesh()
    {
        glDeleteBuffers(1, &element_buffer_id_);
        glDeleteBuffers(1, &vertex_buffer_id_);
        glDeleteBuffers(1, &vertex_array_id_);
    }


    void StaticMesh::draw() const
    {
        // Bind the vertex array object (implicitly binds vertex buffer object)
        glBindVertexArray(vertex_array_id_);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_id_);
        glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    }
}
