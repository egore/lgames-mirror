/*
 * theme.cpp
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
#include "mixer.h"
#include "theme.h"
#include "game.h"

extern SDL_Renderer *mrc;

/** Load resources and scale if necessary using bricks screen height.
 * Whatever is missing: Fall back to Standard theme. */
void Theme::load(string name, Renderer &r)
{
	string path, fpath;
	vector<string> fnames;
	Texture cbase; /* card front */
	Texture pic;
	int dx, dy, dw, dh;

	path = string(DATADIR) + "/themes/" + name;

	if (!fileExists(path))
		_logerr("CRITICAL ERROR: theme %s not found. I will continue but most likely crash...\n",path.c_str());
	_loginfo("Loading theme %s\n",path.c_str());

	Texture::setRenderScaleQuality(1);

	readDir(path, RD_FILES, fnames);

	/* load wallpapers */
	numWallpapers = 0;
	for (uint i = 0; i < fnames.size(); i++) {
		if (fnames[i].substr(0,2) == "b_") {
			wallpapers[numWallpapers].load(path + "/" + fnames[i]);
			wallpapers[numWallpapers].setBlendMode(0);
			numWallpapers++;
		}
		if (numWallpapers == MAXWALLPAPERS)
			break;
	}
	if (numWallpapers == 0) {
		wallpapers[0].load(stdPath + "/b_0.jpg");
		wallpapers[0].setBlendMode(0);
		numWallpapers = 1;
	}

	/* base images for cards */
	cardBack.load(testRc(path,"c_back.png"));
	cardFocus.load(testRc(path,"c_focus.png"));
	cbase.load(testRc(path,"c_front.png"));

	/* cards - no fallback this has to be present */
	numCards = 0;
	for (uint i = 0; i < fnames.size(); i++) {
		if (fnames[i].substr(0,2) == "p_") {
			_logdebug(1,"  Building %s\n",fnames[i].c_str());
			pic.load(path + "/" + fnames[i]);
			cards[numCards].duplicate(cbase);
			cards[numCards].setBlendMode(1);
			r.setTarget(cards[numCards]);
			if (pic.getWidth() > pic.getHeight()) {
				dw = 0.94 * cbase.getWidth();
				dh = dw * pic.getHeight() / pic.getWidth();
			} else {
				dh = 0.94 * cbase.getHeight();
				dw = dh * pic.getWidth() / pic.getHeight();
			}
			dx = (cbase.getWidth() - dw) / 2;
			dy = (cbase.getHeight() - dh) / 2;
			pic.copy(dx,dy,dw,dh);
			r.clearTarget();
			numCards++;
		}
		if (numCards == MAXPICTURES)
			break;
	}
	cardShadow.createShadow(cbase);

	/* fonts */
	fSmall.load(testRc(path,"f_normal.otf"), r.ry2sy(0.028));
	fNormal.load(testRc(path,"f_bold.otf"), r.ry2sy(0.037));

	/* menu */
	menuX = r.rx2sx(0.14);
	menuY = r.ry2sy(0.63);
	menuItemWidth = r.rx2sx(0.22);
	menuItemHeight = r.ry2sy(0.037);
	menuBackground.load(testRc(path,"menu.png"));
	fMenuNormal.load(testRc(path,"f_normal.otf"), r.ry2sy(0.033));
	fMenuNormal.setColor({255,255,255,255});
	fMenuFocus.load(testRc(path,"f_bold.otf"), r.ry2sy(0.037));
	fMenuFocus.setColor({255,220,0,255});

	/* sounds */
	sClick.load(testRc(path,"s_click.wav"));
	sRemove.load(testRc(path,"s_remove.wav"));
	sFail.load(testRc(path,"s_fail.wav"));
	sMenuClick.load(testRc(path,"s_menuclick.wav"));
	sMenuMotion.load(testRc(path,"s_menumotion.wav"));
}
