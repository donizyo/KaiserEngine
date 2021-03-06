// stdafx.cpp : source file that includes just the standard includes
// $safeprojectname$.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

// @see: https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
void ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf = NULL;
    LPVOID lpDisplayBuf = NULL;
    DWORD dw = GetLastError();

    {
        std::stringstream ss;
        ss
            << "$ ErrorExit(lpszFunction=\"" << lpszFunction << "\") = " << dw
            << std::endl;
        OutputDebugStringA(ss.str().c_str());
    }

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));

#pragma warning(push)
#pragma warning(disable: 6067)
#pragma warning(disable: 28183)

    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error 0x%X: %s"),
        lpszFunction, dw, lpMsgBuf);

#pragma warning(pop)

    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

static HMODULE module = (HMODULE)NULL;

void * GetAnyGLFuncAddress(const char * name)
{
    void * p = (void *)wglGetProcAddress(name);

#ifdef _DEBUG
    {
        std::stringstream ss;
        ss << "OPENGL: " << name << " = " << p << std::endl;
        OutputDebugStringA(ss.str().c_str());
    }
#endif

    if (p == (void *)0
        || (p == (void *)0x1)
        || (p == (void *)0x2)
        || (p == (void *)0x3)
        || (p == (void *)-1))
    {
        if (!module)
            module = LoadLibraryA("opengl32.dll");

#pragma warning(push)
#pragma warning(disable: 6387)
        p = (void *)GetProcAddress(module, name);
#pragma warning(pop)
    }

    return p;
}

void CleanDll()
{
    if (module)
    {
        FreeLibrary(module);
        module = (HMODULE)NULL;
    }
}

static LPCSTR const codelist[] = {
    "GL_NO_ERROR",
    "GL_INVALID_ENUM",
    "GL_INVALID_VALUE",
    "GL_INVALID_OPERATION",
    "GL_STACK_OVERFLOW",
    "GL_STACK_UNDERFLOW",
    "GL_OUT_OF_MEMORY"
};

void DetectGLError(const char* function)
{
#if (APP_OPENGL_DEBUG_SEVERITY != APP_OPENGL_DEBUG_NONE)
    GLenum err = glGetError();

    std::stringstream ss;

#if (APP_OPENGL_DEBUG_SEVERITY == APP_OPENGL_DEBUG_ALL)
    if (err == GL_NO_ERROR)
    {
        ss << "[GL   LOG> ";
    }
    else
    {
        ss << "[GL ERROR> ";
    }
#elif (APP_OPENGL_DEBUG_SEVERITY == APP_OPENGL_DEBUG_MINIMAL)
    if (err == GL_NO_ERROR)
        return;
    ss << "[GL ERROR> ";
#endif

    ss << function
        << ' ';
    if (err != GL_NO_ERROR)
    {
        switch (err)
        {
        case GL_INVALID_ENUM:
        case GL_INVALID_VALUE:
        case GL_INVALID_OPERATION:
        case GL_STACK_OVERFLOW:
        case GL_STACK_UNDERFLOW:
        case GL_OUT_OF_MEMORY:
            ss << codelist[err & 0xf];
            break;
        default:
            ss << "GL_UNKNOWN_ERROR";
            break;
        }
    }
    else
        ss << "GL_NO_ERROR";
    ss << ' '
        << "0x" << std::hex << err << std::dec
        << std::endl;

    OutputDebugStringA(ss.str().c_str());
#endif
}

void DetectGLError(const std::stringstream& ss)
{
    DetectGLError(ss.str().c_str());
}

void SuppressGLError()
{
    GLenum err = glGetError();
}
