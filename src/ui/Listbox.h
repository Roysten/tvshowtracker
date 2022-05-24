#pragma once

#include <stdbool.h>

#include <windef.h>

typedef struct Listbox Listbox;

typedef void (*On_selection_changed) (Listbox *listbox, unsigned selection_index);

struct Listbox
{
    HWND control;
    HMENU id;
    On_selection_changed callback_selection_changed;
};


void LISTBOX_create(Listbox *listbox, HWND parent, HMENU id, int x, int y, int w, int h);

void LISTBOX_populate(Listbox *listbox, const wchar_t **strings, size_t string_count);

void LISTBOX_set_enabled(Listbox *listbox, bool enabled);

void LISTBOX_add_string(Listbox *listbox, const wchar_t *string);

void LISTBOX_reset_content(Listbox *listbox);

void LISTBOX_set_on_selection_changed(Listbox *listbox, On_selection_changed callback_selection_changed);

DWORD LISTBOX_get_selected_index(Listbox *listbox);
