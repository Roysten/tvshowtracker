#include "Json_writer.h"

#include <inttypes.h>
#include <stdlib.h>

#include "../util/str_util.h"

static void write_json_string(FILE * f, const char* str);

void JSON_OBJECT_WRITER_init(Json_object_writer *writer, FILE *ostream)
{
    *writer = (Json_object_writer) {
        .ostream = ostream,
    };
    fputc('{', writer->ostream);
}

void JSON_OBJECT_WRITER_add_str(Json_object_writer *writer, const char *name, const char *txt)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    fprintf(writer->ostream, "\"%s\":", name);
    write_json_string(writer->ostream, txt);
    writer->member_count += 1;
}

void JSON_OBJECT_WRITER_add_wstr(Json_object_writer *writer, const char *name, const wchar_t *txt)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    char *tmp = STR_UTIL_convert_utf8(txt);
    fprintf(writer->ostream, "\"%s\":", name);
    write_json_string(writer->ostream, tmp);
    free(tmp);
    writer->member_count += 1;
}

void JSON_OBJECT_WRITER_add_int(Json_object_writer *writer, const char *name, int64_t value)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    fprintf(writer->ostream, "\"%s\":%" PRId64, name, value);
    writer->member_count += 1;
}

void JSON_OBJECT_WRITER_add_bool(Json_object_writer *writer, const char *name, bool value)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    fprintf(writer->ostream, "\"%s\":%s", name, value ? "true" : "false");
    writer->member_count += 1;
}

Json_object_writer JSON_OBJECT_WRITER_add_object(Json_object_writer *writer, const char *name)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    fprintf(writer->ostream, "\"%s\":", name);
    Json_object_writer child;
    JSON_OBJECT_WRITER_init(&child, writer->ostream);
    writer->member_count += 1;
    return child;
}

Json_array_writer JSON_OBJECT_WRITER_add_array(Json_object_writer *writer, const char *name)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    fprintf(writer->ostream, "\"%s\":", name);
    Json_array_writer child;
    JSON_ARRAY_WRITER_init(&child, writer->ostream);
    writer->member_count += 1;
    return child;
}

void JSON_OBJECT_WRITER_close(Json_object_writer *writer)
{
    fputc('}', writer->ostream);
}

void JSON_ARRAY_WRITER_init(Json_array_writer *writer, FILE *ostream)
{
    *writer = (Json_array_writer) {
        .ostream = ostream,
    };
    fputc('[', writer->ostream);
}

void JSON_ARRAY_WRITER_add_str(Json_array_writer *writer, const char *txt)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    write_json_string(writer->ostream, txt);
    writer->member_count += 1;
}

void JSON_ARRAY_WRITER_add_wstr(Json_array_writer *writer, const wchar_t *txt)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    char *tmp = STR_UTIL_convert_utf8(txt);
    write_json_string(writer->ostream, tmp);
    free(tmp);
    writer->member_count += 1;
}

void JSON_ARRAY_WRITER_add_int(Json_array_writer *writer, int64_t value)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    fprintf(writer->ostream, "%" PRId64, value);
    writer->member_count += 1;
}

void JSON_ARRAY_WRITER_add_bool(Json_array_writer *writer, bool value)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    fprintf(writer->ostream, "%s", value ? "true" : "false");
    writer->member_count += 1;
}

Json_object_writer JSON_ARRAY_WRITER_add_object(Json_array_writer *writer)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    Json_object_writer child;
    JSON_OBJECT_WRITER_init(&child, writer->ostream);
    writer->member_count += 1;
    return child;
}

Json_array_writer JSON_ARRAY_WRITER_add_array(Json_array_writer *writer)
{
    if (writer->member_count > 0) {
        fputc(',', writer->ostream);
    }
    Json_array_writer child;
    JSON_ARRAY_WRITER_init(&child, writer->ostream);
    writer->member_count += 1;
    return child;
}

void JSON_ARRAY_WRITER_close(Json_array_writer *writer)
{
    fputc(']', writer->ostream);
}

static void write_json_string(FILE * f, const char* str)
{
	fputc('"', f);
	for (const char *c = str; *c != '\0'; ++c) {
		switch(*c) {
			case '"':
			case '\\':
			case '/':
			case '\b':
			case '\f':
			case '\n':
			case '\r':
			case '\t':
				fputc('\\', f);
				break;
			default:
				break;
		}
		fputc(*c, f);
	}
	fputc('"', f);
}
