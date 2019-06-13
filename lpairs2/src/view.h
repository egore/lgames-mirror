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
	/* general */
	MAX_PLAYERS = 4,

	/* states */
	VS_IDLE = 0,
	VS_OPENINGCARD,
	VS_CLOSINGCARDS,
	VS_OPENINGANDCLOSINGCARDS,

	/* mixer */
	MIX_CHANNELNUM = 16,
	MIX_CUNKSIZE = 2048,

	/* waitForKey types */
	WT_ANYKEY = 0,
	WT_YESNO,
	WT_PAUSE,

	/* animations */
	ANIM_TURNDURATION = 200,
	ANIM_FADEDURATION = 1000
};

class View {
	/* general */
	Renderer &renderer;
	Config &config;
	Theme theme;
	Mixer mixer;

	/* menu */
	bool menuActive;
	unique_ptr<Menu> rootMenu;
	Menu *curMenu, *graphicsMenu;
	vector<string> themeNames;
	Label lblCredits1, lblCredits2;
	bool noGameYet;

	/* game */
	int state;
	Game &game;
	bool quitReceived;
	uint curWallpaperId;
	Texture imgBackground;
	int cxoff, cyoff;
	int shadowOffset; /* shadow offset */
	Label lblScore, lblTime, lblErrors;
	Label lblRestart;
	int mcx, mcy; /* mouse cursor position */
	list<unique_ptr<Sprite>> sprites;

	/* stats */
	Uint32 fpsCycles, fpsStart;
	double fps;

	void createMenus();
	void waitForInputRelease();
	int waitForKey(int type);
	void darkenScreen(int alpha = 32);
	bool showInfo(const string &line, int type);
	bool showInfo(const vector<string> &text, int type);
	void dim();
	void handleMenuEvent(SDL_Event &ev);
	void startGame();
	void changeWallpaper();
	void startTurningAnimation(uint cid);
	bool skipAnimatedCard(uint cid);

public:
	View(Renderer &r, Config &cfg, Game &gm);
	void init(string t, uint f);
	void run();
	void render();
};

#endif /* VIEW_H_ */
