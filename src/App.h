#pragma once

#include "Tv_show.h"
#include "util/vec.h"

typedef struct App App;
struct App
{
    bool dirty;
    Vec tv_shows;
};

bool APP_from_file(App *app, const char *path);

bool APP_to_file(App *app, const char *path);
