#pragma once

#include <vector>

#include "meshbuffer.h"
#include "shader.h"

struct GLFWwindow;

namespace moodysim
{
    bool hello_simulation();

    // Not to be confused with OpenGL Context
    class GLFWContext
    {
    public:

        GLFWContext();
        ~GLFWContext();

        // Cannot be copied or moved since there is no resource to be transfered
        // though could implement a reference tracking version
        GLFWContext(const GLFWContext&) = delete;
        GLFWContext& operator=(const GLFWContext&) = delete;
        // Move operations should not be implicitly declared with the copy operations deleted
    };


    class Window
    {
    public:

        Window();
        ~Window();

        // I don't think it is possible to "copy" a GLFWwindow object, but moving is fine
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Move operations should not be implicitly declared with the copy operations deleted
        Window(Window&& rhs) noexcept;
        Window& operator=(Window&& rhs) noexcept;

        bool should_close();

        void make_current();

        void swap_buffers();

    private:

        GLFWwindow* window_{};

        static bool is_glad_initialized;
    };


    class Application
    {
    public:

        void setup();

        void run();

        void add_mesh(SurfaceMeshData mesh_data);

    private:

        Window window_{};
        std::vector<MeshBuffer> meshes_{};
        ShaderProgram shader_{};
    };
}
