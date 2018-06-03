/*
 * theme.h
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

#ifndef SRC_THEME_H_
#define SRC_THEME_H_

class View;

enum {
	MAXWALLPAPERS= 10
};

class Theme {
	friend View;

	/* loaded from theme.ini */
	string title;
	string author;
	string version;
	uint brickFileWidth;
	uint brickFileHeight;
	string fontSmallName;
	uint fontSmallSize;
	string fontNormalName;
	uint fontNormalSize;
	SDL_Color fontColorNormal;
	SDL_Color fontColorHighlight;
	uint shotFrameNum;
	uint shotAnimDelay;
	uint weaponFrameNum;
	uint weaponAnimDelay;
	uint explFrameNum;
	uint explAnimDelay;

	Image wallpapers[MAXWALLPAPERS];
	uint numWallpapers;
	Image frame;
	GridImage bricks;
	GridImage paddles;
	GridImage balls;
	GridImage extras;
	GridImage shot;
	GridImage weapon;
	GridImage explosions;
	Image life;
	Font fSmall;
	Font fNormal;

	void addBox(Image &img, int x, int y, int w, int h);
public:
	void load(string name, uint screenWidth, uint screenHeight,
				uint brickScreenWidth, uint brickScreenHeight);
};

#endif /* SRC_THEME_H_ */
