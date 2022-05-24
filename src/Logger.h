#pragma once

#include <stdbool.h>

bool LOGGER_init(const char *log_fname);

void LOGGER_log(const char *fmt, ...);

void LOGGER_log2(const char *fname, const char *fn, unsigned line_number, const char *fmt, ...);

#define LOG(fmt, ...) do { LOGGER_log2(__FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__); } while (false)
