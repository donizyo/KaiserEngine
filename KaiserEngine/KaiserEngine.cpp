// KaiserEngine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "KaiserEngine.h"
#include "loadgl.h"
#include "UserInput.h"
#include "Event.h"
#include "EventManager.h"

#define MAX_LOADSTRING 100

void gl_init(HWND hWnd);
void vao_init();
void vao_exit();
void vao_draw();

class BaseWindow abstract
    : public AbstractWindow<BaseWindow>
    , public EventManager
{
public:
    BaseWindow()
    {
#ifdef _DEBUG
        {
            bool res = SetInputMethodEnabled(false);
            std::stringstream ss;
            ss << "SetInputMethodEnabled(false) -> ";
            ss << std::boolalpha << res << std::noboolalpha;
            ss << std::endl;
            OutputDebugStringA(ss.str().c_str());
        }
#endif
    }

    void AttachUserInput()
    {
        EnableWindow(getWindowHandle(), true);
    }

    void DetachUserInput()
    {
        EnableWindow(getWindowHandle(), false);
    }

    LRESULT CALLBACK HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        // @see: https://docs.microsoft.com/zh-cn/windows/win32/winmsg/about-messages-and-message-queues
        return TraverseList(hWnd, message, wParam, lParam);
    }

protected:
    virtual int InitPixelFormat(HDC hdc) = 0;
    virtual int OnCreate() = 0;

    int OnTimer()
    {
        return 0;
    }

    int OnClose()
    {
        HDC hDC = getDeviceContext();
        HGLRC hRC = getRenderContext();
        wglMakeCurrent(hDC, nullptr);
        setRenderContext(nullptr);
        if (hRC)
            wglDeleteContext(hRC);
        wglMakeCurrent(nullptr, nullptr);
        setDeviceContext(nullptr);
        if (hDC)
            ReleaseDC(hWnd, hDC);
        isWindowClosing = true;

        return 0;
    }
};

class MainWindow
    : public BaseWindow
{
public:
    MainWindow(HINSTANCE hInstance)
    {
        wchar_t szTitle[MAX_LOADSTRING];
        wchar_t szWindowClass[MAX_LOADSTRING];
        LoadStringW(hInstance, IDC_KAISERENGINE, szWindowClass, MAX_LOADSTRING);
        LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

        Create(hInstance, nullptr, szWindowClass, szTitle);
        Setup(getWindowHandle());
        AttachUserInput();
    }

    // RecreateContext(HDC hdc)
    int InitPixelFormat(HDC hdc) override
    {
        const int attribList_pixel_format[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0, // End
        };
        //const FLOAT pfAttribFList[] = { 0 };
        int pixelFormat;
        UINT numFormats;
        int format = wglChoosePixelFormatARB(hdc,
            attribList_pixel_format,
            (const FLOAT *)NULL,
            1,
            &pixelFormat,
            &numFormats);
        if (format == 0)
        {
            ErrorExit(L"wglChoosePixelFormatARB");
            return -1;
        }

        PIXELFORMATDESCRIPTOR pfd = { 0 };
        DescribePixelFormat(hdc, pixelFormat, sizeof(pfd), &pfd);

        // @see: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-setpixelformat
        if (!SetPixelFormat(hdc, format, &pfd))
        {
            ErrorExit(L"SetPixelFormat");
            return -1;
        }

        return 0;
    }

    int OnCreate() override
    {
        if (wglGetCurrentDC())
            return -1;

        HDC hDC = GetDC(hWnd);
        if (hDC == NULL) return -1;
        setDeviceContext(hDC);

        if (InitPixelFormat(hDC) < 0) return -1;

        const int attribList_context[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4, // opengl major version
            WGL_CONTEXT_MINOR_VERSION_ARB, 6, // opengl minor version
            WGL_CONTEXT_PROFILE_MASK_ARB, (WGL_CONTEXT_CORE_PROFILE_BIT_ARB), // opengl profile
            WGL_CONTEXT_FLAGS_ARB, (WGL_CONTEXT_DEBUG_BIT_ARB), // debug context
            0, // End
        };

        HGLRC hRC = wglCreateContextAttribsARB(hDC, (HGLRC)NULL, attribList_context);
        if (hRC == NULL)
        {
            ErrorExit(L"wglCreateContextAttribsARB");
            return -1;
        }
        setRenderContext(hRC);

        if (!wglMakeCurrent(hDC, hRC))
        {
            ErrorExit(L"wglMakeCurrent");
            return -1;
        }

        // install timer
        SetTimer(hWnd,
            0, // timer id
            20, // timeout value, in milliseconds;
                // this configuration setup fixs frame rate
                // 1000 MS = 1 FPS
                // 20 MS = 50 FPS
            NULL); // make system post WM_TIMER message
        // returned timer id should be 1,
        // if no timer has been created yet

        // Initialize OpenGL
        gl_init(hWnd);
        vao_init();

        return 0;
    }

    int OnTimer()
    {
#ifdef _DEBUG
        OutputDebugStringA("On drawing OpenGL ...");
#endif
        vao_draw();

        return 0;
    }

    int OnClose()
    {
        CleanDll();
        this->BaseWindow::OnClose();
        vao_exit();

        return 0;
    }
};

class DummyWindow
    : public BaseWindow
{
public:
    DummyWindow(HINSTANCE hInstance)
    {
        wchar_t szWindowClass[MAX_LOADSTRING];
        wchar_t szTitle[MAX_LOADSTRING];
        LoadStringW(hInstance, IDC_KAISERENGINE_OPENLOADER, szWindowClass, MAX_LOADSTRING);
        LoadStringW(hInstance, IDS_APP_TITLE_LOADING, szTitle, MAX_LOADSTRING);

        Create(hInstance, HWND_MESSAGE, szWindowClass, szTitle, 0, 0, 0, 0, 0, 0, SW_HIDE);
        DetachUserInput();
        OnCreate();
        OnClose();
    }

    int InitPixelFormat(HDC hdc) override
    {
        // @see: https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
        // Good pixel format to choose for the dummy context
        //  * 32-bit RGBA color buffer
        //  * 24-bit depth color buffer
        //  * 8-bit stencil
        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
            PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
            32,                   // Colordepth of the framebuffer.
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            24,                   // Number of bits for the depthbuffer
            8,                    // Number of bits for the stencilbuffer
            0,                    // Number of Aux buffers in the framebuffer.
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        int format = ChoosePixelFormat(hdc, &pfd);
        if (format == 0)
        {
            // could not find a pixel format that matches the description,
            // or the PDF was not filled out correctly
            ErrorExit(L"ChoosePixelFormat");
            return -1;
        }

        if (!SetPixelFormat(hdc, format, &pfd))
        {
            ErrorExit(L"SetPixelFormat");
            return -1;
        }

        return 0;
    }

    int OnCreate() override
    {
        if (wglGetCurrentDC())
            return -1;

        HDC hDC = GetDC(hWnd);
        if (hDC == NULL) return -1;
        setDeviceContext(hDC);

        if (InitPixelFormat(hDC) < 0) return -1;

        HGLRC hRC = wglCreateContext(hDC);
        if (hRC == NULL)
        {
            ErrorExit(L"wglCreateContext");
            return -1;
        }
        setRenderContext(hRC);

        if (!wglMakeCurrent(hDC, hRC))
        {
            ErrorExit(L"wglMakeCurrent");
            return -1;
        }

        // Get WGL Extensions
        LoadOpenglFunctions();

        // Clean up
        OnClose();

        return 0;
    }
};

WPARAM StartMessageLoop(HINSTANCE hInstance)
{
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KAISERENGINE));
    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
#if 0
        {
            std::stringstream ss;
            ss << "GetMessage from (" << static_cast<void*>(msg.hwnd) << ")!" << std::endl;
            OutputDebugStringA(ss.str().c_str());
        }
#endif
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}

// @see: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
#ifdef _DEBUG
    {
        std::wstringstream ss;
        ss
            << "--------------------------------------------" << std::endl
            << "CmdLine: \"" << lpCmdLine << "\"" << std::endl
            << "CmdShow: " << nCmdShow << std::endl
            << "--------------------------------------------" << std::endl
            << std::endl;
        OutputDebugString(ss.str().c_str());
    }
#endif

    DummyWindow dummy(hInstance);
    MainWindow win(hInstance);

#if APP_FULLSCREEN
    SetCapture(hWnd);
#endif

    int retCode;
    retCode = StartMessageLoop(hInstance);

#if APP_FULLSCREEN
    ReleaseCapture();
#endif

    return retCode;
}


RECT GetDisplayRect()
{
    RECT rectDisplay;
    GetWindowRect(GetDesktopWindow(), &rectDisplay);
    return rectDisplay;
}

void kglViewport(int width, int height)
{
    const int trimX = 10;
    const int trimY = 10;

    if (height <= 0) height = 1;

    glViewport(trimX, trimY, width - 2 * trimX, height - 2 * trimY);

    const double aspectRatio = double(width) / double(height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspectRatio, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
}

void OnResize(int width, int height)
{
    kglViewport(width, height);
}

GLuint vbo[4] = { -1,-1,-1,-1 };
GLuint vao[4] = { -1,-1,-1,-1 };
const GLfloat vao_pos[] =
{
    //  x    y    z
    -1.0,-1.0,-1.0,
    +1.0,-1.0,-1.0,
    +1.0,+1.0,-1.0,
    -1.0,+1.0,-1.0,
    -1.0,-1.0,+1.0,
    +1.0,-1.0,+1.0,
    +1.0,+1.0,+1.0,
    -1.0,+1.0,+1.0,
};
const GLfloat vao_col[] =
{
    //  r   g   b
    0.0,0.0,0.0,
    1.0,0.0,0.0,
    1.0,1.0,0.0,
    0.0,1.0,0.0,
    0.0,0.0,1.0,
    1.0,0.0,1.0,
    1.0,1.0,1.0,
    0.0,1.0,1.0,
};
const GLuint vao_ix[] =
{
    0,1,2,3,
    4,5,6,7,
    0,1,5,4,
    1,2,6,5,
    2,3,7,6,
    3,0,4,7,
};
void gl_init(HWND hWnd)
{
    RECT rect = { 0 };
    GetClientRect(hWnd, &rect);
    const int screenWidth = rect.right - rect.left;
    if (screenWidth <= 0)
        throw 1;
    const int screenHeight = rect.bottom - rect.top;
    if (screenHeight <= 0)
        throw 1;

    // @see: http://falloutsoftware.com/tutorials/gl/gl2.htm
    glClearColor(.4f, .6f, .9f, .0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    kglViewport(screenWidth, screenHeight);
}
void vao_init()
{
    GLuint i;
    glGenVertexArrays(4, vao);
    glGenBuffers(4, vbo);
    glBindVertexArray(vao[0]);
    i = 0; // vertex
    glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vao_pos), vao_pos, GL_STATIC_DRAW);
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 0, 0);
    i = 1; // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[i]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vao_ix), vao_ix, GL_STATIC_DRAW);
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, 4, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
    i = 2; // normal
    i = 3; // color
    glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vao_col), vao_col, GL_STATIC_DRAW);
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    //  glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}
//---------------------------------------------------------------------------
void vao_exit()
{
    glDeleteVertexArrays(4, vao);
    glDeleteBuffers(4, vbo);
}
//---------------------------------------------------------------------------
void vao_draw()
{
    const HDC hdc = wglGetCurrentDC();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao[0]);
    //  glDrawArrays(GL_LINE_LOOP,0,8);                 // lines ... no indices
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);  // indices (choose just one line not both !!!)
    glBindVertexArray(0);

    SwapBuffers(hdc);

    DetectGLError(9);
}
