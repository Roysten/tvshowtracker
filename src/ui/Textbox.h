#pragma once

#include <stdbool.h>

#include <windef.h>

typedef struct Textbox Textbox;
typedef void (*On_submit) (Textbox *textbox);

struct Textbox
{
    HWND control;
    HMENU id;
    On_submit callback_submit;
};

void TEXTBOX_create(Textbox *textbox, HWND parent, HMENU id, int x, int y, int w, int h);

void TEXTBOX_set_placeholder_text(Textbox *textbox, const wchar_t *txt);

void TEXTBOX_set_on_submit(Textbox *textbox, On_submit callback_on_submit);

void TEXTBOX_set_enabled(Textbox *textbox, bool enabled);

void TEXTBOX_set_text_limit(Textbox *textbox, size_t length);

const wchar_t *TEXTBOX_get_text(Textbox *textbox, wchar_t *buf, size_t buf_len);
