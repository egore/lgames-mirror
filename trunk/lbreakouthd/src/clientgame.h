/*
 * clientgame.h
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

#ifndef SRC_CLIENTGAME_H_
#define SRC_CLIENTGAME_H_

#include "../libgame/gamedefs.h"
#include "../libgame/game.h"
#include "hiscores.h"

class ClientPlayer {
	string name;
	uint lives;
	uint maxLives;
	int score;
	uint level;
	Level snapshot;
public:
	ClientPlayer(const string &n, uint l, uint ml) :
			name(n), lives(l), maxLives(ml), score(0), level(0) {
		_loginfo("Added player %s\n",n.c_str());
	};
	const string& getName() { return name; }
	uint getLives() { return lives; }
	uint getMaxLives() { return maxLives; }
	void gainLife() {
		if (lives < maxLives)
			lives++;
	}
	int looseLife() {
		if (lives > 0)
			lives--;
		return lives;
	}
	int getScore() { return score; }
	int addScore(int s) {
		score += s;
		return score;
	}
	void setScore(int s) { score = s; }
	uint getLevel() { return level; }
	uint nextLevel() { return ++level; }
	void setLevel(uint l) { level = l; }
	void setLevelSnapshot(const Level *l) {
		if (l == NULL)
			game_get_level_snapshot(&snapshot);
		else
			snapshot = *l;
	}
	Level *getLevelSnapshot() { return &snapshot; }
};

class PaddleInputState {
public:
	int left = 0;
	int right = 0;
	int turbo = 0;
	int leftFire = 0;
	int rightFire = 0;
	int speedUp = 0;
	int recall = 0;

	void reset() {
		left = right = turbo = leftFire = rightFire = speedUp = recall = 0;
	}
};

enum {
	CGF_NONE = 0,
	CGF_UPDATEBACKGROUND = 1,
	CGF_UPDATEBRICKS = 2,
	CGF_UPDATESCORE = 4,
	CGF_UPDATEEXTRAS = 8,
	CGF_NEWLEVEL = 16,
	CGF_GAMEOVER = 32,
	CGF_PLAYERMESSAGE = 64
};
class ClientGame {
	Config &config;
	LevelSet *levelset;
	Game *game; /* current game context */
	Hiscores hiscores;
	vector<unique_ptr<ClientPlayer>> players;
	uint curPlayer;
	string msg;

	ClientPlayer *getNextPlayer();
	void initLevel(Level *l);
public:
	ClientGame(Config &cfg);
	~ClientGame();
	int init(const string& setname);
	int update(uint ms, int px, PaddleInputState &pis);
	Game *getGameContext() { return game; }
	string getLevelsetName() { return levelset->name; }
	int getCurrentLevelId() { return levelset->cur_level; }
	int getLevelCount() { return levelset->count; }
	HiscoreChart *getHiscoreChart() { return hiscores.get(levelset->name); }
	ClientPlayer* getCurrentPlayer() {return players[curPlayer].get(); }
	int darknessActive() { return game->extra_active[EX_DARKNESS]; }
	void updateHiscores();
	const string& getPlayerMessage() { return msg; }
};

#endif /* SRC_CLIENTGAME_H_ */
