#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct Json_object_writer Json_object_writer;
typedef struct Json_array_writer Json_array_writer;

struct Json_object_writer
{
    FILE *ostream;
    size_t member_count;
    bool closed;
};

struct Json_array_writer
{
    FILE *ostream;
    size_t member_count;
    bool closed;
};

void JSON_OBJECT_WRITER_init(Json_object_writer *writer, FILE *ostream);

void JSON_OBJECT_WRITER_add_str(Json_object_writer *writer, const char *name, const char *txt);

void JSON_OBJECT_WRITER_add_wstr(Json_object_writer *writer, const char *name, const wchar_t *txt);

void JSON_OBJECT_WRITER_add_int(Json_object_writer *writer, const char *name, int64_t value);

void JSON_OBJECT_WRITER_add_bool(Json_object_writer *writer, const char *name, bool value);

Json_object_writer JSON_OBJECT_WRITER_add_object(Json_object_writer *writer, const char *name);

Json_array_writer JSON_OBJECT_WRITER_add_array(Json_object_writer *writer, const char *name);

void JSON_OBJECT_WRITER_close(Json_object_writer *writer);

void JSON_ARRAY_WRITER_init(Json_array_writer *writer, FILE *ostream);

void JSON_ARRAY_WRITER_add_str(Json_array_writer *writer, const char *txt);

void JSON_ARRAY_WRITER_add_wstr(Json_array_writer *writer, const wchar_t *txt);

void JSON_ARRAY_WRITER_add_int(Json_array_writer *writer, int64_t value);

void JSON_ARRAY_WRITER_add_bool(Json_array_writer *writer, bool value);

Json_object_writer JSON_ARRAY_WRITER_add_object(Json_array_writer *writer);

Json_array_writer JSON_ARRAY_WRITER_add_array(Json_array_writer *writer);

void JSON_ARRAY_WRITER_close(Json_array_writer *writer);
