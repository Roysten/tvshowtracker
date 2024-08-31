#include "Tv_show.h"

#include "util/str_util.h"

bool TV_SHOW_create(Tv_show *tv_show, int64_t id, const char *name, const char *first_air_date)
{
    *tv_show = (Tv_show) {0};
    tv_show->id = id;
    tv_show->name = STR_UTIL_convert(name);
    tv_show->first_air_date = STR_UTIL_convert(first_air_date);

    return tv_show->name != NULL;
}

void TV_SHOW_destroy(Tv_show *tv_show)
{
    free((void *) tv_show->name);
    *tv_show = (Tv_show) {0};

    for (size_t i = 0; i < vec_size(tv_show->seasons); ++i) {
        Tv_show_season *season = vec_get(Tv_show_season, tv_show->seasons, i);
        TV_SHOW_SEASON_destroy(season);
    }
    vec_destroy(tv_show->seasons);

    for (size_t i = 0; i < vec_size(tv_show->episodes); ++i) {
        Tv_show_episode *episode = vec_get(Tv_show_episode, tv_show->episodes, i);
        TV_SHOW_EPISODE_destroy(episode);
    }
    vec_destroy(tv_show->episodes);
}

void TV_SHOW_SEASON_destroy(Tv_show_season *season)
{
    free((void *) season->name);
}

void TV_SHOW_EPISODE_destroy(Tv_show_episode *episode)
{
    free((void *) episode->name);
}
