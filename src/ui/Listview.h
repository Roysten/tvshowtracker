#pragma once

#include <stdbool.h>

#include <windef.h>

typedef struct Listview Listview;
struct Listview {
    HMENU id;
    HWND control;
};

void LISTVIEW_create(Listview *listview, HWND parent, HMENU id, int x, int y, int w, int h);

void LISTVIEW_add_column(Listview *listview, wchar_t *txt, int index);

void LISTVIEW_column_auto_width(Listview *listview, int col_index);

void LISTVIEW_column_auto_width_header(Listview *listview, int col_index);

void LISTVIEW_set_groups_enabled(Listview *listview, bool enabled);

void LISTVIEW_add_group(Listview *listview, wchar_t *group_name, int group_id);

void LISTVIEW_clear_items(Listview *listview);

void LISTVIEW_clear_groups(Listview *listview);

void LISTVIEW_set_item_count(Listview *listview, int item_count);

void LISTVIEW_set_enabled(Listview *listview, bool enabled);

void LISTVIEW_set_selection(Listview *listview, int index);

void LISTVIEW_set_collapsed(Listview *listview, int group_id, bool collapsed);

DWORD LISTVIEW_get_selected_index(Listview *listview);
