/*
 * clientgame.cpp
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

using namespace std;

#include "tools.h"
#include "clientgame.h"

extern GameDiff diffs[DIFF_COUNT];

ClientGame::ClientGame(Config &cfg) : config(cfg), levelset(0), game(0),
		curPlayer(0), msg("")
{
}

ClientGame::~ClientGame()
{
	if (levelset)
		levelset_delete(&levelset);
	if (game)
		game_delete(&game);
}

int ClientGame::init(const string& setname)
{
	/* kill running game if any */
	if (levelset)
		levelset_delete(&levelset);
	if (game)
		game_delete(&game);

	/* init players */
	curPlayer = 0;
	players.clear();
	for (int i = 0; i < config.player_count; i++)
		players.push_back(
			unique_ptr<ClientPlayer>(
				new ClientPlayer(config.player_names[i],
						diffs[config.diff].lives,
						diffs[config.diff].max_lives)));

	/* load levelset from install directory or home directory */
	if ((levelset = levelset_load(setname.c_str(), 0)) == 0) {
		_logerr("Could not load levelset %s\n",setname.c_str());
		return -1;
	}
	if (levelset->count == 0) {
		_logerr("Levelset %s is empty\n",setname.c_str());
		return -1;
	}
	/* create game context and init first level */
	if ((game = game_create(GT_LOCAL,config.diff,config.rel_warp_limit)) == 0) {
		_logerr("Could not create game context\n");
		return -1;
	}
	game_set_current(game);
	game_init(game,levelset->levels[0]);
	game_set_convex_paddle( config.convex );
	game_set_ball_auto_return( !config.return_on_click );
	game_set_ball_random_angle( config.random_angle );
	game_set_ball_accelerated_speed( config.maxballspeed_float );

	/* set first level as snapshot to all players */
	for (auto& p : players)
		p->setLevelSnapshot(levelset->levels[0]);
	return 0;
}

/** Cycle through list to get next player or NULL if none (=game over) */
ClientPlayer *ClientGame::getNextPlayer()
{
	ClientPlayer *p = NULL;
	uint startId = curPlayer;

	do {
		curPlayer++;
		if (curPlayer == players.size())
			curPlayer = 0;
		p = players[curPlayer].get();
		if (p->getLives() > 0 && p->getLevel() < (uint)levelset->count)
			return p;
	} while (curPlayer != startId);
	return NULL;
}

/** ms is passed milliseconds since last call
 * px is wanted paddle center position scaled to 640 virtual width
 * 	0 (impossible) means no (mouse) input position for this cycle
 * pis is what controls have been activated
 * return flags what has to be rendered new
 */
int ClientGame::update(uint ms, int px, PaddleInputState &pis)
{
	int oldScore = game->paddles[0]->score;
	int oldWallActive = game->paddles[0]->extra_active[EX_WALL];
	int ret = 0;

	/* reset old modifications (View needs current mods afterwards) */
	game_reset_mods();

	/* as long as any extra is active render active extras
	 * FIXME: only a limited set of extras is relevant so this can
	 * be checked way more sufficiently...
	 */
	for (int i = 0; i < EX_NUMBER; i++)
		if (game->extra_active[i]) {
			ret |= CGF_UPDATEEXTRAS;
			break;
		}
	if (!(ret & CGF_UPDATEEXTRAS))
		for (int i = 0; i < EX_NUMBER; i++)
			if (game->paddles[0]->extra_active[i]) {
				ret |= CGF_UPDATEEXTRAS;
				break;
			}

	/* handle paddle movement */
	if (game->paddles[0]->frozen)
		px = 0;
	else if (pis.left || pis.right) {
		if (pis.left)
			px = game->paddles[0]->cur_x - config.key_speed *
							(ms << pis.turbo);
		if (pis.right)
			px = game->paddles[0]->cur_x + config.key_speed *
							(ms << pis.turbo);
	} else if (px != 0) {
		/* convert center px into left paddle starting position */
		px = px - game->paddles[0]->w/2;
	}

	/* update bottom paddle state */
	game_set_paddle_state(0,px,0,pis.leftFire,pis.rightFire,pis.recall);

	/* recall idle balls */
	game->paddles[0]->ball_return_key_pressed = pis.recall;

	/* update all game objects */
	game->paddles[0]->maxballspeed_request = pis.speedUp;
	game_update(ms);
	game->paddles[0]->maxballspeed_request_old = pis.speedUp;

	/* switch level/player? */
	if (game->level_over) {
		ClientPlayer *p = players[curPlayer].get();
		/* bonus levels are just skipped on failure */
		if (game->winner == PADDLE_BOTTOM || game->level_type != LT_NORMAL) {
			if (p->nextLevel() < (uint)levelset->count)
				p->setLevelSnapshot(levelset->levels[p->getLevel()]);
			else {
				strprintf(msg,_("Congratulations, %s, you cleared all levels!"),p->getName().c_str());
				ret |= CGF_PLAYERMESSAGE;
			}
		} else {
			p->setLevelSnapshot(NULL);
			if (p->looseLife() == 0) {
				strprintf(msg,_("Game over, %s!"), p->getName().c_str());
				ret |= CGF_PLAYERMESSAGE;
			}
		}
		p = getNextPlayer();
		if (p == NULL) {
			_logdebug(1,"Game over!\n");
			ret |= CGF_GAMEOVER;
			return ret;
		}
		_logdebug(1,"Next player: %s\n",p->getName().c_str());
		game_finalize(game);
		game_init(game,p->getLevelSnapshot());
		/* score is reset to 0 again so adjust */
		game->paddles[0]->score = p->getScore();
		ret |= CGF_NEWLEVEL;
		return ret;
	}

	/* handle (some) collected extras (most is done in game itself) */
	for (int i = 0; i < game->mod.collected_extra_count[0]; i++) {
		switch (game->mod.collected_extras[0][i]) {
		case EX_LIFE:
			players[curPlayer]->gainLife();
			ret |= CGF_UPDATEBACKGROUND; /* life is on the frame */
		break;
		}
	}
	if (oldWallActive != game->paddles[0]->extra_active[EX_WALL])
		ret |= CGF_UPDATEBRICKS;

	/* handle other modifications */
	if (game->mod.brick_hit_count > 0)
		ret |= CGF_UPDATEBRICKS | CGF_NEWANIMATIONS;
	if (game->paddles[0]->score != oldScore) {
		players[curPlayer]->setScore(game->paddles[0]->score);
		ret |= CGF_UPDATESCORE;
	}
	return ret;
}

void ClientGame::updateHiscores()
{
	HiscoreChart *hs = hiscores.get(levelset->name);
	for (auto& p : players)
		hs->add(p->getName(), p->getLevel(), p->getScore());
}
