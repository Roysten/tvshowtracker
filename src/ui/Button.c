#include "Button.h"

#include <winuser.h>

void BUTTON_create(Button *button, HWND parent, HMENU id, const wchar_t *button_text, int x, int y, int w, int h)
{
    button->id = id;
    button->control = CreateWindow(L"BUTTON", button_text, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, (HMENU) button->id, NULL, NULL);
}

void BUTTON_set_enabled(Button *button, bool enabled)
{
    EnableWindow(button->control, enabled);
}
