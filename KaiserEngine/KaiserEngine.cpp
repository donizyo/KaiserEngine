// KaiserEngine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "KaiserEngine.h"
#include "UserInput.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd;                                      // current window
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
std::atomic_bool isWindowClosing(false);
#if APP_FULLSCREEN
std::atomic_bool isWindowActivated(false);
#endif

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// @see: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KAISERENGINE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
#if APP_FULLSCREEN
    SetCapture(hWnd);
#endif

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KAISERENGINE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

#ifdef _DEBUG
        // close window when key 'ESC' is pressed
        if (GetAsyncKeyState(VK_ESCAPE))
            PostMessage(hWnd, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
#endif
    }

#if APP_FULLSCREEN
    ReleaseCapture();
#endif

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    UINT style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
#if APP_USERINPUT_DBLCLKS
    style |= CS_DBLCLKS;
#endif

    wcex.style          = style;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KAISERENGINE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr; // MAKEINTRESOURCEW(IDC_KAISERENGINE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   int x;
   int y;
   int width;
   int height;

   RECT rectDisplay;
   const HWND hDisplay = GetDesktopWindow();
   GetWindowRect(hDisplay, &rectDisplay);

   // window style
   DWORD winStyle;
#if APP_FULLSCREEN
   x = 0;
   y = 0;
   width = rectDisplay.right;
   height = rectDisplay.bottom;

   winStyle = WS_POPUP;
#else
   // calcuate initial position (x, y)
   // center window
   width = 1280;
   height = 720;
   x = (rectDisplay.right - width) / 2;
   y = (rectDisplay.bottom - height) / 2;


   winStyle = WS_OVERLAPPEDWINDOW;
#if (!APP_RESIZABLE)
   winStyle ^= WS_THICKFRAME;
#endif
#endif
   winStyle |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

   hWnd = CreateWindow(szWindowClass, szTitle,
       winStyle,
       x, y, // initial position
       width, height, // initial size
       nullptr, // parent window
       nullptr, // menu
       hInstance, // instance handle
       nullptr); // additional application data

   if (!hWnd)
   {
      return FALSE;
   }


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
#if APP_FULLSCREEN
   isWindowActivated = true;
#endif

   return TRUE;
}

int InitPixelFormat(HDC hdc)
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

#include "loadgl.h"

int RecreateContext(HDC hdc)
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
    int format = wglChoosePixelFormatARB(hdc, attribList_pixel_format, (const FLOAT *) NULL, 1, &pixelFormat, &numFormats);
    if (format == 0)
    {
        ErrorExit(L"wglChoosePixelFormatARB");
        return -1;
    }

    if (SetPixelFormat(hdc, format, NULL))
    {
        ErrorExit(L"SetPixelFormat");
        return -1;
    }

    const int attribList_context[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4, // opengl major version
        WGL_CONTEXT_MINOR_VERSION_ARB, 6, // opengl minor version
        WGL_CONTEXT_PROFILE_MASK_ARB, (WGL_CONTEXT_CORE_PROFILE_BIT_ARB), // opengl profile
        WGL_CONTEXT_FLAGS_ARB, (WGL_CONTEXT_DEBUG_BIT_ARB), // debug context
        0, // End
    };
    HGLRC hglrc = wglCreateContextAttribsARB(hdc, (HGLRC) NULL, attribList_context);
    if (hglrc == NULL)
    {
        ErrorExit(L"wglCreateContextAttribsARB");
        return -1;
    }

    if (!wglMakeCurrent(hdc, hglrc))
    {
        ErrorExit(L"wglMakeCurrent");
        return -1;
    }

    return 0;
}

void gl_init(HWND hWnd);
void vao_init();
void vao_exit();
void vao_draw();

size_t tickCounter;

void updateWindowTitle()
{
#ifdef _DEBUG
    // change window title name
    std::wstringstream sNewTitle;
    sNewTitle << szTitle;
    sNewTitle << " (OpenGL version: " << (LPCSTR) glGetString(GL_VERSION);
    sNewTitle << ", Tick: " << tickCounter;
    sNewTitle << ")";
    SetWindowText(hWnd, sNewTitle.str().c_str());
#endif
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

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//  @see: https://www.khronos.org/opengl/wiki/Platform_specifics:_Windows#When_do_I_render_my_scene.3F
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            HDC hDC = GetDC(hWnd);
            if (hDC == NULL) return -1;

            if (InitPixelFormat(hDC) < 0) return -1;

            HGLRC hRC = wglCreateContext(hDC);
            if (hRC == NULL)
            {
                ErrorExit(L"wglCreateContext");
                return -1;
            }

            if (!wglMakeCurrent(hDC, hRC))
            {
                ErrorExit(L"wglMakeCurrent");
                return -1;
            }

            tickCounter = 0;
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
            updateWindowTitle();

            // Get WGL Extensions
            LoadOpenglFunctions();

            // Clean up dummy context
            if (!wglMakeCurrent(hDC, NULL))
            {
                ErrorExit(L"wglMakeCurrent");
                return -1;
            }
            if (!wglDeleteContext(hRC))
            {
                ErrorExit(L"wglDeleteContext");
                return -1;
            }

            // Recreate context
            if (RecreateContext(hDC) < 0) return -1;

            // Initialize OpenGL
            gl_init(hWnd);
            vao_init();
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
#if APP_FULLSCREEN
    case WM_ACTIVATEAPP:
    {
        // prevent CPU "hogging" when it is not necessary
        isWindowActivated = (BOOL)wParam;
    }
    break;
#endif
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        {
            vao_exit();
            CleanDll();
            HDC hDC = wglGetCurrentDC();
            ReleaseDC(hWnd, hDC);
            HGLRC hRC = wglGetCurrentContext();
            wglMakeCurrent(hDC, NULL);
            wglDeleteContext(hRC);

            PostQuitMessage(0);
        }
        break;
    case WM_TIMER:
        {
#if APP_FULLSCREEN
            if (!isWindowActivated)
                break;
#endif
            tickCounter++;
            vao_draw();
            updateWindowTitle();
        }
        break;
#if APP_RESIZABLE
    case WM_SIZE:
        {
            // window resize event detected
            const int width = LOWORD(lParam);
            const int height = HIWORD(lParam);
            OnResize(width, height);
        }
        break;
#endif
    case WM_CLOSE:
        {
            isWindowClosing = true;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
#if APP_USERINPUT
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
#if APP_USERINPUT_DBLCLKS
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
#endif
    case WM_MOUSEMOVE:
        return HandleMouseInput(hWnd, message, wParam, lParam);
#endif
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
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
    GetWindowRect(hWnd, &rect);
    const int screenWidth = rect.right - rect.left;
    if (screenWidth <= 0)
        throw 1;
    const int screenHeight = rect.bottom - rect.top;
    if (screenHeight <= 0)
        throw 1;

    // @see: http://falloutsoftware.com/tutorials/gl/gl2.htm
    glClearColor(.4f, .6f, .9f, .0f);
    glClearDepth(.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    kglViewport(screenWidth, screenHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    glBegin(GL_LINES);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(100.0f, 100.0f, 0.0f); // origin of the FIRST line
        glVertex3f(200.0f, 140.0f, 5.0f); // ending point of the FIRST line
        glVertex3f(120.0f, 170.0f, 10.0f); // origin of the SECOND line
        glVertex3f(240.0f, 120.0f, 5.0f); // ending point of the SECOND line
    glEnd();

    glBindVertexArray(vao[0]);
    //  glDrawArrays(GL_LINE_LOOP,0,8);                 // lines ... no indices
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);  // indices (choose just one line not both !!!)
    glBindVertexArray(0);

    SwapBuffers(hdc);
}
