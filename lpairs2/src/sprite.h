/*
 * sprite.h
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

#ifndef SRC_SPRITE_H_
#define SRC_SPRITE_H_

/* Graphical object (hail to the C64 :) that can be rendered and updated. */
class Sprite {
public:
	virtual ~Sprite() {};
	virtual int update(uint ms) = 0; /* return 1 if to be removed, 0 otherwise */
	virtual void render() = 0;
};

class FadeAnimation : public Sprite {
	Texture &texture;
	Vec pos, vel;
	uint w, h;
	SmoothCounter alphaCounter;
public:
	FadeAnimation(Texture &t, double sx, double sy, double dx, double dy,
					uint dw, uint dh, uint duration);
	int update(uint ms) {
		pos.add(ms, vel);
		return alphaCounter.update(ms);
	}
	void render() {
		texture.setAlpha(alphaCounter.get());
		texture.copy(pos.getX(), pos.getY(), w, h);
		texture.clearAlpha();
	}
};

class TurnAnimation : public Sprite {
	Texture &t1, &t2;
	int x, y; /* center position */
	uint w, h;
	uint duration;
	Timeout timer;
public:
	TurnAnimation(Texture &_t1, Texture &_t2, int dx, int dy,
					uint dw, uint dh, uint dur);
	int update(uint ms) {
		return timer.update(ms);
	}
	void render() {
		uint phasedur = duration/2;
		if (timer.get() < phasedur) {
			/* phase 2: collapse first texture */
			uint dw = w * (phasedur - timer.get()) / phasedur;
			t2.copy(x - dw/2, y - h/2, dw, h);
		} else {
			/* phase 1: unfold second texture */
			uint dw = w * (timer.get() - phasedur) / phasedur;
			t1.copy(x - dw/2, y - h/2, dw, h);
		}
	}
};

#endif /* SRC_SPRITE_H_ */
