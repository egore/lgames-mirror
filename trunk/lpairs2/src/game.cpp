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

/** Initialize game board of virtual size w,h.
 *  @mode determines layout and number of cards depending in @fscreen
 *  @climit is the maximum number of cards that can be dealt */
void Game::init(uint w, uint h, uint mode, uint setsize, uint matchsize, int fscreen, uint climit)
{
	uint nx = 4, ny = 4;

	/* game mode */
	curPlayer = 0;
	switch (mode) {
	case GM_SOLO:
		numPlayers = 1;
		players[0].init("You", PC_HUMAN);
		break;
	case GM_VSHUMAN:
		numPlayers = 2;
		players[0].init("Player A", PC_HUMAN);
		players[1].init("Player B", PC_HUMAN);
		break;
	case GM_VSCPU:
		numPlayers = 2;
		players[0].init("You", PC_HUMAN);
		players[1].init("CPU", PC_CPU);
		break;
	}

	/* only regular modes supported for now */
	switch (setsize) {
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

	double pgap = 0.1; /* gap in percent of card width */
	uint maxcw = (double)w/(nx*(1 + pgap));
	uint maxch = (double)h/(ny*(1 + pgap));
	uint sz = (maxcw>maxch)?maxch:maxcw;
	cgap = pgap * sz;
	int cxoff = (w - (nx*sz + (nx-1)*cgap)) / 2;
	int cyoff = (h - (ny*sz + (ny-1)*cgap)) / 2;
	int x = cxoff, y = cyoff;

	/* layout cards */
	numCards = nx*ny - ((nx*ny)%matchsize);
	if (numCards > climit)
		numCards = climit;
	uint pos = 0;
	for (uint j = 0; j < ny && pos < numCards; j++) {
		for (uint i = 0; i < nx && pos < numCards; i++) {
			cards[pos++].setGeometry(x, y, sz, sz);
			x += sz + cgap;
		}
		y += sz + cgap;
		x = cxoff;
	}
	numCardsLeft = numCards;

	/* get pair ids (first numCards/2 are chosen from all) */
	random_device rd;
	mt19937 g(rd());
	vector<int> pids;
	for (uint i = 0; i < climit/matchsize; i++)
		pids.push_back(i);
	shuffle(pids.begin(), pids.end(), g);

	/* set card ids */
	vector<int> cids;
	for (uint j = 0; j < matchsize; j++)
		for (uint i = 0; i < numCards/matchsize; i++)
			cids.push_back(pids[i]);
	shuffle(cids.begin(), cids.end(), g);

	for (uint i = 0; i < numCards; i++)
		cards[i].set(cids[i]);

	numMaxOpenCards = matchsize;
	gameStarted = false;
	gameover = false;
	gtime = 0;
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

		/* quick closing cards is only allowed in solo game */
		if (numPlayers == 1 && numOpenCards == numMaxOpenCards) {
			ret |= closeCards();
			closeTimeout.clear();
		}

		if (cid != -1 && numOpenCards < numMaxOpenCards) {
			cards[cid].toggle(); /* open card */
			openCardIds[numOpenCards++] = cid;
			if (!gameStarted)
				gameStarted = true;
			ret |= GF_CARDOPENED;

			for (uint i = 0; i < numPlayers; i++)
				if (players[i].isCPU())
					players[i].setCPUMemoryCell(cid,
						getAdjacentCards(cid, NULL, false));
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
	if (cpuSelectTimeout.running())
		cpuSelectTimeout.update(ms);

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
			if (numCardsLeft == 0) {
				gameover = true;
				ret |= GF_GAMEOVER;
			}
		}
		getCurrentPlayer().incScore();
		ret |= GF_SCORECHANGED;
		ret |= GF_CARDSREMOVED;
	} else {
		if (checkError()) {
			getCurrentPlayer().incErrors();
			ret |= GF_ERRORSCHANGED;
		}
		for (uint i = 0; i < numOpenCards; i++)
			cards[openCardIds[i]].toggle();
		ret |= GF_CARDSCLOSED;

		/* set next player */
		if (numPlayers > 1) {
			curPlayer++;
			if (curPlayer >= numPlayers)
				curPlayer = 0;
			ret |= GF_NEXTPLAYER;
			/* as cpu wait with first until closing animation is done */
			if (players[curPlayer].isCPU())
				cpuSelectTimeout.set(500);
		}
	}
	numOpenCards = 0;
	isMatch = false;

	for (uint i = 0; i < numPlayers; i++)
		if (players[i].isCPU()) {
			for (uint j = 0; j < numCards; j++)
				players[i].reduceCPUMemoryCell(j,
						getAdjacentCards(j, NULL, false));
		}

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

/** Get all cards that are within cw+gap,ch+gap */
uint Game::getAdjacentCards(uint cid, vector<uint> *adjCards, bool onlyClosed)
{
	uint num = 0;

	if (adjCards)
		adjCards->clear();

	if (cid >= numCards)
		return 0;

	int cx = cards[cid].x;
	int cy = cards[cid].y;
	int dist = cards[cid].w + cgap; /* same for h as cards are square */

	for (uint i = 0; i < numCards; i++) {
		if (i == cid || cards[i].removed)
			continue;
		if (onlyClosed && cards[i].open)
			continue;
		if (abs(cx - cards[i].x) <= dist && abs(cy - cards[i].y) <= dist) {
			num++;
			if (adjCards)
				adjCards->push_back(i);
		}
	}
	return num;
}

/** Get next CPU click if select timeout not running.
 * FIXME should be done by player itself with access to game.
 */
void Game::getNextCPUClick(int &button, int &bx, int &by)
{
	button = 0;

	if (cpuSelectTimeout.running())
		return;

	uint pos = INVALIDCARDID;

	if (numOpenCards == 0) {
		/* select first card */
		pos = cpuFindBestKnownPairCard();
		if (pos == INVALIDCARDID)
			pos = cpuSelectRandomCard();
		else
			pos = cpuTryCard(pos);
		if (pos != INVALIDCARDID) {
			button = 1;
			bx = cards[pos].x + cards[pos].w/2;
			by = cards[pos].y + cards[pos].h/2;
			cpuSelectTimeout.set(1000);
		}
	} else if (numOpenCards < numMaxOpenCards){
		/* select next card */
		if ((pos = cpuFindKnownMatch(openCardIds[numOpenCards-1])) != INVALIDCARDID)
			pos = cpuTryCard(pos);
		else
			pos = cpuSelectRandomCard();
		if (pos != INVALIDCARDID) {
			button = 1;
			bx = cards[pos].x + cards[pos].w/2;
			by = cards[pos].y + cards[pos].h/2;
			cpuSelectTimeout.set(1000);
		}
	}
}

/** Find matching known card for card at cid.
 * Return id or INVALIDCARDID if not found.
 */
uint Game::cpuFindKnownMatch(uint cid)
{
	if (cid >= numCards || cards[cid].removed)
		return INVALIDCARDID;
	for (uint i = 0; i < numCards; i++) {
		if (cid == i)
			continue;
		if (cards[i].id != cards[cid].id)
			continue;
		if (getCurrentPlayer().cmem[i] > 0)
			return i;
	}
	return INVALIDCARDID;
}
/** Get weaker card of best known pair.
 * Return id or INVALIDCARDID if not found. */
uint Game::cpuFindBestKnownPairCard()
{
	double kp[MAXCARDS];
	uint match = INVALIDCARDID, mk = 0, mp=0;
	/* copy player's cmemory to kp */
	for (uint i = 0; i < numCards; i++)
		if (cards[i].removed)
			kp[i] = 0;
		else
			kp[i] = getCurrentPlayer().cmem[i];
	/* find best card of best pair */
	for (uint i = 0; i < numCards; i++) {
		if (kp[i] == 0)
			continue; /* no or unknown card */
		/* get matching card or drop position if none */
		if ((match = cpuFindKnownMatch(i)) == INVALIDCARDID) {
			kp[i] = 0; /* no valid choice */
			continue;
		}
		/* add higher chance to weaker card to try that card
		 * first to not give away position in case of failure */
		if (kp[match] < kp[i]) {
			kp[match] += kp[i];
			kp[i] = 0;
		} else {
			kp[i] += kp[match];
			kp[match] = 0;
		}
	}
	/* select best card */
	mk = 0;
	mp = INVALIDCARDID;
	for (uint i = 0; i < numCards; i++)
		if (kp[i] > mk) {
			mp = i;
			mk = kp[i];
		}
	return mp;
}
/** Return position of a random closed card but re-roll up to 5 times
 * if position has a known card.
 */
uint Game::cpuSelectRandomCard()
{
	uint rerolls = 0;
	uint pos = INVALIDCARDID;
	uint numClosedCards = 0;

	for (uint i = 0; i < numCards; i++)
		if (!cards[i].removed && !cards[i].open)
			numClosedCards++;

	if (numClosedCards == 0)
		return INVALIDCARDID; /* no more cards??? */

	while (pos == INVALIDCARDID) {
		/* get random position */
		pos = rand() % numClosedCards; /* pos in closed cards */
		for (uint i = 0; i < numCards; i++)
			if (!cards[i].removed && !cards[i].open)
				if (pos-- == 0) {
					pos = i; /* pos in all cards */
					break;
				}
		/* reroll if remembered as known */
		if (getCurrentPlayer().canRememberCard(pos) && rerolls<5) {
			pos = INVALIDCARDID;
			rerolls++;
		}
	}
	return pos;
}
/** Try to pick closed card with chance to miss. If successful return correct
 * position. If missed, reroll:
 *   If successful try adjacent card.
 *   Otherwise pick random card.
 * Return position or INVALIDCARDID.
 */
uint Game::cpuTryCard(uint pos)
{
	uint reroll = 0;

	if (pos >= numCards || cards[pos].removed || cards[pos].open)
		return INVALIDCARDID;

	/* return id if remembered properly */
	if (getCurrentPlayer().canRememberCard(pos))
		return pos;

	/* reroll up to 3 times for chance on adjacent card */
	vector<uint> ac;
	getAdjacentCards(pos, &ac, true);
	if (ac.size() == 0)
		pos = INVALIDCARDID;
	else if (getCurrentPlayer().canRememberCard(pos)) do {
		/* okay we have adjacent cards try one */
		pos = ac[rand() % ac.size()];
		/* go random if cpu remembers it's not the right one */
		if (getCurrentPlayer().canRememberCard(pos))
			pos = INVALIDCARDID;
		reroll++;
	} while (pos == INVALIDCARDID && reroll < 3);

	/* go for a random card? */
	if (pos == INVALIDCARDID)
		pos = cpuSelectRandomCard();

	return pos;
}
