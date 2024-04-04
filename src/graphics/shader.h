#pragma once

#include <string_view>

namespace moodysim
{
    // Basic vertex shader
    const char* const BASIC_VS_SRC = "#version 420 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() { gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0); }\0";

    // Basic fragment shader
    const char* const BASIC_FS_SRC = "#version 420 core\n"
        "void main() { FragColor = vec4(VertexColor, 1.0f); }\0";


    // Vertex color vertex shader
    const char* const VCOLOR_VS_SRC = "#version 420 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 Color;\n"
        "out vec3 VertexColor;\n"
        "uniform mat4 transform;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "VertexColor = Color;\n"
        "}\0";

    /* // Vertex color vertex shader
    const char* const VCOLOR_VS_SRC = "#version 420 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 Color;\n"
        "out vec3 VertexColor;\n"
        "uniform mat4 transform;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "gl_Position = projection * transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "VertexColor = Color;\n"
        "}\0"; */

        // Vertex color fragment shader
    const char* const VCOLOR_FS_SRC = "#version 420 core\n"
        "out vec4 FragColor;\n"
        "in vec3 VertexColor;\n"
        "void main() { FragColor = vec4(VertexColor, 1.0f); }\0";


    enum class ShaderType
    {
        VertexShader,
        FragmentShader
    };

    class ShaderProgram;

    class Shader
    {
        friend ShaderProgram;

    public:

        Shader() : shader_id_(0) {}
        Shader(const std::string_view source, ShaderType shader_type);
        ~Shader();

        Shader(const Shader& rhs) = delete;
        Shader& operator=(const Shader& rhs) = delete;

        Shader(Shader&& rhs) noexcept;
        Shader& operator=(Shader&& rhs) noexcept;

    private:

        unsigned int shader_id_{};
    };


    class ShaderProgram
    {
    public:

        ShaderProgram() : shader_id_(0) {}
        ShaderProgram(const std::string_view vertex_source, const std::string_view fragment_source);
        ~ShaderProgram();

        ShaderProgram(const ShaderProgram& rhs) = delete;
        ShaderProgram& operator=(const ShaderProgram& rhs) = delete;

        ShaderProgram(ShaderProgram&& rhs) noexcept;
        ShaderProgram& operator=(ShaderProgram&& rhs) noexcept;

        void bind();

    private:

        unsigned int shader_id_{};
    };
}
