#pragma once

#include <stdbool.h>

#include <windef.h>

#include "rest/Rest_client.h"
#include "Tv_show.h"

typedef struct Requester_thread Requester_thread;
struct Requester_thread
{
    Rest_client client;
    HANDLE handle;
    bool stop;
};

typedef void (*Show_info_callback) (bool success, Tv_show *show);
typedef void (*Show_refresh_all_finished_callback) ();
typedef void (*Show_search_callback) (bool success);

void REQUESTER_THREAD_create(Requester_thread *thread);

void REQUESTER_THREAD_search_show(Requester_thread *thread, const wchar_t *search_term,
    Tv_show *search_storage, size_t show_storage_len, Show_search_callback callback);

void REQUESTER_THREAD_get_show_info(Requester_thread *thread, int64_t show_id, Show_info_callback callback);

void REQUESTER_THREAD_refresh_shows(Requester_thread *thread, Vec shows,
    Show_info_callback progress_callback, Show_refresh_all_finished_callback finished_callback);
