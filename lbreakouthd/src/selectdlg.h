/*
 * selectdlg.h
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

#ifndef SRC_SELECTDLG_H_
#define SRC_SELECTDLG_H_

enum {
	SEL_NONE = -1,
	SEL_PREV = -2,
	SEL_NEXT = -3
};

class SelectDialog {
	Theme &theme;
	bool quitReceived;
	vector<string> entries;
	int sel;
	uint pos, max, vlen;
	int lx, ly; /* list start */
	uint cw, ch; /* cell size */

	Image background;
	Image preview;

	void render();
	void renderPreview();
	void goNextPage() {
		if (pos == max)
			return;
		pos += vlen-2;
		if (pos > max)
			pos = max;
	}
	void goPrevPage() {
		if (pos == 0)
			return;
		if (pos < vlen-2)
			pos = 0;
		else
			pos -= vlen-2;
	}
public:
	SelectDialog(Theme &t) : theme(t), quitReceived(false) {
		sel = SEL_NONE;
		pos = max = vlen = 0;
		lx = ly = 0;
		cw = ch = 0;
	}
	void init(vector<string> &list);
	int run();
	string get() {
		if (sel >= 0)
			return entries[sel];
		return "none";
	}
	bool quitRcvd() { return quitReceived; }
};

#endif /* SRC_SELECTDLG_H_ */
