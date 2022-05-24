#include "Statusbar.h"

#include <winuser.h>

#include <commctrl.h>

void STATUSBAR_create(Statusbar *statusbar, HWND parent, HMENU id)
{
    statusbar->id = id;
    statusbar->control = CreateWindow(STATUSCLASSNAMEW, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, parent, (HMENU) statusbar->id, (HINSTANCE) GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);
}

void STATUSBAR_set_text(Statusbar *statusbar, const wchar_t *txt)
{
    SendMessage(statusbar->control, WM_SETTEXT, 0, (LPARAM) txt);
}

void STATUSBAR_on_resize(Statusbar *statusbar)
{
    SendMessage(statusbar->control, WM_SIZE, 0, 0);
}
