#include "Textbox.h"

#include <winuser.h>

#include <commctrl.h>

static LRESULT CALLBACK window_procedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

void TEXTBOX_create(Textbox *textbox, HWND parent, HMENU id, int x, int y, int w, int h)
{
    textbox->id = id;
    textbox->control = CreateWindow(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, x, y, w, h, parent, id, NULL, NULL);
    textbox->callback_submit = NULL;

    SetWindowSubclass(textbox->control, window_procedure, 0, (DWORD_PTR) textbox);
}

void TEXTBOX_set_placeholder_text(Textbox *textbox, const wchar_t *txt)
{
    SendMessage(textbox->control, EM_SETCUEBANNER, FALSE, (LPARAM) txt);
}

void TEXTBOX_set_on_submit(Textbox *textbox, On_submit callback_on_submit)
{
    textbox->callback_submit = callback_on_submit;
}

void TEXTBOX_set_enabled(Textbox *textbox, bool enabled)
{
    EnableWindow(textbox->control, enabled);
}

void TEXTBOX_set_text_limit(Textbox *textbox, size_t length)
{
    SendMessage(textbox->control, EM_SETLIMITTEXT, length, 0);
}

const wchar_t *TEXTBOX_get_text(Textbox *textbox, wchar_t *buf, size_t buf_len)
{
    if (buf_len == 0) {
        return buf;
    }

    if (buf_len == 1) {
        buf[0] = '\0';
        return buf;
    }

    GetWindowTextW(textbox->control, buf, buf_len - 2);
    buf[buf_len - 1] = '\0';
    return buf;
}

LRESULT CALLBACK window_procedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    Textbox *textbox = (Textbox*) dwRefData;
    if (msg == WM_CHAR && wParam == VK_RETURN && textbox->callback_submit) {
        textbox->callback_submit(textbox);
        return 0;
    }
    return DefSubclassProc(hWnd, msg, wParam, lParam);
}
