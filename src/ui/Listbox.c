#include "Listbox.h"

#include <winuser.h>

#include "../Logger.h"

void LISTBOX_create(Listbox *listbox, HWND parent, HMENU id, int x, int y, int w, int h)
{
    listbox->id = id;
    listbox->control = CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY | LBS_HASSTRINGS, x, y, w, h, parent, (HMENU) listbox->id, (HINSTANCE) GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);
    listbox->callback_selection_changed = NULL;
}

void LISTBOX_populate(Listbox *listbox, const wchar_t **strings, size_t string_count)
{
    LISTBOX_reset_content(listbox);
    for (size_t i = 0; i < string_count; ++i) {
        LISTBOX_add_string(listbox, strings[i]);
    }
}

void LISTBOX_add_string(Listbox *listbox, const wchar_t *string) {
    SendMessageW(listbox->control, LB_ADDSTRING, 0, (LPARAM) string);
}

void LISTBOX_reset_content(Listbox *listbox)
{
    SendMessageW(listbox->control, LB_RESETCONTENT, 0, 0);
}

void LISTBOX_set_enabled(Listbox *listbox, bool enabled)
{
    EnableWindow(listbox->control, enabled);
}

void LISTBOX_set_on_selection_changed(Listbox *listbox, On_selection_changed callback_selection_changed)
{
    listbox->callback_selection_changed = callback_selection_changed;
}

DWORD LISTBOX_get_selected_index(Listbox *listbox)
{
    return SendMessage(listbox->control, LB_GETCURSEL, 0, 0);
}

// INT_PTR CALLBACK ListBoxExampleProc(HWND hWnd, UINT message, 
//         WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//     case WM_INITDIALOG:
//         {
//             // Add items to list. 
//             HWND hwndList = GetDlgItem(hWnd, IDC_LISTBOX_EXAMPLE);  
//             for (int i = 0; i < ARRAYSIZE(shows); i++) 
//             { 
//                 int pos = (int)SendMessage(hwndList, LB_ADDSTRING, 0, 
//                     (LPARAM) shows[i].title); 
//                 // Set the array index of the player as item data.
//                 // This enables us to retrieve the item from the array
//                 // even after the items are sorted by the list box.
//                 SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM) i); 
//             } 
//             // Set input focus to the list box.
//             SetFocus(hwndList); 
//             return TRUE;               
//         } 

//     case WM_COMMAND:
//         switch (LOWORD(wParam))
//         {
//         case IDOK:
//         case IDCANCEL:
//             EndDialog(hWnd, LOWORD(wParam));
//             return TRUE;

//         case IDC_LISTBOX_EXAMPLE:
//             {
//                 switch (HIWORD(wParam)) 
//                 { 
//                 case LBN_SELCHANGE:
//                     {
//                         HWND hwndList = GetDlgItem(hWnd, IDC_LISTBOX_EXAMPLE); 

//                         // Get selected index.
//                         int lbItem = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0); 

//                         // Get item data.
//                         int i = (int)SendMessage(hwndList, LB_GETITEMDATA, lbItem, 0);

//                         // Do something with the data from Roster[i]
//                         TCHAR buff[MAX_PATH];
//                         StringCbPrintf (buff, ARRAYSIZE(buff),  
//                             TEXT("Position: %s\nGames played: %d\nGoals: %d"), 
//                             Roster[i].achPosition, Roster[i].nGamesPlayed, 
//                             Roster[i].nGoalsScored);

//                         SetDlgItemText(hWnd, IDC_STATISTICS, buff); 
//                         return TRUE; 
//                     } 
//                 }
//             }
//             return TRUE;
//         }
//     }
//     return FALSE;
// }
