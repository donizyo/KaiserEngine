#include "stdafx.h"
#include "Canvas.h"
#include "loadgl.h"

using namespace MatrixMath;
using mat4 = MatrixQ<float, 4>;

Canvas::
Canvas()
{
}

Canvas::
~Canvas()
{
    this->dispose();
}

void
Canvas::
setParent(MainWindow* window)
{
    parent = window;
}

const MainWindow*
Canvas::
getParent() const
{
    return parent;
}

// @see: https://www.scratchapixel.com/
// @see: http://www.songho.ca/opengl/gl_projectionmatrix.html
template <typename _Ty>
const MatrixQ<_Ty, 4>
MakePerspectiveProjectionMatrix(
    const _Ty& aspectRatio,
    const _Ty& fieldOfView,
    const _Ty& zFar,
    const _Ty& zNear)
{
    const _Ty yScale = 1 / std::tan(fieldOfView / 2);
    const _Ty xScale = yScale / aspectRatio;
    const _Ty negatvieFrustumLength = zNear - zFar;
    MatrixQ<_Ty, 4> mq4;
    mq4.setData(0, 0, xScale);
    mq4.setData(1, 1, yScale);
    mq4.setData(2, 2, (zFar + zNear) / negatvieFrustumLength);
    mq4.setData(2, 3, (2 * zFar * zNear) / negatvieFrustumLength);
    mq4.setData(3, 2, -1);

    return mq4;
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakePerspectiveProjectionMatrix(
    const _Ty& width, const _Ty& height,
    const _Ty& fieldOfView,
    const _Ty& zFar,
    const _Ty& zNear)
{
    const _Ty aspectRatio = width / height;
    return MakePerspectiveProjectionMatrix<_Ty>(aspectRatio, fieldOfView, zFar, zNear);
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeOrthographicProjectionMatrix(
    const _Ty& right,
    const _Ty& left,
    const _Ty& top,
    const _Ty& bottom,
    const _Ty& zFar,
    const _Ty& zNear)
{
    return MatrixQ<_Ty, 4>{
        2 / (right - left), 0, 0, 0,
        0, 2 / (top - bottom), 0, 0,
        0, 0, -2 / (zFar - zNear), 0,
        (right + left) / (left - right), (top + bottom) / (bottom - top), (zFar + zNear) / (zNear - zFar), 1,
    };
}

// simplified because:
// (1) right + left = 0
// (2) top + bottom = 0
template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeOrthographicProjectionMatrix(
    const _Ty& right,
    const _Ty& top,
    const _Ty& zFar,
    const _Ty& zNear)
{
    return MakeOrthographicProjectionMatrix(right, -right, top, -top, zFar, zNear);
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeViewMatrix()
{
    return IdentityMatrix<_Ty, 4>();
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeModelMatrix()
{
    return IdentityMatrix<_Ty, 4>();
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeTranslationMatrix(const _Ty& x, const _Ty& y, const _Ty& z)
{
    return MatrixQ<_Ty, 4>{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1,
    };
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeScalingMatrix(const _Ty& Sx, const _Ty& Sy, const _Ty& Sz)
{
    return MatrixQ<_Ty, 4>{
        Sx, 0, 0, 0,
        0, Sy, 0, 0,
        0, 0, Sz, 0,
        0, 0, 0, 1,
    };
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeRotationMatrix_x(const _Ty& Rx)
{
    const _Ty cos = std::cos(Rx);
    const _Ty sin = std::sin(Rx);
    return MatrixQ<_Ty, 4>{
        1, 0, 0, 0,
        0, cos, -sin, 0,
        0, sin, cos, 0,
        0, 0, 0, 1,
    };
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeRotationMatrix_y(const _Ty& Ry)
{
    const _Ty cos = std::cos(Ry);
    const _Ty sin = std::sin(Ry);
    return MatrixQ<_Ty, 4>{
        cos, 0, sin, 0,
        0, 1, 0, 0,
        -sin, 0, cos, 0,
        0, 0, 0, 1,
    };
}

template <typename _Ty>
const MatrixQ<_Ty, 4>
MakeRotationMatrix_z(const _Ty& Rz)
{
    const _Ty cos = std::cos(Rz);
    const _Ty sin = std::sin(Rz);
    return MatrixQ<_Ty, 4>{
        cos, -sin, 0, 0,
        sin, cos, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
}

template <typename _Ty>
const Vector<_Ty, 4>
Transform(const MatrixQ<_Ty, 4>& TranslationMatrix,
    const MatrixQ<_Ty, 4>& RotationMatrix,
    const MatrixQ<_Ty, 4>& ScaleMatrix,
    const Vector<_Ty, 4>& OriginalVector)
{
    const Vector<_Ty, 4> TransformedVector = TranslationMatrix * RotationMatrix * ScaleMatrix * OriginalVector;
    return TransformedVector;
}

static
void
CALLBACK
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::stringstream ss;
    ss << std::hex << "GL CALLBACK: "
        << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
        << " source = 0x" << source
        << ", type = 0x" << type
        << ", id = 0x" << id
        << ", severity = 0x" << severity
        << ", message = " << message
        << std::endl;
    OutputDebugStringA(ss.str().c_str());
}


void
Canvas::
setup()
{
    RECT rect = { 0 };
    const MainWindow* parent = this->getParent();
    if (parent == nullptr)
        throw std::exception("NullPointerException: parent(const MainWindow*) is nullptr!");
    HWND hWnd = parent->getWindowHandle();
    if (hWnd == nullptr)
        throw std::exception("NullPointerException: hWnd(HWND) is nullptr!");
    GetClientRect(hWnd, &rect);
    const LONG screenWidth = rect.right - rect.left;
    const LONG screenHeight = rect.bottom - rect.top;
    this->setup(screenWidth, screenHeight);
}

void
Canvas::
setup(const int& screenWidth, const int& screenHeight)
{
    static const int trimX = 10;
    static const int trimY = 10;

    if (screenWidth <= 0) throw std::exception("Invalid screen width!");
    if (screenHeight <= 0) throw std::exception("Invalid screen height!");

    const GLsizei width{ screenWidth - 2 * trimX };
    const GLsizei height{ screenHeight - 2 * trimY };
    glViewport(trimX, trimY, width, height);
    {
        std::stringstream ss;
        ss << "glViewport(" << trimX << ", "
            << trimY << ", "
            << width << ", "
            << height << ")";
        DetectGLError(ss.str().c_str());
    }

    // setup shaders
    std::map<GLenum, GLstring> shaders;
    shaders[GL_VERTEX_SHADER] = "vertex.shader";
    shaders[GL_FRAGMENT_SHADER] = "fragment.shader";

    std::vector<GLstring> attributes{
        "in_position",          // 0
        "in_color",             // 1
        "in_texture_coord",     // 2
    };

    std::map<GLstring, GLint> uniforms;
    uniforms["matrix_projection"] = 0;
    uniforms["matrix_view"] = 0;
    uniforms["matrix_model"] = 0;

    program.Setup(shaders, &attributes, &uniforms);

    // beware the following matrices are instances of MatrixQ<double, 4>

    // make matrices
#pragma warning(push)
#pragma warning(disable: 4244) // suppress the warning about the following implicit conversion from int to _Ty(float)
    const mat4 mp = MakePerspectiveProjectionMatrix<float>(screenWidth, screenHeight, degrees2radians(60.0f), 100.0f, 0.1f);
    const mat4 mv = MakeViewMatrix<float>();
    const mat4 mm = MakeModelMatrix<float>();
#pragma warning(pop)

    GLProgram::UseProgram(&program);

    program.LoadUniformMatrix(uniforms["matrix_projection"], mp);
    program.LoadUniformMatrix(uniforms["matrix_view"], mv);
    program.LoadUniformMatrix(uniforms["matrix_model"], mm);

    GLProgram::UseProgram();
}

// @see: http://falloutsoftware.com/tutorials/gl/gl2.htm
void
Canvas::
prepare()
{
    glEnable(GL_DEBUG_OUTPUT);
    DetectGLError("glEnable");
    glDebugMessageCallback(MessageCallback, nullptr);
    DetectGLError("glDebugMessageCallback");

    // Setup an XNA like background color
    glClearColor(0.4f, 0.6f, 0.9f, 0.0f);
    DetectGLError("glClearColor");

    this->setup();
}

void
Canvas::
clear() const
{
    glClear(GL_COLOR_BUFFER_BIT);
    DetectGLError("glClear");
}

void
Canvas::
render() const
{
    this->clear();

    // TODO
    //OutputDebugStringA("Rendering ... \r\n");

    // bind textures if any

    // bind VAOs if any

    // bind VBOs if any

    // draw vertices

    // put everything back to default
    GLProgram::UseProgram();

    SwapBuffers(wglGetCurrentDC());
}

void
Canvas::
dispose()
{

}
