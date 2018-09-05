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

/** Helper to render a part of the menu item. Position is determined
 * by given alignment. */
void MenuItem::renderPart(const string &str, int align)
{
	if (!parent)
		return; /* should never happen */

	int tx = x + w/2, ty = y + h/2;
	if (align == ALIGN_X_LEFT)
		tx = x;
	else if (align == ALIGN_X_RIGHT)
		tx = x + w;

	Font *f = parent->getNormalFont();
	if (focus)
		f = parent->getFocusFont();

	f->setAlign(align | ALIGN_Y_CENTER);
	f->write(tx,ty,str,focus?255:(255-fadingAlpha));
	if (!focus && fadingAlpha > 0) {
		f = parent->getFocusFont();
		f->setAlign(align | ALIGN_Y_CENTER);
		f->write(tx,ty,str,fadingAlpha);
	}
}

void MenuItem::render() {
	renderPart(caption, ALIGN_X_LEFT);
}

void MenuItemRange::render() {
	renderPart(caption, ALIGN_X_LEFT);
	renderPart(to_string(val), ALIGN_X_RIGHT);
}

void MenuItemList::render() {
	renderPart(caption, ALIGN_X_LEFT);
	renderPart(options[val], ALIGN_X_RIGHT);
}

void MenuItemIntList::render() {
	renderPart(caption, ALIGN_X_LEFT);
	renderPart(to_string(val), ALIGN_X_RIGHT);
}

void MenuItemKey::render() {
	renderPart(caption, ALIGN_X_LEFT);
	if (waitForNewKey)
		renderPart("???", ALIGN_X_RIGHT);
	else
		renderPart(SDL_GetScancodeName((SDL_Scancode)sc), ALIGN_X_RIGHT);
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
