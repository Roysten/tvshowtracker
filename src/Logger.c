#include "Logger.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

struct Logger
{
    int fd;
    char buf[2048];
};

static struct Logger logger;

bool LOGGER_init(const char *log_fname)
{
    logger.fd = open(log_fname, O_CREAT | O_APPEND | O_RDWR, 0644);
    return logger.fd != -1;
}

void LOGGER_log(const char *fmt, ...)
{
    time_t now = 0;
    time(&now);

    struct tm tm;
    localtime_s(&tm, &now);
    size_t written = strftime(logger.buf, sizeof(logger.buf)- 1, "[%Y-%m-%d %H:%M:%S] ", &tm);

    va_list args;
    va_start(args, fmt);
    written += vsnprintf(logger.buf + written, sizeof(logger.buf) - 1 - written, fmt, args);
    logger.buf[written] = '\n';
    write(logger.fd, logger.buf, written + 1);
    va_end(args);
}

void LOGGER_log2(const char *fname, const char *fn, unsigned line_number, const char *fmt, ...)
{
    time_t now = 0;
    time(&now);

    struct tm tm;
    localtime_s(&tm, &now);
    size_t written = strftime(logger.buf, sizeof(logger.buf)- 1, "[%Y-%m-%d %H:%M:%S] ", &tm);

    va_list args;
    va_start(args, fmt);
    written += vsnprintf(logger.buf + written, sizeof(logger.buf) - 1 - written, fmt, args);
    written += snprintf(logger.buf + written, sizeof(logger.buf) - 1 - written, " (%s:%u)\n", fname, line_number);
    write(logger.fd, logger.buf, written);
    va_end(args);
}
