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

Font *MenuItem::getCurFont() {
	if (!parent)
		return NULL;
	if (focus)
		return parent->getFocusFont();
	else
		return parent->getNormalFont();
}

void MenuItem::render() {
	if (Font *f = getCurFont()) {
		f->setAlign(ALIGN_X_LEFT | ALIGN_Y_CENTER);
		f->write(x,y+h/2,caption);
	}
}

void MenuItemRange::render() {
	MenuItem::render();
	if (Font *f = getCurFont()) {
		f->setAlign(ALIGN_X_RIGHT | ALIGN_Y_CENTER);
		f->write(x+w,y+h/2,to_string(val));
	}
}

void MenuItemList::render() {
	MenuItem::render();
	if (val < 0 || (uint)val >= options.size())
		return;
	if (Font *f = getCurFont()) {
		f->setAlign(ALIGN_X_RIGHT | ALIGN_Y_CENTER);
		f->write(x+w,y+h/2,options[val]);
	}
}

void MenuItemKey::render() {
	MenuItem::render();
	if (Font *f = getCurFont()) {
		f->setAlign(ALIGN_X_RIGHT | ALIGN_Y_CENTER);
		if (waitForNewKey)
			f->write(x+w,y+h/2,"???");
		else
			f->write(x+w,y+h/2,SDL_GetScancodeName((SDL_Scancode)sc));
	}
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
