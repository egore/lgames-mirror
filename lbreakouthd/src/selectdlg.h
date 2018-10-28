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

class SelectDialog {
	Theme &theme;

	void render();
	void renderPreview();
public:
	SelectDialog(Theme &t) : theme(t) {}
	void init(vector<string> entries);
	void run();
};

#endif /* SRC_SELECTDLG_H_ */
