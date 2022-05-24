#pragma once

#include <stdint.h>
#include <stddef.h>

const wchar_t *STR_UTIL_convert(const char* src);

char *STR_UTIL_convert_utf8(const wchar_t *src);
