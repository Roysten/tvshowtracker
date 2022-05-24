#include "Groupbox.h"

#include <winuser.h>

void GROUPBOX_create(Groupbox *groupbox, HWND parent, HMENU id, const wchar_t *txt, int x, int y, int w, int h)
{
    CreateWindow(L"BUTTON", txt, WS_GROUP | WS_CHILD | WS_VISIBLE | BS_GROUPBOX, x, y, w, h, parent, NULL, GetModuleHandle(0), NULL);
}
