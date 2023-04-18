#include "sprite.h"

SpriteArray sprites;

void sprite_init()
{
	sprites.count = 0;
}

void sprite_create(Texture* tex, short x, short y)
{
	/*int add_index;
	if (sprites.count >= SPRITE_MAX)
	{
		int oldest_index = -1;
		Uint64 oldest_time = -1;
		for (int i = 0; i < sprites.count; ++i)
		{
			if (oldest_)
		}
	}
	else
	{
		add_index = sprites.count;
		++sprites.count;
	}*/

	sprites.data[sprites.count].x = x;
	sprites.data[sprites.count].y = y;
	sprites.data[sprites.count].tex = tex;

	sprites.count++;
}
