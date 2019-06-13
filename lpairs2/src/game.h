/*
 * game.h
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

#ifndef GAME_H_
#define GAME_H_

class View;
class Game;

enum {
	MAXCARDS = 100,
	MAXOPENCARDS = 4,

	GM_SMALL = 0,
	GM_MEDIUM,
	GM_LARGE,
	GM_HUGE,

	GF_NONE = 0,
	GF_SCORECHANGED = 1,
	GF_ERRORSCHANGED = 2,
	GF_TIMECHANGED = 4,
	GF_CARDSCLOSED = 8,
	GF_CARDSREMOVED = 16,
	GF_CARDOPENED = 32
};

class Card {
	friend View;
	friend Game;

	uint id; /* theme id of card */
	int x,y,w,h; /* position and size */
	bool open;
	bool known;
	bool removed;
public:
	Card() : id(0), x(0), y(0), w(0), h(0),
			open(false), known(false), removed(false) {}
	void set(int i) {
		id = i;
		open = false;
		known = false;
		removed = false;
	}
	void setGeometry(int _x, int _y, int _w, int _h) {
		x = _x;
		y = _y;
		w = _w;
		h = _h;
	}
	bool hasFocus(int cx, int cy) {
		if (removed)
			return false;
		return (cx >= x && cx < x + w && cy >= y && cy < y + h);
	}
	bool isClosed()	{
		return !open;
	}
	bool isKnown() {
		return known;
	}
	bool toggle() {
		open = !open;
		if (!open)
			known = true; /* mark as known when closed again */
		return open;
	}
	void clear() {
		removed = true;
	}
};

class Game {
	friend View;

	uint numMaxOpenCards;
	Card cards[MAXCARDS];
	uint numCards;
	uint numCardsLeft;
	uint openCardIds[MAXOPENCARDS];
	uint numOpenCards;
	uint closedCardIds[MAXOPENCARDS];
	Timeout closeTimeout;
	bool isMatch;

	bool gameStarted; /* first click happened? */
	bool gameover;
	uint gtime; /* gaming time in ms */
	int score; /* number of pairs collected */
	int errors; /* misclicks of known cards */

	/* auto click new card after closing cards
	 * to save one opening click */
	bool autoClick;
	int autoClickX, autoClickY;
	Timeout autoclickTimeout;

	int closeCards();
public:
	Game() : numMaxOpenCards(2), numCards(0), numCardsLeft(0),
			numOpenCards(0), isMatch(false),
			gameStarted(false), gameover(false),
			gtime(0), score(0), errors(0),
			autoClick(false), autoClickX(-1), autoClickY(-1) {}
	void init(uint w, uint h, int mode, int fscreen, uint climit);
	int update(uint ms, int button, int bx, int by);
	int handleClick(int cx, int cy);
};

#endif /* GAME_H_ */
