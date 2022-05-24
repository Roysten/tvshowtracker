#pragma once

#include <windef.h>

typedef struct Groupbox Groupbox;

struct Groupbox
{
    HWND control;
    HMENU id;
};

void GROUPBOX_create(Groupbox *groupbox, HWND parent, HMENU id, const wchar_t *txt, int x, int y, int w, int h);
