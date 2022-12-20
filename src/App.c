#include "App.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "json/Json_writer.h"
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

	Json_object_writer elem_root;
	JSON_OBJECT_WRITER_init(&elem_root, f);
	Json_array_writer elem_tv_shows_array = JSON_OBJECT_WRITER_add_array(&elem_root, "tv_shows");

	for (size_t i = 0; i < app->tv_shows.len; ++i) {
		Tv_show *show = vec_get(Tv_show, app->tv_shows, i);
		int episode_idx = 0;

		Json_object_writer elem_tv_show_object = JSON_ARRAY_WRITER_add_object(&elem_tv_shows_array);
		JSON_OBJECT_WRITER_add_wstr(&elem_tv_show_object, "name", show->name);
		JSON_OBJECT_WRITER_add_int(&elem_tv_show_object, "id", show->id);
		JSON_OBJECT_WRITER_add_int(&elem_tv_show_object, "last_sync", show->last_sync);
		Json_array_writer elem_tv_show_seasons_array = JSON_OBJECT_WRITER_add_array(&elem_tv_show_object, "seasons");

		for (size_t j = 0; j < show->seasons.len; ++j) {
			Tv_show_season *season = vec_get(Tv_show_season, show->seasons, j);
			Json_object_writer elem_tv_show_season_object = JSON_ARRAY_WRITER_add_object(&elem_tv_show_seasons_array);
			JSON_OBJECT_WRITER_add_wstr(&elem_tv_show_season_object, "name", season->name);
			JSON_OBJECT_WRITER_add_int(&elem_tv_show_season_object, "air_date", season->air_date);
			JSON_OBJECT_WRITER_add_int(&elem_tv_show_season_object, "season_number", season->season_number);
			Json_array_writer elem_tv_show_season_episodes_array = JSON_OBJECT_WRITER_add_array(&elem_tv_show_season_object, "episodes");

			for (; episode_idx < show->episodes.len; ++episode_idx) {
				Tv_show_episode *episode = vec_get(Tv_show_episode, show->episodes, episode_idx);
				if (episode->season_number != season->season_number) {
					break;
				}

				Json_object_writer elem_tv_show_season_episode_object = JSON_ARRAY_WRITER_add_object(&elem_tv_show_season_episodes_array);
				JSON_OBJECT_WRITER_add_wstr(&elem_tv_show_season_episode_object, "name", episode->name);
				JSON_OBJECT_WRITER_add_int(&elem_tv_show_season_episode_object, "air_date", episode->air_date);
				JSON_OBJECT_WRITER_add_int(&elem_tv_show_season_episode_object, "season_number", episode->season_number);
				JSON_OBJECT_WRITER_add_int(&elem_tv_show_season_episode_object, "episode_number", episode->season_episode_number);
				JSON_OBJECT_WRITER_close(&elem_tv_show_season_episode_object);
			}
			JSON_ARRAY_WRITER_close(&elem_tv_show_season_episodes_array);
			JSON_OBJECT_WRITER_close(&elem_tv_show_season_object);
		}
		JSON_ARRAY_WRITER_close(&elem_tv_show_seasons_array);
		JSON_OBJECT_WRITER_close(&elem_tv_show_object);
	}

	JSON_ARRAY_WRITER_close(&elem_tv_shows_array);
	JSON_OBJECT_WRITER_close(&elem_root);
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
