#pragma once

#include "stdafx.h"
#include "Event.h"
#include "EventHandler.h"

class EventManager
    : public virtual EventHandler
{
public:
    EventManager();
    virtual ~EventManager();
    void Setup(HWND hWnd);
    void Dispose();
};

