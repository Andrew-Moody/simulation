#include "shader.h"

#include <iostream>
#include <string_view>
#include <utility>

// glad must be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace moodysim
{
    Shader::Shader(const std::string_view source, ShaderType shader_type)
    {
        GLenum gl_shader_type{};

        switch (shader_type)
        {
        case ShaderType::VertexShader:
        {
            gl_shader_type = GL_VERTEX_SHADER;
            break;
        }
        case ShaderType::FragmentShader:
        {
            gl_shader_type = GL_FRAGMENT_SHADER;
            break;
        }
        default:
        {
            std::cerr << "Shader Error: invalid ShaderType" << std::endl;
            break;
        }
        }


        // get an id for the shader to be created
        shader_id_ = glCreateShader(gl_shader_type);

        // glShaderSource takes an array of char* with the option to pass an array
        // of lengths if the strings are not null terminated (which they may not be with string_view)
        constexpr size_t array_size{ 1 };
        const char* sources[array_size]{ source.data() };
        const int lengths[array_size]{ static_cast<int>(source.size()) };

        // Copies the contents of sources into the shader object so free to release after this returns
        // shader_id, source_array_size, source_array,
        glShaderSource(shader_id_, array_size, sources, lengths);

        // Compile Sources
        glCompileShader(shader_id_);

        // Check that the shader was successfully compiled
        int success{};
        glGetShaderiv(shader_id_, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            constexpr int buffsize{ 512 };
            char infoLog[buffsize];

            glGetShaderInfoLog(shader_id_, buffsize, nullptr, infoLog);

            std::cerr << "Shader compiling failed: " << infoLog << std::endl;
        }
    }

    Shader::~Shader()
    {
        glDeleteShader(shader_id_);
    }

    Shader::Shader(Shader&& rhs) noexcept : shader_id_(std::exchange(rhs.shader_id_, 0)) {}

    Shader& Shader::operator=(Shader&& rhs) noexcept
    {
        Shader tmp{ std::move(rhs) };
        std::swap(shader_id_, tmp.shader_id_);
        return *this;
    }


    ShaderProgram::ShaderProgram(const std::string_view vertex_source, const std::string_view fragment_source)
    {
        // The individual shaders are not needed after linking the shader program unless you want to reuse them
        Shader vertex_shader{ vertex_source, ShaderType::VertexShader };
        Shader fragment_shader{ fragment_source, ShaderType::FragmentShader };

        // A shader program is created by linking a vertex shader and fragment shader together
        shader_id_ = glCreateProgram();

        glAttachShader(shader_id_, vertex_shader.shader_id_);
        glAttachShader(shader_id_, fragment_shader.shader_id_);

        glLinkProgram(shader_id_);

        // Check that the shader program was successfully linked
        int success{};
        glGetProgramiv(shader_id_, GL_LINK_STATUS, &success);
        if (!success)
        {
            // If there is already a general OpenGL error shader info log won't give a value
            if (int error = glGetError())
            {
                std::cout << "GL_ERROR while linking shaders: " << std::hex << error << std::dec << std::endl;
            }
            else
            {
                constexpr int buffsize{ 512 };
                char infoLog[buffsize];
                glGetShaderInfoLog(shader_id_, buffsize, nullptr, infoLog);
                std::cerr << "Shader linking failed: " << infoLog << std::endl;
            }
        }
    }

    ShaderProgram::~ShaderProgram()
    {
        glDeleteShader(shader_id_);
    }

    ShaderProgram::ShaderProgram(ShaderProgram&& rhs) noexcept : shader_id_(std::exchange(rhs.shader_id_, 0)) {}

    ShaderProgram& ShaderProgram::operator=(ShaderProgram&& rhs) noexcept
    {
        ShaderProgram tmp{ std::move(rhs) };
        std::swap(shader_id_, tmp.shader_id_);
        return *this;
    }

    void ShaderProgram::bind()
    {
        glUseProgram(shader_id_);
    }
}