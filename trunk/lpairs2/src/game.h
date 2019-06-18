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
	MAXPLAYERNUM = 2,

	INVALIDCARDID = MAXCARDS,

	GM_SOLO = 0,
	GM_VSCPU,
	GM_VSHUMAN,

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
	GF_CARDOPENED = 32,
	GF_NEXTPLAYER = 64,
	GF_GAMEOVER = 128,

	PC_HUMAN = 0,
	PC_CPU
};

class Player {
	friend View;
	friend Game;

	string name;
	uint score; /* number of pairs collected */
	uint errors; /* misclicks of known cards */
	uint control;

	/* CPU memory settings */
	double cmMinThreshold;
	double cmInitKnown;
	double cmInitLossPerAdjacentCard;
	double cmTurnLoss;
	double cmTurnSavePerMissingCard;
	double cmem[MAXCARDS];
public:
	void init(const string &n, uint ctrl) {
		name = n;
		score = 0;
		errors = 0;
		control = ctrl;

		cmMinThreshold = 0.1;
		cmInitKnown = 1.2;
		cmInitLossPerAdjacentCard = 0.05;
		cmTurnLoss = 0.1;
		cmTurnSavePerMissingCard = 0.012;
		for (uint i = 0; i < MAXCARDS; i++)
			cmem[i] = 0;
	}
	void incScore() {
		score++;
	}
	void incErrors() {
		errors++;
	}
	uint getScore() {
		return score;
	}
	uint getErrors() {
		return errors;
	}
	const string &getName() {
		return name;
	}

	bool isCPU() {
		return control == PC_CPU;
	}
	void setCPUMemoryCell(uint cid, uint numAdjCards) {
		if (cid >= MAXCARDS)
			return;
		cmem[cid] = cmInitKnown - cmInitLossPerAdjacentCard*numAdjCards;
	}
	void reduceCPUMemoryCell(uint cid, uint numAdjCards) {
		if (cid >= MAXCARDS)
			return;
		cmem[cid] -= cmTurnLoss - cmTurnSavePerMissingCard*(8-numAdjCards);
		if (cmem[cid] <= cmMinThreshold)
			cmem[cid] = 0;
	}
	bool canRememberCard(uint cid) {
		if (cid >= MAXCARDS)
			return false;
		return (rand() % 1000) < (cmem[cid]*1000);
	}
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

	Config &config;

	Player players[MAXPLAYERNUM];
	uint numPlayers;
	uint curPlayer;

	bool gameStarted; /* first click happened? */
	bool gameover;
	uint gtime; /* gaming time in ms */

	Card cards[MAXCARDS];
	uint numCards;
	uint numCardsLeft;
	uint numMaxOpenCards;
	uint openCardIds[MAXOPENCARDS];
	uint numOpenCards;
	uint closedCardIds[MAXOPENCARDS];
	Timeout closeTimeout;
	Timeout cpuSelectTimeout;
	uint cgap; /* gap between two cards */
	bool isMatch;

	int closeCards();
	bool checkError();

	uint cpuFindKnownMatch(uint cid);
	uint cpuFindBestKnownPairCard();
	uint cpuSelectRandomCard();
	uint cpuTryCard(uint cid);
public:
	Game(Config &cfg) : config(cfg), numPlayers(1), curPlayer(0),
			gameStarted(false), gameover(false), gtime(0),
			numCards(0), numCardsLeft(0), numMaxOpenCards(2),
			numOpenCards(0), cgap(0), isMatch(false) {}
	void init(uint w, uint h, uint mode, uint setsize,
				uint matchsize, int fscreen, uint climit);
	int update(uint ms, int button, int bx, int by);
	int handleClick(int cx, int cy);
	Player &getCurrentPlayer() {
		return players[curPlayer];
	}
	uint getAdjacentCards(uint cid, vector<uint> *adjCards);
	void getNextCPUClick(int &button, int &bx, int &by);
};

#endif /* GAME_H_ */
