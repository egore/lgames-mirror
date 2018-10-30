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

/** Copy (and rearrange list). Set list values */
void SelectDialog::init(vector<string> &list)
{
	uint sw = theme.menuBackground.getWidth();
	uint sh = theme.menuBackground.getHeight();

	vlen = (0.7 * sh) / theme.fNormal.getSize(); /* vlen = displayed entries */
	sel = SEL_NONE;
	pos = max = 0;
	if (list.size() > vlen)
		max = list.size() - vlen;
	cw = 0.2*sw;
	ch = 1.1 * theme.fNormal.getSize();
	lx = 0.1*sw;
	ly = (sh - vlen*ch)/2;

	entries.clear();
	for (auto& e : list) {
		entries.push_back(e);
		_loginfo("%s\n",e.c_str());
	}

	background.createFromScreen();
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
		font.write(lx, y, entries[pos + i]);
	}
	if (pos < max) {
		if (sel == SEL_NEXT)
			font.setColor(theme.fontColorHighlight);
		else
			font.setColor(theme.fontColorNormal);
		font.write(lx, y, _("<Next Page>"));
	}
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
