#pragma once

#include <windef.h>

typedef struct Statusbar Statusbar;

struct Statusbar
{
    HWND control;
    HMENU id;
};

void STATUSBAR_create(Statusbar *statusbar, HWND parent, HMENU id);

void STATUSBAR_set_text(Statusbar *statusbar, const wchar_t *txt);

void STATUSBAR_on_resize(Statusbar *statusbar);
