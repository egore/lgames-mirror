/*
 * game.cpp
 * (C) 2019 by Michael Speck
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
#include "sprite.h"
#include "game.h"
#include "mixer.h"
#include "theme.h"
#include "menu.h"
#include "view.h"

extern SDL_Renderer *mrc;

/** Initialize game board of size w,h.
 *  @mode determines layout and number of cards depending in @fscreen
 *  @climit is the maximum number of cards that can be dealt */
void Game::init(uint w, uint h, int mode, int fscreen, uint climit)
{
	uint nx = 4, ny = 4;

	/* only regular modes supported for now */
	switch (mode) {
	case GM_SMALL:
		nx = 6;
		ny = 4;
		break;
	case GM_MEDIUM:
		nx = fscreen?8:7;
		ny = fscreen?4:5;
		break;
	case GM_LARGE:
		nx = fscreen?10:8;
		ny = fscreen?5:6;
		break;
	default: /* unknown mode defaults to huge */
		nx = fscreen?12:10;
		ny = fscreen?6:7;
		break;
	}

	double pgap = 0.15;
	uint maxcw = (double)w/(nx*(1 + pgap));
	uint maxch = (double)h/(ny*(1 + pgap));
	uint sz = (maxcw>maxch)?maxch:maxcw;
	uint gap = pgap * sz;
	int cxoff = (w - (nx*sz + (nx-1)*gap)) / 2;
	int cyoff = (h - (ny*sz + (ny-1)*gap)) / 2;
	int x = cxoff, y = cyoff;
	bool skipcenter = ((nx%2) && (ny%2)); /* uneven number of slots? */

	/* layout cards */
	numCards = 0;
	for (uint j = 0; j < ny; j++) {
		if (numCards >= climit)
			break;
		for (uint i = 0; i < nx; i++) {
			if (skipcenter && i==nx/2 && j==ny/2) {
				x += sz + gap;
				continue;
			}
			cards[numCards].setGeometry(x, y, sz, sz);
			numCards++;
			x += sz + gap;
		}
		y += sz + gap;
		x = cxoff;
	}
	numCardsLeft = numCards;

	/* get pair ids (first numCards/2 are chosen from all) */
	random_device rd;
	mt19937 g(rd());
	vector<int> pids;
	for (uint i = 0; i < climit/2; i++)
		pids.push_back(i);
	shuffle(pids.begin(), pids.end(), g);

	/* set card ids */
	vector<int> cids;
	for (uint i = 0; i < numCards/2; i++)
		cids.push_back(pids[i]);
	for (uint i = 0; i < numCards/2; i++)
		cids.push_back(pids[i]);
	shuffle(cids.begin(), cids.end(), g);

	for (uint i = 0; i < numCards; i++)
		cards[i].set(cids[i]);

	/* TODO fix adjacent pairs because this happens a lot */

	gameStarted = false;
	gameover = false;
	gtime = 0;
	score = 0;
	errors = 0;
	numOpenCards = 0;
	isMatch = false;
	closeTimeout.clear();
}

int Game::update(uint ms, int button, int bx, int by)
{
	int ret = GF_NONE;

	if (gameStarted && !gameover) {
		uint ot = gtime;
		gtime += ms;
		if (ot/1000 != gtime/1000)
			ret |= GF_TIMECHANGED;
	}

	if (button) {
		int cid = -1;

		for (uint i = 0; i < numCards; i++)
			if (cards[i].hasFocus(bx,by) && cards[i].isClosed()) {
				cid = i;
				break;
			}

		if (numOpenCards == numMaxOpenCards) {
			ret |= closeCards();
			closeTimeout.clear();
		}

		if (cid != -1 && numOpenCards < numMaxOpenCards) {
			cards[cid].toggle();
			openCardIds[numOpenCards++] = cid;
			if (!gameStarted)
				gameStarted = true;
			ret |= GF_CARDOPENED;
		}

		if (numOpenCards == numMaxOpenCards) {
			uint pid = cards[openCardIds[0]].id;
			isMatch = true;
			for (uint i = 1; i < numOpenCards; i++)
				if (cards[openCardIds[i]].id != pid) {
					isMatch = false;
					break;
				}
			if (isMatch)
				closeTimeout.set(
					config.animations?ANIM_TURNDURATION:0+200);
			else
				closeTimeout.set(config.closedelay*1000);
		}
	}

	if (closeTimeout.running() && closeTimeout.update(ms))
		ret |= closeCards();

	return ret;
}

int Game::closeCards()
{
	int ret = 0;

	for (uint i = 0; i < numOpenCards; i++)
		closedCardIds[i] = openCardIds[i];

	if (isMatch) {
		for (uint i = 0; i < numOpenCards; i++) {
			cards[openCardIds[i]].clear();
			numCardsLeft--;
			if (numCardsLeft == 0)
				gameover = true;
		}
		score++;
		ret |= GF_SCORECHANGED;
		ret |= GF_CARDSREMOVED;
	} else {
		if (checkError()) {
			errors++;
			ret |= GF_ERRORSCHANGED;
		}
		for (uint i = 0; i < numOpenCards; i++)
			cards[openCardIds[i]].toggle();
		ret |= GF_CARDSCLOSED;
	}
	numOpenCards = 0;
	isMatch = false;

	return ret;
}

/** Check mismatched open cards. Last opened card must be the mismatch
 * because for more than 2 cards to be matched it stops at a mismatch.
 *
 * If last opened card is known it's an error.
 * If any closed matching card for first card is known it's an error.
 */
bool Game::checkError()
{
	if (numOpenCards < 2)
		return false; /* should never happen but make sure */
	if (cards[openCardIds[numOpenCards-1]].known)
		return true;
	for (uint i = 0; i < numCards; i++) {
		if (cards[i].removed || cards[i].open)
			continue;
		if (cards[i].id == cards[openCardIds[0]].id && cards[i].known)
			return true;
	}
	return false;
}
