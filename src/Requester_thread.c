#include "Requester_thread.h"

#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <processthreadsapi.h>
#include <winbase.h>
#include <synchapi.h>

#include "json/pdjson.h"

#include "Logger.h"
#include "Tv_show.h"
#include "rest/Rest_client.h"
#include "util/str_util.h"
#include "util/vec.h"

typedef struct Context_search_show Context_search_show;
struct Context_search_show
{
    Requester_thread *requester_thread;
    const wchar_t *search_term;
    Tv_show *show_storage;
    size_t show_storage_len;
    Show_search_callback callback;
};

typedef struct Context_show_info Context_show_info;
struct Context_show_info
{
    Requester_thread *requester_thread;
	int64_t show_id;
	Show_info_callback callback;
};

typedef struct Context_refresh_shows Context_refresh_shows;
struct Context_refresh_shows
{
	Requester_thread *requester_thread;
	Vec shows;
	Show_info_callback progress_callback;
	Show_refresh_all_finished_callback finished_callback;
};

static DWORD thread_loop(void *parameter);
static bool init_rest_client(Rest_client *client);
static void handle_request_search_show(ULONG_PTR data);
static bool parse_search_show_json(const char* json_txt, Tv_show *show_storage, size_t show_storage_len);
static bool parse_search_show_info_json(json_stream *stream, Tv_show *show);
static bool json_streq( json_stream *stream, const char *key);
static void handle_request_show_info(ULONG_PTR data);
static bool parse_show_info_json(const char *json_txt, Tv_show *show);
static bool parse_show_info_seasons_json(json_stream *stream, Tv_show *show);
static bool parse_show_info_season_json(const char *json_txt, Tv_show *show, Tv_show_season *season);
static time_t parse_date(const char *date_txt);
static void handle_refresh_shows(ULONG_PTR data);

static const int REST_CLIENT_RECV_BUF_LEN = 1000000;
static const wchar_t REST_API_URL[] = L"api.themoviedb.org";

#define str(a) #a

void REQUESTER_THREAD_create(Requester_thread *thread)
{
    *thread = (Requester_thread) {
        .handle = CreateThread(NULL, 0, thread_loop, thread, 0, NULL),
        .stop = false,
    };
    init_rest_client(&thread->client);
}

void REQUESTER_THREAD_search_show(Requester_thread *thread, const wchar_t *search_term,
	Tv_show *show_storage, size_t show_storage_len, Show_search_callback callback)
{
    Context_search_show *context_search_show = malloc(sizeof(*context_search_show));
    *context_search_show = (Context_search_show) {
        .requester_thread = thread,
        .search_term = search_term,
        .show_storage = show_storage,
        .show_storage_len = show_storage_len,
        .callback = callback,
    };
    QueueUserAPC(handle_request_search_show, thread->handle, (ULONG_PTR) context_search_show);
}

void REQUESTER_THREAD_get_show_info(Requester_thread *thread, int64_t show_id, Show_info_callback callback)
{
    Context_show_info *context_show_info = malloc(sizeof(*context_show_info));
	*context_show_info = (Context_show_info) {
		.requester_thread = thread,
		.show_id = show_id,
		.callback = callback,
	};
    QueueUserAPC(handle_request_show_info, thread->handle, (ULONG_PTR) context_show_info);
}

static DWORD thread_loop(void *parameter)
{
    Requester_thread *thread = parameter;
    while (!thread->stop) {
        SleepEx(INFINITE, TRUE);
    }
    return 0;
}

static bool init_rest_client(Rest_client *client)
{
	bool create_ok = REST_CLIENT_create(client, REST_CLIENT_RECV_BUF_LEN);
	if (!create_ok) {
		LOG("Unable to create rest client");
		return false;
	}

	bool connect_ok = REST_CLIENT_connect(client, REST_API_URL);
	if (!connect_ok) {
		LOG("Unable to connect rest client");
	}

	return true;
}

static void handle_request_search_show(ULONG_PTR data)
{
    Context_search_show *context_search_show = (Context_search_show*) data;
    size_t search_term_len = wcslen(context_search_show->search_term);
    size_t urlencoded_buf_len = search_term_len * 3 + 1;
	wchar_t *urlencoded_buf = calloc(urlencoded_buf_len, sizeof(*urlencoded_buf));
    REST_CLIENT_urlencode(context_search_show->search_term, urlencoded_buf);

	size_t request_uri_buf_len = urlencoded_buf_len + 200;
	wchar_t *request_uri_buf = calloc(request_uri_buf_len, sizeof(wchar_t));
    swprintf(request_uri_buf, request_uri_buf_len, L"/3/search/tv?api_key=%s&language=en-US&page=1&query=%ls", str(REST_API_KEY), urlencoded_buf);
	free(urlencoded_buf);
    LOG("Request: %ls", request_uri_buf);

    bool request_ok = REST_CLIENT_get_resource(&context_search_show->requester_thread->client, request_uri_buf, L"GET");
	free(request_uri_buf);
    if (!request_ok) {
        context_search_show->callback(false);
        return;
    }

    parse_search_show_json(context_search_show->requester_thread->client.buf, context_search_show->show_storage, context_search_show->show_storage_len);
    context_search_show->callback(true);

    free(context_search_show);
}

static bool parse_search_show_json(const char* json_txt, Tv_show *show_storage, size_t show_storage_len)
{
	// First cleanup results from previous search
	for (int i = 0; i < show_storage_len; ++i) {
		if (show_storage[i].id == 0) {
            break;
		}
        TV_SHOW_destroy(&show_storage[i]);
	}

	json_stream stream;
	json_open_string(&stream, json_txt);
	enum json_type event = json_next(&stream);
	if (event != JSON_OBJECT) {
		json_close(&stream);
		return false;
	}

	bool found_results = false;
	for (event = json_next(&stream); event != JSON_DONE; event = json_next(&stream)) {
		if (event == JSON_ARRAY && strcmp(json_get_string(&stream, NULL), "results") == 0) {
			found_results = true;
			break;
		}
	}

	if (!found_results) {
		json_close(&stream);
		return false;
	}

	int show_index = 0;
	for (event = json_next(&stream); event != JSON_ARRAY_END && show_index < show_storage_len; event = json_next(&stream)) {
		if (parse_search_show_info_json(&stream, &show_storage[show_index])) {
			show_index += 1;
		}
	}

	json_close(&stream);
	return true;
}

static bool parse_search_show_info_json(json_stream *stream, Tv_show *show)
{
	if (json_get_context(stream, NULL) != JSON_OBJECT) {
		return false;
	}

	int64_t show_id = 0;
	char *show_name = NULL;

	for (enum json_type event = json_next(stream); event != JSON_OBJECT_END;) {
		if (json_streq(stream, "name")) {
			event = json_next(stream);
			// Copy the name because the pointer is reused
			show_name = strdup(json_get_string(stream, NULL));
		} else if (json_streq(stream, "id")) {
			event = json_next(stream);
			show_id = json_get_number(stream);
		} else {
			event = json_skip(stream);
		}
	}

	bool result = false;
	if (show_id != 0 && show_name != NULL) {
		result = TV_SHOW_create(show, show_id, show_name);
	}

	if (show_name != NULL) {
		free(show_name);
	}

	return result;
}

static bool json_streq( json_stream *stream, const char *key)
{
	return json_get_string(stream, NULL) != NULL && strcmp(json_get_string(stream, NULL), key) == 0;
}

static void handle_request_show_info(ULONG_PTR data)
{
    Context_show_info *context_info = (Context_show_info*) data;
    LOG("APC for: %" PRIi64, context_info->show_id);

    wchar_t request_uri_buf[256];
    swprintf(request_uri_buf, ARRAYSIZE(request_uri_buf), L"/3/tv/%" PRIi64 "?api_key=%s&language=en-US", context_info->show_id, str(REST_API_KEY));
    LOG("Request: %ls", request_uri_buf);

    bool ok = REST_CLIENT_get_resource(&context_info->requester_thread->client, request_uri_buf, L"GET");
    if (!ok) {
        context_info->callback(false, NULL);
		free(context_info);
        return;
    }

	Tv_show *show = calloc(1, sizeof(*show));
	show->last_sync = time(NULL);
	show->id = context_info->show_id;
	ok = parse_show_info_json(context_info->requester_thread->client.buf, show);
	if (!ok) {
		TV_SHOW_destroy(show);
		context_info->callback(false, NULL);
		free(context_info);
		return;
	}

	show->episodes = vec_create(Tv_show_episode);
	for (int i = 0; i < show->seasons.len; ++i) {
		Tv_show_season *season = vec_get(Tv_show_season, show->seasons, i);
		swprintf(request_uri_buf, ARRAYSIZE(request_uri_buf), L"/3/tv/%" PRIi64 "/season/%d?api_key=%s&language=en-US", show->id, season->season_number, str(REST_API_KEY));
		LOG("Request: %ls", request_uri_buf);
		ok = REST_CLIENT_get_resource(&context_info->requester_thread->client, request_uri_buf, L"GET");
		if (!ok || !parse_show_info_season_json(context_info->requester_thread->client.buf, show, season)) {
			TV_SHOW_destroy(show);
			context_info->callback(false, NULL);
			free(context_info);
			return;
		}
	}

	context_info->callback(ok, show);
    free(context_info);
}

static bool parse_show_info_json(const char *json_txt, Tv_show *show)
{
	json_stream stream;
	json_open_string(&stream, json_txt);

	enum json_type event = json_next(&stream);
	if (event != JSON_OBJECT) {
		json_close(&stream);
		return false;
	}

	for (event = json_next(&stream); event != JSON_OBJECT_END; event = json_next(&stream)) {
		if (event != JSON_STRING) {
			LOG("Expected JSON_STRING, but got %d instead", event);
			json_close(&stream);
			return false;
		}

		if (show->name == NULL && strcmp(json_get_string(&stream, NULL), "name") == 0) {
			json_next(&stream);
			show->name = STR_UTIL_convert(json_get_string(&stream, NULL));
		} else if (strcmp(json_get_string(&stream, NULL), "seasons") == 0) {
			event = json_next(&stream);
			if (event == JSON_ARRAY) {
				if (!parse_show_info_seasons_json(&stream, show)) {
					LOG("Failed to parse seasons json");
					json_close(&stream);
					return false;
				}
			}
		} else {
			json_skip(&stream);
		}
	}

	json_close(&stream);
	return true;
}

static bool parse_show_info_seasons_json(json_stream *stream, Tv_show *show)
{
	show->seasons = vec_create(Tv_show_season);
	for (enum json_type event = json_next(stream); event != JSON_ARRAY_END; event = json_next(stream)) {
		if (event != JSON_OBJECT) {
			LOG("Expected JSON_OBJECT, but got %d instead", event);
			vec_destroy(show->seasons);
			return false;
		}

		vec_push(Tv_show_season, show->seasons, (Tv_show_season) {0});
		Tv_show_season* season = vec_back(Tv_show_season, show->seasons);

		for (event = json_next(stream); event != JSON_OBJECT_END; event = json_next(stream)) {
			if (event != JSON_STRING) {
			LOG("Expected JSON_STRING, but got %d instead", event);
				vec_destroy(show->seasons);
				return false;
			}

			const char *key = json_get_string(stream, NULL);
			if (strcmp(key, "air_date") == 0) {
				event = json_next(stream);
				season->air_date = parse_date(json_get_string(stream, NULL));
			} else if (strcmp(key, "name") == 0) {
				event = json_next(stream);
				season->name = STR_UTIL_convert(json_get_string(stream, NULL));
			} else if (strcmp(key, "episode_count") == 0) {
				event = json_next(stream);
			} else if (strcmp(key, "season_number") == 0) {
				event = json_next(stream);
				season->season_number = json_get_number(stream);
			} else {
				event = json_skip(stream);
			}
		}
	}
	return true;
}

static bool parse_show_info_season_json(const char *json_txt, Tv_show *show, Tv_show_season *season)
{
	json_stream stream;
	json_open_string(&stream, json_txt);

	enum json_type event = json_next(&stream);
	if (event != JSON_OBJECT) {
		LOG("Root is not an object");
		json_close(&stream);
		return false;
	}

	for (event = json_next(&stream); event != JSON_OBJECT_END; event = json_next(&stream)) {
		if (event != JSON_STRING) {
			LOG("Expected JSON_STRING, but got %d instead", event);
			json_close(&stream);
			return false;
		}

		const char *key = json_get_string(&stream, NULL);
		if (strcmp(key, "episodes") == 0) {
			event = json_next(&stream);
			if (event != JSON_ARRAY) {
				LOG("Expected JSON_ARRAY, but got %d instead", event);
				json_close(&stream);
				return false;
			}

			for (event = json_next(&stream); event != JSON_ARRAY_END; event = json_next(&stream)) {
				if (event != JSON_OBJECT) {
					LOG("Expected JSON_OBJECT, but got %d instead", event);
					json_close(&stream);
					return false;
				}

				vec_push(Tv_show_episode, show->episodes, (Tv_show_episode){0});
				Tv_show_episode *episode = vec_back(Tv_show_episode, show->episodes);
				episode->season_number = season->season_number;
				for (event = json_next(&stream); event != JSON_OBJECT_END; event = json_next(&stream)) {
					if (strcmp(key, "air_date") == 0) {
						json_next(&stream);
						episode->air_date = parse_date(json_get_string(&stream, NULL));
					} else if (strcmp(key, "episode_number") == 0) {
						json_next(&stream);
						episode->season_episode_number = json_get_number(&stream);
					} else if (strcmp(key, "name") == 0) {
						json_next(&stream);
						episode->name = STR_UTIL_convert(json_get_string(&stream, NULL));
					} else {
						json_skip(&stream);
					}
				}
			}
		} else {
			json_skip(&stream);
		}
	}
	return true;
}

static time_t parse_date(const char *date_txt)
{
	struct tm tm = {.tm_isdst = -1};
	sscanf(date_txt, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	return _mkgmtime(&tm);
}

void REQUESTER_THREAD_refresh_shows(Requester_thread *thread, Vec shows, 
	Show_info_callback progress_callback, Show_refresh_all_finished_callback finished_callback)
{
	Context_refresh_shows *context = malloc(sizeof(*context));
	if (context == NULL) {
		return;
	}

	*context = (Context_refresh_shows) {
		.requester_thread = thread,
		.shows = shows,
		.progress_callback = progress_callback,
		.finished_callback = finished_callback,
	};


    QueueUserAPC(handle_refresh_shows, thread->handle, (ULONG_PTR) context);
}

void handle_refresh_shows(ULONG_PTR data) {
	Context_refresh_shows *context = (Context_refresh_shows*) data;

	for (size_t i = 0; i < vec_size(context->shows); ++i) {
		Tv_show *show = vec_get(Tv_show, context->shows, i);
		Context_show_info *context_show_info = malloc(sizeof(*context_show_info));
		*context_show_info = (Context_show_info) {
			.requester_thread = context->requester_thread,
			.show_id = show->id,
			.callback = context->progress_callback,
		};
		handle_request_show_info((ULONG_PTR) context_show_info);
	}

	context->finished_callback();
	free(context);
}
