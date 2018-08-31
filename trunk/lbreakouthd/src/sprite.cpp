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

extern SDL_Renderer *mrc;

Particle::Particle(GridImage &simg, int gx, int gy,
					int sx, int sy, int sw, int sh,
					double px, double py, double vx, double vy,
					double vpms, uint lifetime)
	: pos(px,py), vel(vx,vy)
{
	/* create image */
	img.create(sw,sh);
	SDL_Texture *oldTarget = SDL_GetRenderTarget(mrc);
	SDL_SetRenderTarget(mrc, img.getTex());
	simg.copy(gx,gy,sx,sy,sw,sh,0,0);
	SDL_SetRenderTarget(mrc, oldTarget);

	/* adjust velocity vector */
	vel.setLength(vpms);

	/* use lifetime to count down alpha from 255 to 0 */
	sc.init(SCT_ONCE, 255, 0, lifetime/255.0);
}
