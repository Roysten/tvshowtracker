#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "util/vec.h"

typedef struct Tv_show Tv_show;
typedef struct Tv_show_season Tv_show_season;
typedef struct Tv_show_episode Tv_show_episode;

struct Tv_show
{
    int64_t id;
    const wchar_t *name;
    const wchar_t *first_air_date;
    time_t last_sync;
    Vec seasons;
    Vec episodes;
};

struct Tv_show_season
{
    int season_number;
    const wchar_t *name;
    time_t air_date;
};

struct Tv_show_episode
{
    int season_number;
    int season_episode_number;
    const wchar_t *name;
    time_t air_date;
};

bool TV_SHOW_create(Tv_show *tv_show, int64_t id, const char *name, const char *first_air_date);

void TV_SHOW_destroy(Tv_show *tv_show);

void TV_SHOW_SEASON_destroy(Tv_show_season *season);

void TV_SHOW_EPISODE_destroy(Tv_show_episode *episode);
