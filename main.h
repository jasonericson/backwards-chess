#pragma once

typedef short           int16;
typedef int             int32;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

extern bool g_mouse_down;
extern int g_mouse_x;
extern int g_mouse_y;

extern bool g_space_down;

void cursor_set(bool hand);
