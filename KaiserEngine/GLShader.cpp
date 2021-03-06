#include "stdafx.h"
#include "GLShader.h"
#include "loadgl.h"

static
const std::string
slurp(std::ifstream& ifs)
{
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

GLShader::
GLShader()
    : GLNamedObject()
{
}

GLShader::
GLShader(const std::string& path, const GLenum& type)
    : GLShader()
{
    switch (type)
    {
    case GL_VERTEX_SHADER:
    case GL_FRAGMENT_SHADER:
        break;
    default:
        throw std::invalid_argument("Invalid argument 'type'!");
    }

    const std::string text = slurp(std::ifstream{ path });
    if (GLuint shaderId = glCreateShader(type))
    {
        {
            std::stringstream ss;
            ss << "glCreateShader(" << type << ")";
            DetectGLError(ss);
        }
        {
            std::stringstream ss;
            ss << "Generated shader ID = " << shaderId
                << " "
                << std::boolalpha
                << static_cast<bool>(glIsShader(shaderId))
                << std::endl;
            OutputDebugStringA(ss.str().c_str());
        }
        const GLsizei count = 1;
        const GLchar* string = text.c_str();
        const GLint length = static_cast<GLint>(0xffffffff & text.length());
        glShaderSource(shaderId, count, &string, &length);
        {
            std::stringstream ss;
            ss << "glShaderSource("
                << shaderId << ", "
                << count << ", \"...\", "
                << length
                << ")";
            DetectGLError(ss);
        }
        glCompileShader(shaderId);
        {
            std::stringstream ss;
            ss << "glCompileShader("
                << shaderId
                << ")";
            DetectGLError(ss);
        }
        tp = type;
        id = shaderId;
    }
    else
    {
        throw std::runtime_error("Fail to create shader!");
    }
}

GLShader::
~GLShader()
{
    if (id)
    {
        glDeleteShader(id);

        {
            std::stringstream ss;
            ss << "Deleted shader ID = " << id
                << std::endl;
            OutputDebugStringA(ss.str().c_str());
        }
    }
    id = 0;
}

const bool
GLShader::
IsShader() const
{
    return id != 0 && glIsShader(id);
}

const GLenum
GLShader::
GetType() const
{
    return tp;
}
