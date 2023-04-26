#pragma once

typedef short           int16;
typedef int             int32;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

extern bool g_mouse_down;
extern bool g_mouse_up;
extern bool g_mouse_long_up;
extern int g_mouse_x;
extern int g_mouse_y;
extern int g_mouse_raw_x;
extern int g_mouse_raw_y;

extern bool g_space_down;

void cursor_set(bool hand);
