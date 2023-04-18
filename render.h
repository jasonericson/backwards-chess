#pragma once

#include "main.h"

#include "sprite.h"

extern int16 vertices[12 * SPRITE_MAX];
extern float uvs[12 * SPRITE_MAX];

void render_init();
void render_update();
void render_cleanup();
