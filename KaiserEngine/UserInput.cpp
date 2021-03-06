#include "stdafx.h"
#include "UserInput.h"
#include "KaiserEngine.h"

// @see: https://docs.microsoft.com/en-us/windows/win32/learnwin32/mouse-clicks
LRESULT HandleMouseInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const int x = GET_X_LPARAM(lParam);
    const int y = GET_Y_LPARAM(lParam);
    const bool isKeyDown_ctrl = wParam & MK_CONTROL; // check if the CTRL key is down
    const bool isKeyDown_shift = wParam & MK_SHIFT; // check if the SHIFT key is down
    // If you need to find the state of other keys besides CTRL and SHIFT, use the GetKeyState function
    std::stringstream ss;
    ss << "Mouse ";

    std::string sMessage;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        sMessage = "LB down";
        break;
    case WM_LBUTTONUP:
        sMessage = "LB up";
        break;
    case WM_MBUTTONDOWN:
        sMessage = "MB down";
        break;
    case WM_MBUTTONUP:
        sMessage = "MB up";
        break;
    case WM_RBUTTONDOWN:
        sMessage = "RB down";
        break;
    case WM_RBUTTONUP:
        sMessage = "RB up";
        break;
    case WM_XBUTTONDOWN:
        sMessage = "XB down";
        break;
    case WM_XBUTTONUP:
        sMessage = "XB up";
        break;
    case WM_LBUTTONDBLCLK:
        sMessage = "LB double click";
        break;
    case WM_MBUTTONDBLCLK:
        sMessage = "MB double click";
        break;
    case WM_RBUTTONDBLCLK:
        sMessage = "RB double click";
        break;
    case WM_XBUTTONDBLCLK:
        sMessage = "XB double click";
        break;
    case WM_MOUSEMOVE:
        sMessage = "move";
        break;
    }

    ss << sMessage;

    ss << " @ (" << x << ", " << y << ")";
    ss << std::endl;
    OutputDebugStringA(ss.str().c_str());
    return 0;
}

LRESULT HandleKeyboardInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::stringstream ss;

    switch (message)
    {
    case WM_KEYDOWN:
    case WM_KEYUP:
        {
            // Virtual-key code
            // @see: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
            int vk = (int)(0xff & wParam);
            // Repeat code
            // The number of times the keystroke is autorepeated
            // as a result of the user holding down the key.
            // The repeat count is always 1 for a WM_KEYUP message
            int rc = (int)(0xffff & lParam);
            // Scan code
            // The value depends on the OEM
            int sc = (int)(0x7f & (lParam >> 16));
            // Extended key
            // Indicates whether the key is an extended key, such as the right-hand ALT
            // and CTRL keys that appear on an enhanced 101- or 102-key keyboard
            // The value is 1 if it is an extended key; otherwise, it is 0
            int ek = (int)(0x1 & (lParam >> 24));
            // Context code
            // The value is always 0 for a WM_KEYUP message
            // The value is always 0 for a WM_KEYDOWN message
            int cc = (int)(0x1 & (lParam >> 29));
            // Previous key state
            // The value is always 1 for a WM_KEYUP message
            // The value is 1 if the key is down before the message is sent, or it is zero if the key is up
            int ks = (int)(0x1 & (lParam >> 30));
            // Transition state
            // The value is always 1 for a WM_KEYUP message
            // The value is always 0 for a WM_KEYDOWN message
            int ts = (int)(0x1 & (lParam >> 31));

            char ch = 0;
            if (0x30 <= vk && vk <= 0x39)
            {
                ch = vk - 0x30 + '0';
            }
            else if (0x41 <= vk && vk <= 0x5A)
            {
                ch = vk - 0x41 + 'A';
            }

            ss << (ts ? "WM_KEYUP" : "WM_KEYDOWN");
            ss << " (vk=";
            if (ch)
                ss << "\"" << ch << "\"" << " ";
            ss << vk << ")";
            ss << std::endl;
            OutputDebugStringA(ss.str().c_str());
        }
        break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        {
            // Transition state
            // The value is always 1 for a WM_SYSKEYUP message
            // The value is always 0 for a WM_SYSKEYDOWN message
            int ts = (int)(0x1 & (lParam >> 31));

            ss << (ts ? "WM_SYSKEYUP" : "WM_SYSKEYDOWN");
            ss << std::endl;
            OutputDebugStringA(ss.str().c_str());
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
