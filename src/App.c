#include "App.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "json/pdjson.h"
#include "Logger.h"
#include "Tv_show.h"
#include "util/str_util.h"
#include "util/vec.h"

static bool load_tv_shows(json_stream *stream, App *app);
static bool load_tv_show(json_stream *stream, Tv_show *show);
static bool load_tv_show_seasons(json_stream *stream, Tv_show *show);
static bool load_tv_show_season(json_stream *stream, Tv_show *show, Tv_show_season *season);
static bool load_tv_show_season_episodes(json_stream *stream, Tv_show *show, Tv_show_season *season);
static bool load_tv_show_season_episode(json_stream *stream, Tv_show_episode *episode);
static void write_json_string(FILE * f, const char* str);

bool APP_from_file(App *app, const char *path)
{
	*app = (App){0};

	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		app->tv_shows = vec_create(Tv_show);
		return false;
	}

	struct json_stream stream;
	json_open_stream(&stream, f);
	enum json_type event = json_next(&stream);
	if (event != JSON_OBJECT) {
		fclose(f);
		return false;
	}

	bool load_ok = false;
	for (enum json_type event = json_next(&stream); event != JSON_OBJECT_END; event = json_next(&stream)) {
		if (event != JSON_STRING) {
			LOG("Expected JSON_STRING, got type %d instead", event);
			return false;
		}

		const char *key = json_get_string(&stream, NULL);
		if (strcmp(key, "tv_shows") == 0) {
			load_ok = load_tv_shows(&stream, app); 
		} else {
			json_skip(&stream);
		}
	}

	fclose(f);
	return load_ok;
}

bool APP_to_file(App *app, const char *path)
{
	FILE *f = fopen(path, "wb");
	if (f == NULL) {
		LOG("Could not open app_state.json");
		return false;
	}

	fprintf(f, "{\"tv_shows\":[");
	for (size_t i = 0; i < app->tv_shows.len; ++i) {
		Tv_show *show = vec_get(Tv_show, app->tv_shows, i);
		int episode_idx = 0;
		char *converted_name = STR_UTIL_convert_utf8(show->name);
		fprintf(f, "{\"name\":");
		write_json_string(f, converted_name);
		fprintf(f, ",\"id\":%" PRId64 ",\"last_sync\":%" PRId64 ",\"seasons\":[", show->id, show->last_sync);
		free(converted_name);
		for (size_t j = 0; j < show->seasons.len; ++j) {
			Tv_show_season *season = vec_get(Tv_show_season, show->seasons, j);
			converted_name = STR_UTIL_convert_utf8(season->name);
			fprintf(f, "{\"name\":");
			write_json_string(f, converted_name);
			fprintf(f, ",\"air_date\":%" PRId64 ",\"season_number\":%d,\"episodes\":[", season->air_date, season->season_number);
			free(converted_name);
			int start = episode_idx;
			for (; episode_idx < show->episodes.len; ++episode_idx) {
				Tv_show_episode *episode = vec_get(Tv_show_episode, show->episodes, episode_idx);
				if (episode->season_number != season->season_number) {
					fprintf(f, "]}");
					break;
				}

				if (episode_idx != start) {
					fputs(",", f);
				}
				converted_name = STR_UTIL_convert_utf8(episode->name);
				fprintf(f, "{\"name\":");
				write_json_string(f, converted_name);
				fprintf(f, ",\"air_date\":%" PRId64 ",\"season_number\":%d,\"episode_number\":%d}", episode->air_date, episode->season_number, episode->season_episode_number);
				free(converted_name);
			}
			if (j < show->seasons.len - 1) {
				fprintf(f, ",");
			} else {
				fprintf(f, "]}");
			}
		}
		
		if (i < app->tv_shows.len - 1) {
			fprintf(f, "]},");
		} else {
			fprintf(f, "]}");
		}
	}
	fprintf(f, "]}");
	fclose(f);

	return true;
}

static bool load_tv_shows(json_stream *stream, App *app)
{
	enum json_type event = json_next(stream);
	if (event != JSON_ARRAY) {
		return false;
	}

	app->tv_shows = vec_create(Tv_show);
	for (event = json_next(stream); event != JSON_ARRAY_END; event = json_next(stream)) {
		if (event != JSON_OBJECT) {
			LOG("Expected JSON_OBJECT, got type %d instead", event);
			return false;
		}

		vec_push(Tv_show, app->tv_shows, (Tv_show){0});
		if (!load_tv_show(stream, vec_back(Tv_show, app->tv_shows))) {
			return false;
		}
	}
	return true;
}

static bool load_tv_show(json_stream *stream, Tv_show *show)
{
	show->episodes = vec_create(Tv_show_episode);
	for (enum json_type event = json_next(stream); event != JSON_OBJECT_END; event = json_next(stream)) {
		if (event != JSON_STRING) {
			LOG("Expected JSON_STRING, got type %d instead", event);
			return false;
		}

		const char *key = json_get_string(stream, NULL);
		if (strcmp(key, "name") == 0) {
			json_next(stream);
			show->name = STR_UTIL_convert(json_get_string(stream, NULL));
		} else if (strcmp(key, "id") == 0) {
			json_next(stream);
			show->id = json_get_number(stream);
		} else if (strcmp(key, "last_sync") == 0) {
			json_next(stream);
			show->last_sync = json_get_number(stream);
		} else if (strcmp(key, "seasons") == 0) {
			if (json_next(stream) != JSON_ARRAY) {
				LOG("Expected JSON_ARRAY, got type %d instead", event);
				return false;
			}

			if (!load_tv_show_seasons(stream, show)) {
				LOG("Load season failure");
				return false;
			}
		} else {
			json_skip(stream);
		}
	}

	return true;
}

static bool load_tv_show_seasons(json_stream *stream, Tv_show *show)
{
	show->seasons = vec_create(Tv_show_season);
	for (enum json_type event = json_next(stream); event != JSON_ARRAY_END; event = json_next(stream)) {
		if (event != JSON_OBJECT) {
			LOG("Expected JSON_OBJECT, got type %d instead", event);
			return false;
		}

		vec_push(Tv_show_season, show->seasons, (Tv_show_season){0});
		if (!load_tv_show_season(stream, show, vec_back(Tv_show_season, show->seasons))) {
			return false;
		}
	}
	return true;
}

static bool load_tv_show_season(json_stream *stream, Tv_show *show, Tv_show_season *season)
{
	for (enum json_type event = json_next(stream); event != JSON_OBJECT_END; event = json_next(stream)) {
		if (event != JSON_STRING) {
			LOG("Expected JSON_STRING, got type %d instead", event);
			return false;
		}

		const char *key = json_get_string(stream, NULL);
		if (strcmp(key, "name") == 0) {
			json_next(stream);
			season->name = STR_UTIL_convert(json_get_string(stream, NULL));
		} else if (strcmp(key, "air_date") == 0) {
			json_next(stream);
			season->air_date = json_get_number(stream);
		} else if (strcmp(key, "season_number") == 0) {
			json_next(stream);
			season->season_number = json_get_number(stream);
		} else if (strcmp(key, "episodes") == 0) {
			if (json_next(stream) != JSON_ARRAY) {
				LOG("Expected JSON_ARRAY, got type %d instead", event);
				return false;
			}

			if (!load_tv_show_season_episodes(stream, show, season)) {
				return false;
			}
		} else {
			json_skip(stream);
		}
	}
	return true;
}

static bool load_tv_show_season_episodes(json_stream *stream, Tv_show *show, Tv_show_season *season)
{
	for (enum json_type event = json_next(stream); event != JSON_ARRAY_END; event = json_next(stream)) {
		if (event != JSON_OBJECT) {
			LOG("Expected JSON_OBJECT, got type %d instead", event);
			return false;
		}

		vec_push(Tv_show_episode, show->episodes, (Tv_show_episode){0});
		vec_back(Tv_show_episode, show->episodes)->season_number = season->season_number;
		if (!load_tv_show_season_episode(stream, vec_back(Tv_show_episode, show->episodes))) {
			return false;
		}
	}
	return true;
}

static bool load_tv_show_season_episode(json_stream *stream, Tv_show_episode *episode)
{
	for (enum json_type event = json_next(stream); event != JSON_OBJECT_END; event = json_next(stream)) {
		if (event != JSON_STRING) {
			LOG("Expected JSON_STRING, got type %d instead", event);
			return false;
		}

		const char *key = json_get_string(stream, NULL);
		if (strcmp(key, "name") == 0) {
			json_next(stream);
			episode->name = STR_UTIL_convert(json_get_string(stream, NULL));
		} else if (strcmp(key, "air_date") == 0) {
			json_next(stream);
			episode->air_date = json_get_number(stream);
		} else if (strcmp(key, "episode_number") == 0) {
			json_next(stream);
			episode->season_episode_number = json_get_number(stream);
		} else {
			json_skip(stream);
		}
	}
	return true;
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
