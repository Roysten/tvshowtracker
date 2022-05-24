#pragma once

#include <stdbool.h>

#include <windef.h>

typedef struct Button Button;

// typedef void (*On_selection_changed) (Button *listbox, unsigned selection_index);

struct Button
{
    HWND control;
    HMENU id;
};

void BUTTON_create(Button *button, HWND parent, HMENU id, const wchar_t *button_text, int x, int y, int w, int h);

void BUTTON_set_enabled(Button *button, bool enabled);
