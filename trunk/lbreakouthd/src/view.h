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

enum {
	/* virtual geometry */
	VG_BRICKWIDTH = 40,
	VG_BRICKHEIGHT = 20,
	VG_BRICKAREAWIDTH = MAPWIDTH * VG_BRICKWIDTH,
	VG_BRICKAREAHEIGHT = MAPHEIGHT * VG_BRICKHEIGHT,
	VG_PADDLETILESIZE = 18,
	VG_PADDLEYPOS = VG_BRICKAREAHEIGHT - 2*VG_BRICKHEIGHT,
	VG_BALLRADIUS = 6,
	VG_BALLSIZE = 12,

	/* mixer */
	MIX_CHANNELNUM = 16,
	MIX_CUNKSIZE = 2048
};

class View {
	/* general */
	Config &config;
	Theme theme;
	Mixer mixer;
	MainWindow *mw;

	/* menu */
	unique_ptr<Menu> rootMenu;
	Menu *curMenu, *graphicsMenu;
	int curLevelsetId;
	vector<string> levelsetNames;
	vector<string> themeNames;
	vector<string> modeNames;
	string saveFileName;
	SelectDialog selectDlg;

	/* game */
	ClientGame &cgame;
	Uint32 brickAreaWidth, brickAreaHeight;
	Uint32 brickScreenWidth, brickScreenHeight;
	int scaleFactor; // *100, e.g., 140 means 1.4
	bool quitReceived;
	/* render parts */
	Label lblTitle;
	SmoothCounter lblTitleCounter;
	int curWallpaperId;
	Image imgBackground;
	Image imgBricks;
	int imgBricksX, imgBricksY;
	Image imgScore;
	int imgScoreX, imgScoreY;
	Image imgExtras;
	int imgExtrasX, imgExtrasY;
	Image imgFloor; /* extra wall at bottom */
	int imgFloorX, imgFloorY;
	FrameCounter weaponFrameCounter;
	FrameCounter shotFrameCounter;
	list<unique_ptr<Sprite>> sprites;
	/* stats */
	Uint32 fpsCycles, fpsStart;
	double fps;

	int v2s(int i) { return i * scaleFactor / 100; }
	int s2v(int i) { return i * 100 / scaleFactor; }
	double v2s(double d) { return d * scaleFactor / 100; }
	double s2v(double d) { return d * 100 / scaleFactor; }
	void renderBackgroundImage();
	void renderHiscore(Font &fTitle, Font &fEntry, int x, int y, int w, int h, bool detailed);
	void renderBricksImage();
	void renderScoreImage();
	void renderExtrasImage();
	void renderActiveExtra(int id, int ms, int x, int y);
	void dim();
	bool showInfo(const string &line, bool confirm=false);
	bool showInfo(const vector<string> &text, bool confirm=false);
	void createParticles(BrickHit *hit);
	void createSprites();
	void getBallViewInfo(Ball *ball, int *x, int *y, uint *type);
	void playSounds();
	void createMenus();
	void grabInput(int grab);
	void saveGame();
	int resumeGame();
	void showFinalHiscores();
	int waitForKey(bool confirm);
	void darkenScreen();
	void initTitleLabel();
public:
	View(Config &cfg, ClientGame &_cg);
	~View();
	void init(string t, uint r);
	void run();
	void render();
	void runMenu();
	void renderMenu();
};

#endif /* VIEW_H_ */