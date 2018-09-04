/*
 * menu.cpp
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
#include "clientgame.h"
#include "mixer.h"
#include "theme.h"
#include "menu.h"

bool Menu::inputLocked = false;

void MenuItem::render() {
	if (!parent)
		return;
	Font *f = parent->getNormalFont();
	if (focus)
		f = parent->getFocusFont();
	f->setAlign(ALIGN_X_LEFT | ALIGN_Y_CENTER);
	f->write(x,y+h/2,caption);
}

void MenuItemRange::render() {
	MenuItem::render();
	if (!parent)
		return;
	Font *f = parent->getNormalFont();
	if (focus)
		f = parent->getFocusFont();
	f->setAlign(ALIGN_X_RIGHT | ALIGN_Y_CENTER);
		f->write(x+w,y+h/2,to_string(val));
}

void MenuItemList::render() {
	MenuItem::render();
	if (!parent)
		return;
	if (val < 0 || (uint)val >= options.size())
		return;
	Font *f = parent->getNormalFont();
	if (focus)
		f = parent->getFocusFont();
	f->setAlign(ALIGN_X_RIGHT | ALIGN_Y_CENTER);
	f->write(x+w,y+h/2,options[val]);
}

int Menu::handleEvent(const SDL_Event &ev)
{
	int ret = 0;

	if (ev.type == SDL_MOUSEMOTION) {
		bool onItem = false;
		for (auto& i : items)
			if (i->hasPointer(ev.motion.x,ev.motion.y)) {
				if (i.get() == curItem) {
					onItem = true;
					break;
				}
				if (curItem)
					curItem->setFocus(0);
				curItem = i.get();
				curItem->setFocus(1);
				onItem = true;
				ret = AID_FOCUSCHANGED;
				break;
			}
		if (!onItem && curItem) {
			curItem->setFocus(0);
			curItem = NULL;
			ret = AID_FOCUSCHANGED;
		}
		return ret;
	}

	if (ev.type == SDL_MOUSEBUTTONDOWN && !inputLocked) {
		for (auto& i : items)
			if (i->hasPointer(ev.button.x,ev.button.y)) {
				inputLocked = true;
				return i->handleEvent(ev);
			}
	}
	if (ev.type == SDL_MOUSEBUTTONUP)
		inputLocked = false;

	return ret;
}
