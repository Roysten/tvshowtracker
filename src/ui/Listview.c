#include "Listview.h"

#include <winuser.h>
#include <commctrl.h>

void LISTVIEW_create(Listview *listview, HWND parent, HMENU id, int x, int y, int w, int h)
{
	listview->id = id;
	listview->control = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL | LVS_ALIGNTOP | LVS_SHOWSELALWAYS, x, y, w, h, parent, id, NULL, NULL);
	ListView_SetExtendedListViewStyle(listview->control, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE);
}

void LISTVIEW_add_column(Listview *listview, wchar_t *txt, int index)
{
	LVCOLUMNW col = {0};
	col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
	col.pszText = txt;
	col.iSubItem = index;
	col.cx = 100;
	ListView_InsertColumn(listview->control, index, &col); 
}

void LISTVIEW_column_auto_width(Listview *listview, int col_index)
{
	ListView_SetColumnWidth(listview->control, col_index, LVSCW_AUTOSIZE);
}

void LISTVIEW_column_auto_width_header(Listview *listview, int col_index)
{
	ListView_SetColumnWidth(listview->control, col_index, LVSCW_AUTOSIZE_USEHEADER);
}

void LISTVIEW_set_groups_enabled(Listview *listview, bool enabled)
{
	ListView_EnableGroupView(listview->control, enabled);
}

void LISTVIEW_add_group(Listview *listview, wchar_t *group_name, int group_id)
{
	LVGROUP group = {0};
	group.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_STATE;
	group.cbSize = sizeof(group);
	group.pszHeader = group_name;
	group.iGroupId = group_id;
	group.state = LVGS_COLLAPSIBLE | LVGS_COLLAPSED;
	ListView_InsertGroup(listview->control, -1, &group);
}

void LISTVIEW_clear_items(Listview *listview)
{
	ListView_DeleteAllItems(listview->control);
}

void LISTVIEW_clear_groups(Listview *listview)
{
	ListView_RemoveAllGroups(listview->control);
}

void LISTVIEW_set_item_count(Listview *listview, int item_count)
{
	ListView_DeleteAllItems(listview->control);
	ListView_SetItemCount(listview->control, item_count);
	SendMessage(listview->control, WM_SETREDRAW, FALSE, 0);
	LVITEM item = {0};
	item.mask = LVIF_TEXT;
	item.pszText = LPSTR_TEXTCALLBACK;
	for (int i = 0; i < item_count; ++i) {
		ListView_InsertItem(listview->control, &item);
	}
	SendMessage(listview->control, WM_SETREDRAW, TRUE, 0);
}

void LISTVIEW_set_enabled(Listview *listview, bool enabled)
{
    EnableWindow(listview->control, enabled);
}

void LISTVIEW_set_selection(Listview *listview, int index)
{
	ListView_SetItemState(listview->control, index, LVIS_SELECTED, LVIS_SELECTED);
}

void LISTVIEW_set_collapsed(Listview *listview, int group_id, bool collapsed)
{
	ListView_SetGroupState(listview->control, group_id, LVGS_COLLAPSED, collapsed ? LVGS_COLLAPSED : 0);
}

DWORD LISTVIEW_get_selected_index(Listview *listview)
{
	return ListView_GetNextItem(listview->control, -1, LVNI_SELECTED);
}
