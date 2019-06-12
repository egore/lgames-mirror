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
					uint dw, uint dh, uint lifetime)
		: texture(t), pos(sx,sy), vel(dx - sx, dy - sy), w(dw), h(dh)
{
	/* adjust velocity so it takes lifetime ms to reach destination */
	double len = vel.getLength();
	vel.setLength(len / lifetime);

	/* use lifetime to count down alpha from 255 to 32 */
	alphaCounter.init(SCT_ONCE, 255, 0, lifetime/255.0);
}
