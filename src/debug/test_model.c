#include "../App.h"
#include "../Tv_show.h"

#include <stdio.h>

// Compile using: 
// gcc -DNOWIN test_model.c App.c Tv_show.c -o test_model

int main()
{
    App app;
    APP_from_file(&app, "app_state.json");

    // for (size_t i = 0; i < vec_size(app.tv_shows); ++i) {
    //     Tv_show* show = vec_get(Tv_show, app.tv_shows, i);
    //     printf("Show name: %ls\n", show->name);

    //     for (size_t j = 0; j < vec_size(show->episodes); ++j) {
    //         Tv_show_episode *episode = vec_get(Tv_show_episode, show->episodes, i);
    //         printf("Ep name: %ls\n", episode->name);
    //     }

    //     for (size_t j = 0; j < vec_size(show->seasons); ++j) {
    //         Tv_show_season *season = vec_get(Tv_show_season, show->seasons, i);
    //         printf("Season name: %ls\n", season->name);
    //     }
    // }

    return 0;
}
