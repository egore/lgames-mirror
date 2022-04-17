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
class Menu;

enum {
	MAXWALLPAPERS = 10,
	MAXMOTIFS = 50
};

class Motif {
	Texture motif;
	string caption;
	Label label;
public:
	void set(Texture &t, const string &c, Font &f) {
		motif.duplicate(t);
		size_t pos = c.find('.');
		if (pos != string::npos)
			caption = c.substr(0,pos);
		else
			caption = c;
		label.setText(f,caption);
	}
	Texture& getTexture() {return motif;}
	const string& getCaption() {return caption;}
	Label& getLabel() {return label;}
};

class Theme {
	friend View;
	friend Menu;

	string stdPath; /* path to standard theme for fallbacks */

	Texture menuBackground;
	uint menuX, menuY, menuItemWidth, menuItemHeight;
	Font fMenuNormal, fMenuFocus;

	Texture wallpapers[MAXWALLPAPERS];
	uint numWallpapers;
	Texture cardBack, cardFocus, cardShadow;
	Motif motifs[MAXMOTIFS];
	uint numMotifs;
	Font fSmall, fNormal, fNormalHighlighted;

	Sound sClick, sFail, sRemove;
	Sound sMenuClick, sMenuMotion;

	const string &testRc(const string &path, const string &fname) {
		static string fpath; /* not thread safe */
		if (fileExists(path + "/" + fname))
			fpath = path + "/" + fname;
		else
			fpath = stdPath + "/" + fname;
		return fpath;
	}
public:
	Theme() : menuX(0), menuY(0), menuItemWidth(0), menuItemHeight(0),
			numWallpapers(0), numMotifs(0)
	{
		stdPath = string(DATADIR) + "/themes/Standard";
	}
	void load(string name, Renderer &r);
};

#endif /* SRC_THEME_H_ */
