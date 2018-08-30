/*
 * view.cpp
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
#include "clientgame.h"
#include "theme.h"
#include "sprite.h"
#include "view.h"

extern SDL_Renderer *mrc;

View::View(Config &cfg, ClientGame &_cg)
	: config(cfg), cgame(_cg), quitReceived(false),
	  fpsCycles(0), fpsStart(0), fps(0)
{
	_loginfo("Initializing SDL\n");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
			SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
	if (TTF_Init() < 0)
	 		SDL_Log("TTF_Init failed: %s\n", SDL_GetError());

	/* determine resolution and scale factor */
	int sw, sh;
	sh = config.resolution;
	sw = sh * 16 / 9;
	if (config.fullscreen) {
		/* use window height for resolution */
		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(0,&mode);
		sh = mode.h;
		sw = mode.w;
		_loginfo("Using fullscreen resolution %dx%d\n",mode.w,mode.h);
	} else
		_loginfo("Using window resolution %dx%d\n",sw,sh);
	brickAreaHeight = sh;
	brickAreaWidth = sh * VG_BRICKAREAWIDTH / VG_BRICKAREAHEIGHT;
	scaleFactor = brickAreaWidth * 100 / VG_BRICKAREAWIDTH;
	brickScreenWidth = v2s(VG_BRICKWIDTH);
	brickScreenHeight = v2s(VG_BRICKHEIGHT);
	_loginfo("Scale factor x100: %d\n",scaleFactor);
	_loginfo("Brick screen size: %dx%d\n",brickScreenWidth,brickScreenHeight);

	/* create main window */
	mw = new MainWindow("LBreakoutHD", sw, sh, config.fullscreen);

	/* load theme (scaled if necessary) */
	theme.load("Standard", sw, sh, brickScreenWidth, brickScreenHeight);
	weaponFrameCounter.init(theme.weaponFrameNum, theme.weaponAnimDelay);
	shotFrameCounter.init(theme.shotFrameNum, theme.shotAnimDelay);

	/* create render images and positions */
	int boardX = MAPWIDTH * brickScreenWidth;
	int boardWidth = sw - boardX - brickScreenWidth;
	imgBackground.create(sw,sh);
	imgBackground.setBlendMode(0);
	curWallpaperId = rand() % theme.numWallpapers;
	imgScore.create(brickScreenWidth*3, brickScreenHeight);
	imgScoreX = boardX + (boardWidth - imgScore.getWidth())/2;
	imgScoreY = brickScreenHeight * 15 + brickScreenHeight/2;
	imgBricks.create((MAPWIDTH-2)*brickScreenWidth,
				(MAPHEIGHT-1)*brickScreenHeight);
	imgBricksX = brickScreenWidth;
	imgBricksY = brickScreenHeight;
	imgExtras.create(boardWidth, 4*brickScreenHeight);
	imgExtrasX = boardX;
	imgExtrasY = 19*brickScreenHeight;
}

View::~View()
{
	delete mw;

	/* XXX fonts need to be killed before SDL/TTF_Quit otherwise they
	 * segfault but attribute's dtors are called after ~View is finished */
	theme.fSmall.free();
	theme.fNormal.free();

	_loginfo("Finalizing SDL\n");
	TTF_Quit();
	SDL_Quit();
}

/** Main game loop. Handle events, update game and render view. */
void View::run()
{
	SDL_Event ev;
	int flags;
	double px = 0;
	PaddleInputState pis;
	Uint32 ms, tsNow, tsLast;
	int oldmx = 0;

	tsNow = tsLast = SDL_GetTicks();
	fpsStart = SDL_GetTicks();

	renderBackgroundImage();
	renderBricksImage();
	renderScoreImage();
	render();

	SDL_ShowCursor(0);

	while (!quitReceived) {
		/* handle events */
		if (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				quitReceived = true;
			if (ev.type == SDL_KEYUP && ev.key.keysym.scancode == SDL_SCANCODE_P) {
				showInfo(_("Pause"));
				tsNow = tsLast = SDL_GetTicks();
			}
		}

		/* get paddle input state */
		pis.reset();
		/* mouse input */
		int mx, my;
		Uint32 buttons = SDL_GetMouseState(&mx,&my);
		if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
			pis.leftFire = 1;
		if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT))
			pis.rightFire = 1;
		if (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE))
			pis.speedUp = 1;
		if (mx != oldmx) {
			px = s2v(mx);
			oldmx = mx;
		} else
			px = 0; /* no mouse position input */
		/* key input */
		const Uint8 *keystate = SDL_GetKeyboardState(NULL);
		if (keystate[config.k_maxballspeed])
			pis.speedUp = 1;
		if (keystate[config.k_return])
			pis.recall = 1;
		if (keystate[config.k_left])
			pis.left = 1;
		if (keystate[config.k_right])
			pis.right = 1;
		if (keystate[config.k_lfire])
			pis.leftFire = 1;
		if (keystate[config.k_rfire])
			pis.rightFire = 1;
		if (keystate[config.k_turbo])
			pis.turbo = 1;

		/* get passed time */
		tsNow = SDL_GetTicks();
		ms = tsNow - tsLast;
		if (ms < 1)
			ms = 1;
		tsLast = tsNow;

		/* update animations and particles */
		shotFrameCounter.update(ms);
		weaponFrameCounter.update(ms);
		for (auto it = begin(sprites); it != end(sprites); ++it) {
			if ((*it).get()->update(ms))
				it = sprites.erase(it);
		}

		/* update game context */
		flags = cgame.update(ms, px, pis);
		if (flags & CGF_PLAYERMESSAGE)
			showInfo(cgame.getPlayerMessage());
		if (flags & CGF_GAMEOVER)
			break;
		if (flags & CGF_NEWLEVEL) {
			flags |= CGF_UPDATEBACKGROUND | CGF_UPDATEBRICKS |
					CGF_UPDATESCORE | CGF_UPDATEEXTRAS;
			curWallpaperId = rand() % theme.numWallpapers;
			dim();
		}
		if (flags & CGF_UPDATEBACKGROUND)
			renderBackgroundImage();
		if (flags & CGF_UPDATEBRICKS)
			renderBricksImage();
		if (flags & CGF_UPDATEEXTRAS)
			renderExtrasImage();
		if (flags & CGF_UPDATESCORE)
			renderScoreImage();
		if (flags & CGF_NEWANIMATIONS)
			createSprites();

		/* render */
		render();
		SDL_RenderPresent(mrc);
		if (config.fps)
			SDL_Delay(10);
		SDL_FlushEvent(SDL_MOUSEMOTION); /* prevent event loop from dying */

		/* stats */
		fpsCycles++;
		fps = 1000 * fpsCycles / (SDL_GetTicks() - fpsStart);
		if (fpsCycles > 100) {
			fpsCycles = 0;
			fpsStart = SDL_GetTicks();
		}
	}

	/* check hiscores */
	cgame.updateHiscores();
	/* TODO show final hiscore */
}

/** Render current game state. */
void View::render()
{
	Game *game = cgame.getGameContext(); /* direct lib game context */
	Paddle *paddle = game->paddles[0]; /* local paddle always at bottom */
	Ball *ball = 0;
	Extra *extra = 0;
	Shot *shot = 0;

	if (cgame.darknessActive()) {
		SDL_SetRenderDrawColor(mrc,0,0,0,255);
		SDL_RenderClear(mrc);
		theme.paddles.setAlpha(128);
		theme.shot.setAlpha(128);
		theme.weapon.setAlpha(128);
		theme.balls.setAlpha(128);
		theme.extras.setAlpha(128);
	} else {
		imgBackground.copy();
		imgBricks.copy(imgBricksX,imgBricksY);
		imgScore.copy(imgScoreX,imgScoreY);
		imgExtras.copy(imgExtrasX,imgExtrasY);
		theme.paddles.clearAlpha();
		theme.shot.clearAlpha();
		theme.weapon.clearAlpha();
		theme.balls.clearAlpha();
		theme.extras.clearAlpha();
	}

	/* balls - shadows */
	list_reset(game->balls);
	while ( ( ball = (Ball*)list_next( game->balls ) ) != 0 ) {
		uint type;
		int px, py;
		getBallViewInfo(ball, &px, &py, &type);
		theme.ballsShadow.copy(type, 0,
				px + theme.shadowOffset, py + theme.shadowOffset);
	}

	/* extras - shadows */
	list_reset(game->extras);
	while ( ( extra = (Extra*)list_next( game->extras) ) != 0 )
		theme.extrasShadow.copy(extra->type, 0,
					v2s(extra->x) + theme.shadowOffset,
					v2s(extra->y) + theme.shadowOffset);

	/* shots - shadows */
	list_reset(game->shots);
	while ( ( shot = (Shot*)list_next( game->shots) ) != 0 )
		theme.shotShadow.copy(shotFrameCounter.get(),0,
					v2s(shot->x) + theme.shadowOffset,
					v2s(shot->y) + theme.shadowOffset);

	/* paddle */
	if (!paddle->invis || paddle->invis_delay > 0) {
		int pid = 0;
		int poff = theme.paddles.getGridWidth();
		int ph = theme.paddles.getGridHeight();
		double px = v2s(paddle->x);
		double py = v2s(paddle->y);
		int pmw = v2s(paddle->w) - 2*poff;
		int pxr = px + poff + pmw; /* right end part x */
		if (paddle->frozen)
			pid = 2;
		else if (paddle->slime)
			pid = 1;
		if (paddle->invis || cgame.darknessActive())
			theme.paddles.setAlpha(128);
		else
			theme.paddles.clearAlpha();
		/* FIXME for paddle we do the shadow here as it's composed and
		 * otherwise difficult and quite at the bottom so chance for
		 * graphical errors is pretty low... yes I'm lazy... */
		theme.paddlesShadow.copy(0,pid,px+theme.shadowOffset,py+theme.shadowOffset);
		theme.paddles.copy(0,pid,px,py);
		for (px += poff; px < pxr - poff; px += poff) {
			theme.paddlesShadow.copy(1,pid,px+theme.shadowOffset,py+theme.shadowOffset);
			theme.paddles.copy(1,pid,px,py);
		}
		/* copy last middle part, potentially shortened */
		theme.paddlesShadow.copy(1,pid,0,0,pxr-px,ph,px+theme.shadowOffset,py+theme.shadowOffset);
		theme.paddles.copy(1,pid,0,0,pxr-px,ph,px,py);
		theme.paddlesShadow.copy(2,pid,pxr+theme.shadowOffset,py+theme.shadowOffset);
		theme.paddles.copy(2,pid,pxr,py);
	}
	if (paddle->weapon_inst) {
		int wx = v2s(paddle->x + paddle->w/2) -
						theme.weapon.getGridWidth()/2;
		int wy = v2s(paddle->y);
		theme.weapon.copy(weaponFrameCounter.get(),0,wx,wy);
	}

	/* balls */
	list_reset(game->balls);
	while ( ( ball = (Ball*)list_next( game->balls ) ) != 0 ) {
		uint type;
		int px, py;
		getBallViewInfo(ball, &px, &py, &type);
		theme.balls.copy(type,0,px,py);
	}

	/* shots */
	list_reset(game->shots);
	while ( ( shot = (Shot*)list_next( game->shots) ) != 0 )
		theme.shot.copy(shotFrameCounter.get(),0,v2s(shot->x),v2s(shot->y));

	/* sprites */
	for (auto& s : sprites)
		s->render();

	/* extras */
	list_reset(game->extras);
	while ( ( extra = (Extra*)list_next( game->extras) ) != 0 )
		theme.extras.copy(extra->type, 0, v2s(extra->x), v2s(extra->y));

	/* stats */
	theme.fSmall.setAlign(ALIGN_X_LEFT | ALIGN_Y_TOP);
	theme.fSmall.write(0,0,to_string((int)fps));
}

/** Take background image, add frame and static hiscore chart */
void View::renderBackgroundImage() {
	int bw = theme.bricks.getGridWidth();
	int bh = theme.bricks.getGridHeight();
	SDL_Texture *tex = imgBackground.getTex();

	SDL_SetRenderTarget(mrc,tex);
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

	/* wallpaper background */
	Image& wallpaper = theme.wallpapers[curWallpaperId];
	for (int wy = 0; wy < imgBackground.getHeight(); wy += wallpaper.getHeight())
		for (int wx = 0; wx < imgBackground.getWidth(); wx += wallpaper.getWidth())
			wallpaper.copy(wx,wy);

	/* frame */
	theme.frameShadow.copy(theme.shadowOffset,theme.shadowOffset);
	theme.frame.copy(0,0);

	/* title + current level */
	int tx = MAPWIDTH*bw, ty = bh;
	int tw = mw->getWidth() - tx - bw;
	theme.fNormal.setAlign(ALIGN_X_CENTER | ALIGN_Y_CENTER);
	theme.fSmall.setAlign(ALIGN_X_CENTER | ALIGN_Y_CENTER);
	theme.fNormal.write(tx+tw/2, ty + bh/2, cgame.getLevelsetName().c_str());
	theme.fSmall.write(tx + tw/2, ty + 3*bh/2,
			string(_("(Level ")) +
				to_string(cgame.getCurrentPlayer()->getLevel()+1)
				+ "/" + to_string(cgame.getLevelCount()) + ")");

	/* hiscores */
	renderHiscore(tx+bw/4, 3*bh, tw - bw/2, 10*bh);

	/* player name */
	theme.fNormal.setAlign(ALIGN_X_CENTER | ALIGN_Y_CENTER);
	theme.fNormal.write(tx+tw/2, 15*bh, cgame.getCurrentPlayer()->getName());

	/* active extras */
	theme.fNormal.setAlign(ALIGN_X_CENTER | ALIGN_Y_CENTER);
	theme.fNormal.write(tx+tw/2, 18*bh+bh/2, _("Active Extras"));

	/* lives */
	ClientPlayer *cp = cgame.getCurrentPlayer();
	theme.life.clearAlpha();
	for (uint i = 0; i < cp->getMaxLives(); i++) {
		if (i >= cp->getLives())
			theme.life.setAlpha(128);
		theme.life.copy(0, (MAPHEIGHT-i-1)*bh);
	}

	SDL_SetRenderTarget(mrc,NULL);
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
}

/* Render at given region. */
void View::renderHiscore(int x, int y, int w, int h)
{
	HiscoreChart *chart = cgame.getHiscoreChart();

	int cy = y + (h - theme.fNormal.getLineHeight() - 10*theme.fSmall.getLineHeight())/2;

	theme.fNormal.setAlign(ALIGN_X_CENTER | ALIGN_Y_TOP);
	theme.fNormal.write(x + w/2, cy, _("Hiscores"));
	cy += theme.fNormal.getLineHeight();

	string str;
	int cx_num = x;
	int cx_name = x + theme.fSmall.getCharWidth('0')*3;
	//int cx_level = x + rc.stdFont.getCharWidth('0')*10;
	int cx_score = x + w;

	for (int i = 0; i < CHARTSIZE; i++) {
		const ChartEntry *entry = chart->get(i);

		theme.fSmall.setAlign(ALIGN_X_LEFT | ALIGN_Y_TOP);

		/* number */
		str = to_string(i+1) + ".";
		theme.fSmall.write(cx_num, cy, str);
		/* name */
		theme.fSmall.write(cx_name, cy, entry->name);
		/* level */
		//rc.stdFont.write(img.getTex(), cx_level, cy, "L" + to_string(entry->level));
		/* score */
		theme.fSmall.setAlign(ALIGN_X_RIGHT | ALIGN_Y_TOP);
		theme.fSmall.write(cx_score, cy, to_string(entry->score));

		cy += theme.fSmall.getLineHeight();
	}
}

void View::renderBricksImage()
{
	Game *game = cgame.getGameContext();
	int bw = theme.bricks.getGridWidth();
	int bh = theme.bricks.getGridHeight();

	imgBricks.fill(0,0,0,0);
	SDL_SetRenderTarget(mrc, imgBricks.getTex());

	for (int i = 1; i < MAPWIDTH-1; i++)
		for (int j = 1; j < MAPHEIGHT; j++) {
			Brick *b = &game->bricks[i][j];
			if (b->type != MAP_EMPTY && b->id != INVIS_BRICK_ID)
				theme.bricksShadow.copy(b->id, 0, (i-1)*bw + theme.shadowOffset,
												(j-1)*bh + theme.shadowOffset);
		}
	for (int i = 1; i < MAPWIDTH-1; i++)
		for (int j = 1; j < MAPHEIGHT; j++) {
			Brick *b = &game->bricks[i][j];
			if (b->type != MAP_EMPTY && b->id != INVIS_BRICK_ID)
				theme.bricks.copy(b->id, 0, (i-1)*bw, (j-1)*bh);
		}

	SDL_SetRenderTarget(mrc, NULL);
}
void View::renderScoreImage()
{
	imgScore.fill(0,0,0,0);
	SDL_SetRenderTarget(mrc, imgScore.getTex());
	theme.fNormal.setAlign(ALIGN_X_CENTER | ALIGN_Y_CENTER);
	theme.fNormal.write(imgScore.getWidth()/2, imgScore.getHeight()/2,
				to_string(cgame.getCurrentPlayer()->getScore()));
	SDL_SetRenderTarget(mrc, NULL);
}
void View::renderExtrasImage()
{
	Game *game = cgame.getGameContext();
	uint bw = brickScreenWidth, bh = brickScreenHeight;
	uint xstart = (imgExtras.getWidth() - 3*bw) / 4;
	uint ystart = (imgExtras.getHeight() - 3*bh) / 4;
	uint xoff = bw + xstart;
	uint yoff = bh + ystart;
	uint x = xstart, y = ystart;

	SDL_SetRenderTarget(mrc, imgExtras.getTex());
	SDL_SetRenderDrawColor(mrc,0,0,0,0);
	SDL_RenderClear(mrc);

	theme.extras.setAlpha(192);
	/* energy/explosive/weak balls */
	if (game->extra_active[EX_METAL])
		renderActiveExtra(EX_METAL, game->extra_time[EX_METAL], x, y);
	else if (game->extra_active[EX_EXPL_BALL])
		renderActiveExtra(EX_EXPL_BALL, game->extra_time[EX_EXPL_BALL], x, y);
	else if (game->extra_active[EX_WEAK_BALL])
		renderActiveExtra(EX_WEAK_BALL, game->extra_time[EX_WEAK_BALL], x, y);
	x += xoff;
	/* slow/fast balls */
	if (game->extra_active[EX_SLOW])
		renderActiveExtra(EX_SLOW, game->extra_time[EX_SLOW], x, y);
	else if (game->extra_active[EX_FAST])
		renderActiveExtra(EX_FAST, game->extra_time[EX_FAST], x, y);
	x += xoff;
	/* chaotic */
	if (game->extra_active[EX_CHAOS])
		renderActiveExtra(EX_CHAOS, game->extra_time[EX_CHAOS], x, y);
	x = xstart;
	y += yoff;
	/* sticky paddle */
	if (game->paddles[0]->extra_active[EX_SLIME])
		renderActiveExtra(EX_SLIME, game->paddles[0]->extra_time[EX_SLIME], x, y);
	x += xoff;
	/* weapon - paddle */
	if (game->paddles[0]->extra_active[EX_WEAPON])
		renderActiveExtra(EX_WEAPON, game->paddles[0]->extra_time[EX_WEAPON], x, y);
	x += xoff;
	/* goldshower - paddle */
	if (game->paddles[0]->extra_active[EX_GOLDSHOWER])
		renderActiveExtra(EX_GOLDSHOWER, game->paddles[0]->extra_time[EX_GOLDSHOWER], x, y);
	x = xstart;
	y += yoff;
	/* wall - paddle */
	if (game->paddles[0]->extra_active[EX_WALL])
		renderActiveExtra(EX_WALL, game->paddles[0]->extra_time[EX_WALL], x, y);
	x += xoff;
	/* magnet - paddle */
	if (game->paddles[0]->extra_active[EX_BONUS_MAGNET])
		renderActiveExtra(EX_BONUS_MAGNET, game->paddles[0]->extra_time[EX_BONUS_MAGNET], x, y);
	else if (game->paddles[0]->extra_active[EX_MALUS_MAGNET])
		renderActiveExtra(EX_MALUS_MAGNET, game->paddles[0]->extra_time[EX_MALUS_MAGNET], x, y);
	x += xoff;
	/* ghost - paddle */
	if (game->paddles[0]->extra_active[EX_GHOST_PADDLE])
		renderActiveExtra(EX_GHOST_PADDLE, game->paddles[0]->extra_time[EX_GHOST_PADDLE], x, y);

	theme.extras.clearAlpha();
	SDL_SetRenderTarget(mrc, NULL);
}

/** Render to current target: extra image and time in seconds */
void View::renderActiveExtra(int id, int ms, int x, int y)
{
	SDL_Rect drect = {x,y,(int)brickScreenWidth,(int)brickScreenHeight};
	SDL_RenderFillRect(mrc,&drect);
	theme.extras.copy(id,0,x,y);
	theme.fSmall.setAlign(ALIGN_X_CENTER | ALIGN_Y_CENTER);
	theme.fSmall.write(x + brickScreenWidth/2, y + brickScreenHeight/2,
				to_string(ms/1000+1));
}

/* Dim screen to black. As game engine is already set for new state
 * we cannot use render() but need current screen state. */
void View::dim()
{
	Image img;
	img.createFromScreen();

	for (uint8_t a = 250; a > 0; a -= 10) {
		SDL_SetRenderDrawColor(mrc,0,0,0,255);
		SDL_RenderClear(mrc);
		img.setAlpha(a);
		img.copy();
		SDL_RenderPresent(mrc);
		SDL_Delay(10);
	}
	SDL_FlushEvents(SDL_FIRSTEVENT,SDL_LASTEVENT);
}

/* Darken current screen content and show info text.
 * Wait for any key/click. */
void View::showInfo(const string& str)
{
	SDL_Event ev;
	Image img;
	bool leave = false;

	img.createFromScreen();
	SDL_SetRenderDrawColor(mrc,0,0,0,255);
	SDL_RenderClear(mrc);
	img.setAlpha(64);
	img.copy();

	theme.fNormal.setAlign(ALIGN_X_CENTER | ALIGN_Y_CENTER);
	theme.fNormal.write(mw->getWidth()/2,mw->getHeight()/2,str);

	SDL_RenderPresent(mrc);

	SDL_FlushEvents(SDL_FIRSTEVENT,SDL_LASTEVENT);
	while (!leave) {
		/* handle events */
		if (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				quitReceived = leave = true;
			if (ev.type == SDL_KEYUP || ev.type == SDL_MOUSEBUTTONUP)
				leave = true;
		}
		SDL_Delay(10);
	}
	SDL_FlushEvents(SDL_FIRSTEVENT,SDL_LASTEVENT);
}

/* Create particles for brick hit of type HT_REMOVE (already checked). */
void View::createParticles(BrickHit *hit)
{
	double vx, vy;
	Particle *p;
	int pw, ph;
	int bx = hit->x*brickScreenWidth, by = hit->y*brickScreenHeight;

	switch (hit->dest_type) {
	case SHR_BY_NORMAL_BALL:
		vx = cos( 6.28 * hit->degrees / 180);
		vy = sin( 6.28 * hit->degrees / 180 );
		p = new Particle(theme.bricks, hit->brick_id, 0, 0, 0,
					brickScreenWidth, brickScreenHeight,
					bx, by, vx, vy, v2s(0.13), 500);
		sprites.push_back(unique_ptr<Particle>(p));
		break;
	case SHR_BY_ENERGY_BALL:
		pw = brickScreenWidth/2;
		ph = brickScreenHeight/2;
		for ( int i = 0, sx = 0; i < 2; i++, sx+=pw ) {
			for ( int j = 0, sy = 0; j < 2; j++, sy+=ph ) {
				vx = pw - (sx + pw/2);
				vy = ph - (sy + ph/2);
				p = new Particle(theme.bricks, hit->brick_id, 0,
						sx, sy, pw, ph,
						bx+sx, by+sy,
						vx, vy, v2s(0.02), 500);
				sprites.push_back(unique_ptr<Particle>(p));
			}
		}
		break;
	case SHR_BY_SHOT:
		/* FIXME this will cause artifacts on 768p better start
		 * creating from the middle moving outwards */
		pw = brickScreenWidth / 10;
		for ( int i = 0, sx = 0; i < 5; i++, sx += pw ) {
			p = new Particle(theme.bricks, hit->brick_id, 0,
						sx, 0, pw, brickScreenHeight,
						bx+sx, by,
						0, -1, v2s(0.006*i+0.002), 500);
			sprites.push_back(unique_ptr<Particle>(p));
			p = new Particle(theme.bricks, hit->brick_id, 0,
						brickScreenWidth - sx - pw,
						0, pw, brickScreenHeight,
						bx+brickScreenWidth - sx - pw, by,
						0, -1, v2s(0.006*i+0.002), 500);
			sprites.push_back(unique_ptr<Particle>(p));
		}
		break;
	case SHR_BY_EXPL:
		pw = brickScreenWidth/10;
		ph = brickScreenHeight/5;
		for ( int i = 0, sx=0; i < 10; i++, sx += pw )
			for ( int j = 0, sy=0; j < 5; j++, sy += ph ) {
				vx = 0.5 - 0.01*(rand()%100);
				vy = 0.5 - 0.01*(rand()%100);
				p = new Particle(theme.bricks, hit->brick_id, 0,
						sx, sy, pw, ph, bx+sx, by+sy,
						vx, vy, 0.01*(rand()%10+5), 1000);
				sprites.push_back(unique_ptr<Particle>(p));
			}
		break;
	}
}

/* Create new explosions, particles, etc according to game mods */
void View::createSprites()
{
	Game *game = cgame.getGameContext();
	BrickHit *hit;

	for (int i = 0; i < game->mod.brick_hit_count; i++) {
		hit = &game->mod.brick_hits[i];

		/* brick hit animation */
		if (hit->type == HT_REMOVE)
			createParticles(hit);
		/* explosion */
		if (hit->draw_explosion) {
			int x = hit->x * brickScreenWidth + brickScreenWidth/2 -
										theme.explosions.getGridWidth()/2;
			int y = hit->y * brickScreenHeight + brickScreenHeight/2 -
										theme.explosions.getGridHeight()/2;
			sprites.push_back(unique_ptr<Animation>(
				new Animation(theme.explosions,
					rand() % theme.explosions.getGridSizeY(),
					theme.explAnimDelay, x, y)));
		}

	}
}

void View::getBallViewInfo(Ball *ball, int *x, int *y, uint *type)
{
	Game *game = cgame.getGameContext(); /* direct lib game context */
	Paddle *paddle = game->paddles[0]; /* local paddle always at bottom */

	uint bt = 0;
	double px = ball->cur.x;
	double py = ball->cur.y;
	if (ball->attached) {
		px += paddle->x;
		py += paddle->y;
	}
	px = v2s(px);
	py = v2s(py);
	if (game->extra_active[EX_METAL])
		bt = 1;
	else if (game->extra_active[EX_EXPL_BALL])
		bt = 2;
	else if (game->extra_active[EX_WEAK_BALL])
		bt = 3;
	else if (game->extra_active[EX_CHAOS])
		bt = 4;

	*x = px;
	*y = py;
	*type = bt;
}

