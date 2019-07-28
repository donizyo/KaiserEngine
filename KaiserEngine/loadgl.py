#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

HERE = os.path.abspath(os.path.dirname(__file__))

def GenerateCpp():
    """
    Generate loadgl.h and loadgl.cpp
    """

    buffer_cpp = []
    buffer_init = []
    buffer_h = []

    with open(os.path.join(HERE, "loadgl.txt"),
              mode="r", encoding="utf-8") as file:
        lines = file.read().splitlines()
        lines = list(dict.fromkeys(lines))
        for line in lines:
            if not line:
                continue
            func = line
            gltype = "PFN{}PROC".format(line.upper())

            # cpp
            line = """
    {1} = ({0}) GetAnyGLFuncAddress("{1}");
    if ({1} == NULL) ErrorExit(L"GetAnyGLFuncAddress(\\"{1}\\")");""".format(gltype, func)
            buffer_cpp.append(line)

            # init
            line = """{0} {1} = ({0}) NULL;
""".format(gltype, func)
            buffer_init.append(line)

            # h
            line = """extern {0} {1};
""".format(gltype, func)
            buffer_h.append(line)

    # Generate loadgl.h
    with open(os.path.join(HERE, "loadgl.h"),
              mode="w", encoding="utf-8") as file:
        file.write("""// Generated by loadgl.py
#pragma once

void LoadOpenglFunctions();
void CleanDll();

#include "stdafx.h"

""")

        for line in buffer_h:
            file.write(line)

    # Generate loadgl.cpp
    with open(os.path.join(HERE, "loadgl.cpp"),
              mode="w", encoding="utf-8") as file:
        # include headers
        file.write("""// Generated by loadgl.py
#include "stdafx.h"
#include "loadgl.h"

""")
        for line in buffer_init:
            file.write(line)

        # misc function declaration
        file.write("""
void * GetAnyGLFuncAddress(const char * name);""")
        file.write("\n")
        file.write("""void ErrorExit(LPTSTR lpszFunction);""")

        # function LoadOpenglFunctions
        file.write("""

void LoadOpenglFunctions()
{""")

        for line in buffer_cpp:
            file.write(line)

        # misc function definition
        file.write("""
}

static HMODULE module = (HMODULE) NULL;

void * GetAnyGLFuncAddress(const char * name)
{
    void * p = (void *) wglGetProcAddress(name);
    if (p == (void *) 0
        || (p == (void *) 0x1)
        || (p == (void *) 0x2)
        || (p == (void *) 0x3)
        || (p == (void *) -1))
    {
        if (!module)
            module = LoadLibraryA("opengl32.dll");
        p = (void *) GetProcAddress(module, name);
    }

    return p;
}

void CleanDll()
{
    if (module)
    {
        FreeLibrary(module);
        module = (HMODULE) NULL;
    }
}
""")

        file.write("""
#include <strsafe.h>

// @see: https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
void ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

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
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}
""")

if __name__ == "__main__":
    # Download from OpenGL Registry

    # Modify header files

    GenerateCpp()
