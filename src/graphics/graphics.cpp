#include "graphics.h"

#include <utility>
#include <iostream>

// glad must be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "meshbuffer.h"
#include "surfacemeshdata.h"
#include "mesh.h"

namespace moodysim
{
    bool hello_simulation()
    {
        GLFWContext glfw_context{};

        Application application{};

        application.setup();

        application.add_mesh(generate_sample_mesh());

        application.run();

        return true;
    }

    // Print a message for any error encountered by GLFW
    void glfw_error_callback(int error, const char* description)
    {
        std::cout << "GLFW ERROR: " << error << '\n' << description << std::endl;
    }

    GLFWContext::GLFWContext()
    {
        // Set a function to log errors during initialization (or in general)
        glfwSetErrorCallback(glfw_error_callback);

        // returns true immediatly if there is already a context running
        if (!glfwInit())
        {
            // Might want to throw here since theres really nothing to be done if glfwInit fails
            std::cout << "GLFW ERROR: Failed to initialize" << std::endl;
        }
    }

    GLFWContext::~GLFWContext()
    {
        // Does nothing if there is no context initialized yet
        glfwTerminate();
    }


    // Callback for when a window recieves a resize event
    void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
    {
        // Change the viewport size to the new size
        glViewport(0, 0, width, height);
    }

    bool Window::is_glad_initialized = false;

    Window::Window()
    {
        // Specify the version of OpenGL you wish to use (wsl ubuntu came with 4.2)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create a new window (also crates an OpenGL context)
        window_ = glfwCreateWindow(800, 600, "A GLFW Window", NULL, NULL);
        if (!window_)
        {
            // Consider throwing
            std::cout << "GLFW ERROR: Failed to create a window" << std::endl;
        }

        // Set the context owned by window as the current context to allow changes to be made
        glfwMakeContextCurrent(window_);

        // Set the callback for when a window resize event is recieved
        glfwSetFramebufferSizeCallback(window_, framebuffer_resize_callback);

        // How many frames to wait between buffer swaps (1 essentially acts as VSYNC)
        // setting to 0 removes the limit allowing the framerate to be uncapped
        // I believe the setting is kept for the current context so hopefully doesn't need
        // to be set again when the window is made current again later
        glfwSwapInterval(1);


        // Must be done once after having a window context but before any calls to OpenGL functions
        if (!is_glad_initialized)
        {
            // Use GLAD to load OpenGL function pointers
            // GLFW handles address location for specific operating system
            // must be called after a context is set
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                std::cout << "GLAD ERROR: Failed to load OpenGL functions" << std::endl;
            }

            is_glad_initialized = true;
        }

    }

    Window::~Window()
    {
        // Cleans up resources and context of the window
        glfwDestroyWindow(window_);
    }

    // So while the default move constructors would manage just fine to steal the moved from pointer
    // they don't define what value the moved from pointer is after so you might get a double delete
    Window::Window(Window&& rhs) noexcept : window_(std::exchange(rhs.window_, nullptr)) {}

    // You could just swap the pointers and if rhs is a temp the old window will get destroyed on return
    // but if it's not a temporary the old window won't get destroyed until the moved from object does
    Window& Window::operator=(Window&& rhs) noexcept
    {
        // move construct to steal the window from rhs and leave it empty
        Window temp{ std::move(rhs) };

        // Swap the pointers between window and temp (do not swap the Window objects themselves unless you implement
        // a swap function since std::swap is defined in terms of move and/or copy for class types making an infinite loop)
        std::swap(window_, temp.window_);

        return *this; // temp gets destroyed which destroys the old window
    }

    bool Window::should_close()
    {
        return glfwWindowShouldClose(window_);
    }

    void Window::make_current()
    {
        glfwMakeContextCurrent(window_);
    }

    void Window::swap_buffers()
    {
        glfwSwapBuffers(window_);
    }

    void Application::run()
    {
        // Process Input and Rendering commands until a close event is recieved
        while (!window_.should_close())
        {
            // Check input
            glfwPollEvents();
            //process_input(window);

            //camera_update(application.camera);
            //glUniformMatrix4fv(application.proj_matrix_id, 1, GL_FALSE, (float*)&(application.camera->view_transform));

            // Clear the screen for more drawing
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Draw model
            //model_draw(model, transform_id);

            for (const auto& mesh : meshes_)
            {
                mesh.draw();
            }

            // Present the new frame to the screen
            window_.swap_buffers();
        }
    }

    void Application::setup()
    {
        std::string vsource{ VCOLOR_VS_SRC };
        std::string fsource{ VCOLOR_FS_SRC };

        shader_ = ShaderProgram{ vsource, fsource };

        shader_.bind();

        //glEnable(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Default
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
    }


    void Application::add_mesh(SurfaceMeshData mesh_data)
    {
        MeshBuffer mesh_buffer{ mesh_data };

        meshes_.push_back(std::move(mesh_buffer));
    }
};
