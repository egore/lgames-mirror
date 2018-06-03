/*
 * view.h
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

#ifndef VIEW_H_
#define VIEW_H_

class Animation {
	GridImage& img;
	uint id;
	int x, y; /* position on screen */
	FrameCounter fc;
public:
	Animation(GridImage &_img, uint _id, uint delay, int _x, int _y)
				: img(_img), id(_id), x(_x), y(_y) {
		fc.init(img.getGridSizeX(), delay);
	}
	int update(uint ms) {
		if (fc.update(ms))
			return 1; /* die */
		return 0;
	}
	void render() {
		img.copy(fc.get(), id, x, y);
	}
};

enum {
	/* virtual geometry */
	VG_BRICKWIDTH = 40,
	VG_BRICKHEIGHT = 20,
	VG_BRICKAREAWIDTH = MAPWIDTH * VG_BRICKWIDTH,
	VG_BRICKAREAHEIGHT = MAPHEIGHT * VG_BRICKHEIGHT,
	VG_PADDLETILESIZE = 18,
	VG_PADDLEYPOS = VG_BRICKAREAHEIGHT - 2*VG_BRICKHEIGHT,
	VG_BALLRADIUS = 6,
	VG_BALLSIZE = 12
};

class View {
	Config &config;
	Theme theme;
	MainWindow *mw;
	ClientGame &cgame;
	Uint32 brickAreaWidth, brickAreaHeight;
	Uint32 brickScreenWidth, brickScreenHeight;
	int scaleFactor; // *100, e.g., 140 means 1.4
	bool quitReceived;
	/* render parts */
	int curWallpaperId;
	Image imgBackground;
	Image imgBricks;
	Uint32 imgBricksX, imgBricksY;
	Image imgScore;
	Uint32 imgScoreX, imgScoreY;
	Image imgExtras;
	Uint32 imgExtrasX, imgExtrasY;
	FrameCounter weaponFrameCounter;
	FrameCounter shotFrameCounter;
	list<unique_ptr<Animation>> sprites;
	/* stats */
	Uint32 fpsCycles, fpsStart;
	double fps;

	int v2s(int i) { return i * scaleFactor / 100; }
	int s2v(int i) { return i * 100 / scaleFactor; }
	double v2s(double d) { return d * scaleFactor / 100; }
	double s2v(double d) { return d * 100 / scaleFactor; }
	void renderBackgroundImage();
	void renderHiscore(int x, int y, int w, int h);
	void renderBricksImage();
	void renderScoreImage();
	void renderExtrasImage();
	void renderActiveExtra(int id, int ms, int x, int y);
	void dim();
	void showInfo(const string& str);
	void createSprites();
public:
	View(Config &cfg, ClientGame &_cg);
	~View();
	void loadAsset(const string &name) {
		theme.load("Standard",mw->getWidth(),mw->getHeight(),
				brickScreenWidth, brickScreenHeight);
		theme.load(name,mw->getWidth(),mw->getHeight(),
				brickScreenWidth, brickScreenHeight);
	}
	void run();
	void render();
};

#endif /* VIEW_H_ */
