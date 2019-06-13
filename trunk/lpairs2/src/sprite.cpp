/*
 * sprite.cpp
 * (C) 2018 by Michael Speck
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tools.h"
#include "sdl.h"
#include "mixer.h"
#include "theme.h"
#include "sprite.h"

/* Create animation that moves from (sx,sy) to (dx,dy) in lifetime ms and fades
 * from 255 to 0 during movement. */
FadeAnimation::FadeAnimation(Texture &t, double sx, double sy, double dx, double dy,
					uint dw, uint dh, uint duration)
		: texture(t), pos(sx,sy), vel(dx - sx, dy - sy), w(dw), h(dh)
{
	/* adjust velocity so it takes lifetime ms to reach destination */
	double len = vel.getLength();
	vel.setLength(len / duration);

	/* use lifetime to count down alpha from 255 to 32 */
	alphaCounter.init(SCT_ONCE, 255, 0, duration/255.0);
}

/** Create animation that turns a card. During first half first texture is
 * collapsed, during second half second texture is unfolded. */
TurnAnimation::TurnAnimation(Texture &_t1, Texture &_t2, int dx, int dy,
					uint dw, uint dh, uint dur)
	: t1(_t1), t2(_t2), x(dx), y(dy), w(dw), h(dh), duration(dur)
{
	timer.set(duration);
}
