#include "str_util.h"

#ifdef NOWIN

#include <string.h>
#include <wchar.h>
#include <stdlib.h>
const wchar_t *STR_UTIL_convert(const char* src)
{
    int required_buf_len = strlen(src);
    wchar_t *name_buf = calloc(required_buf_len, sizeof(*name_buf));
    return name_buf;
}

char *STR_UTIL_convert_utf8(const wchar_t *src)
{
    int required_buf_len = wcslen(src);
    char *name_buf = calloc(required_buf_len, sizeof(*name_buf));
    return name_buf;
}

#else

#include <stringapiset.h>

const wchar_t *STR_UTIL_convert(const char* src)
{
    int required_buf_len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    wchar_t *name_buf = calloc(required_buf_len, sizeof(*name_buf));
    MultiByteToWideChar(CP_UTF8, 0, src, -1, name_buf, required_buf_len);
    return name_buf;
}

char *STR_UTIL_convert_utf8(const wchar_t *src)
{
    int required_buf_len = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
    char *name_buf = calloc(required_buf_len, sizeof(*name_buf));
    WideCharToMultiByte(CP_UTF8, 0, src, -1, name_buf, required_buf_len, NULL, NULL);
    return name_buf;
}

#endif
