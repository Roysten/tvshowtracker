#pragma once

#include <windows.h>

/* Register a class for our main window */
BOOL RegisterMainWindowClass(void);

/* Create an instance of our main window */
HWND CreateMainWindow(void);
