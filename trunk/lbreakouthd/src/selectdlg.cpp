/*
 * selectdlg.cpp
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

#include "sdl.h"
#include "tools.h"
#include "hiscores.h"
#include "clientgame.h"
#include "mixer.h"
#include "theme.h"
#include "selectdlg.h"

extern SDL_Renderer *mrc;
extern Brick_Conv brick_conv_table[BRICK_COUNT];

SetInfo::SetInfo(const string &n, Theme &theme)
{
	name = n;
	levels = 0;
	version = "1.00"; /* default if not found */
	author = "?";

	string fpath = getFullLevelsetPath(n);
	string lines[5+EDIT_HEIGHT];
	ifstream ifs(fpath);
	uint offset = 0;

	if (!ifs.is_open()) {
		_logerr("Levelset %s not found, no preview created\n",n.c_str());
		return;
	}
	for (uint i = 0; i < 5+EDIT_HEIGHT; i++)
		getline(ifs,lines[i]);
	if (lines[0].find("Version") != string::npos) {
		version = lines[0].substr(lines[0].find(':')+1);
		offset = 1;
	}
	author = lines[1 + offset];

	/* TODO count levels */

	/* create preview */
	uint sw = theme.menuBackground.getWidth();
	uint sh = theme.menuBackground.getHeight();
	uint bw = theme.bricks.getGridWidth();
	uint bh = theme.bricks.getGridHeight();
	uint soff = bh/3;
	preview.create(MAPWIDTH*theme.bricks.getGridWidth(),
			MAPHEIGHT*theme.bricks.getGridHeight());
	SDL_SetRenderTarget(mrc, preview.getTex());
	Image& wallpaper = theme.wallpapers[rand()%theme.numWallpapers];
	for (uint wy = 0; wy < sh; wy += wallpaper.getHeight())
		for (uint wx = 0; wx < sw; wx += wallpaper.getWidth())
			wallpaper.copy(wx,wy);
	theme.frameShadow.copy(soff,soff);
	/* XXX direct access to brick conversion table from libgame */
	for (uint j = 0; j < EDITHEIGHT; j++)
		for (uint i = 0; i < EDITWIDTH; i++) {
			int k = -1;
			for ( k = 0; k < BRICK_COUNT; k++ )
				if (lines[4+offset+j][i] == brick_conv_table[k].c)
					break;
			if (k != -1)
				theme.bricksShadow.copy(brick_conv_table[k].id,0,
						(i+1)*bw+bh/3, (1+j)*bh+bh/3);
		}
	for (uint j = 0; j < EDITHEIGHT; j++)
		for (uint i = 0; i < EDITWIDTH; i++) {
			int k = -1;
			for ( k = 0; k < BRICK_COUNT; k++ )
				if (lines[4+offset+j][i] == brick_conv_table[k].c)
					break;
			if (k != -1)
				theme.bricks.copy(brick_conv_table[k].id,0,
							(i+1)*bw, (1+j)*bh);
		}
	theme.frame.copy(0,0);
	SDL_SetRenderTarget(mrc, NULL);
}


/** Create levelset list and previews + layout. */
void SelectDialog::init()
{
	uint sw = theme.menuBackground.getWidth();
	uint sh = theme.menuBackground.getHeight();
	vector<string> list;

	readDir(string(DATADIR)+"/levels", RD_FILES, list);

	vlen = (0.7 * sh) / theme.fNormal.getSize(); /* vlen = displayed entries */
	sel = SEL_NONE;
	pos = max = 0;
	if (list.size() > vlen)
		max = list.size() - vlen;
	cw = 0.2*sw;
	ch = 1.1 * theme.fNormal.getSize();
	lx = 0.1*sw;
	ly = (sh - vlen*ch)/2;
	px = 0.4*sw;
	py = 0.1*sh;
	pw = 0.5*sw;
	ph = MAPWIDTH * pw / MAPHEIGHT;

	background.createFromScreen();

	entries.clear();
	for (auto& e : list) {
		SetInfo *si = new SetInfo(e, theme);
		entries.push_back(unique_ptr<SetInfo>(si));
	}
}

void SelectDialog::render()
{
	Font &font = theme.fNormal;
	int y = ly;

	background.copy();

	font.setAlign(ALIGN_X_LEFT | ALIGN_Y_TOP);
	if (pos > 0) {
		if (sel == SEL_PREV)
			font.setColor(theme.fontColorHighlight);
		else
			font.setColor(theme.fontColorNormal);
		font.write(lx, ly-ch, _("<Previous Page>"));
	}
	for (uint i = 0; i < vlen; i++, y += ch) {
		if (pos + i < entries.size() && sel == (int)(pos + i))
			font.setColor(theme.fontColorHighlight);
		else
			font.setColor(theme.fontColorNormal);
		font.write(lx, y, entries[pos + i]->name);
	}
	if (pos < max) {
		if (sel == SEL_NEXT)
			font.setColor(theme.fontColorHighlight);
		else
			font.setColor(theme.fontColorNormal);
		font.write(lx, y, _("<Next Page>"));
	}

	if (sel >= 0)
		entries[sel]->preview.copy(px,py,pw,ph);
}

int SelectDialog::run()
{
	SDL_Event ev;
	bool leave = false;
	int ret = 0;

	render();
	while (!quitReceived && !leave) {
		/* handle events */
		if (SDL_WaitEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				quitReceived = true;
			if (ev.type == SDL_KEYDOWN) {
				switch (ev.key.keysym.scancode) {
				case SDL_SCANCODE_ESCAPE:
					leave = true;
					break;
				case SDL_SCANCODE_PAGEUP:
					goPrevPage();
					break;
				case SDL_SCANCODE_PAGEDOWN:
					goNextPage();
					break;
				default:
					break;
				}
			}
			if (ev.type == SDL_MOUSEMOTION) {
				if (ev.motion.x >= lx && ev.motion.y >= ly &&
						ev.motion.x < (int)(lx + cw) &&
						ev.motion.y < int(ly + ch*vlen)) {
					sel = pos + (ev.motion.y - ly)/ch;
				} else if (ev.motion.y < ly)
					sel = SEL_PREV;
				else if (ev.motion.y > int(ly + ch*vlen))
					sel = SEL_NEXT;
				else
					sel = SEL_NONE;
			}
			if (ev.type == SDL_MOUSEWHEEL) {
				if (ev.wheel.y < 0)
					goNextPage();
				else if (ev.wheel.y > 0)
					goPrevPage();
			}
			if (ev.type == SDL_MOUSEBUTTONUP) {
				if (sel == SEL_PREV)
					goPrevPage();
				else if (sel == SEL_NEXT)
					goNextPage();
				else if (sel != SEL_NONE) {
					ret = 1;
					leave = true;
				}
			}
		}
		/* render */
		render();
		SDL_RenderPresent(mrc);
		SDL_FlushEvent(SDL_MOUSEMOTION); /* prevent event loop from dying */
	}

	/* clear events for menu loop */
	SDL_FlushEvents(SDL_FIRSTEVENT,SDL_LASTEVENT);

	return ret;
}
