/*
 * theme.cpp
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
#include "mixer.h"
#include "clientgame.h"
#include "theme.h"

extern SDL_Renderer *mrc;

/** Load resources and scale if necessary using bricks screen height.
 * Whatever is missing: Fall back to Standard theme. */
void Theme::load(string name, uint screenWidth, uint screenHeight,
				uint brickScreenWidth, uint brickScreenHeight)
{
	string path;
	bool oldTheme = true; // needed for some workarounds

	if (name[0] == '~')
		path = string(getenv( "HOME" )?getenv( "HOME" ):".") +
				"/" + CONFIGDIR + "/themes/" + name.substr(1);
	else
		path = string(DATADIR) + "/themes/" + name;

	if (!fileExists(path))
		_logerr("CRITICAL ERROR: theme %s not found. I will continue but most likely crash...\n",path.c_str());
	_loginfo("Loading theme %s\n",path.c_str());

	/* set default config values for old themes */
	title = "unknown";
	author = "unknown";
	version= "v?.??";
	brickFileWidth = 40;
	brickFileHeight = 20;
	shadowOffset = 10;
	fontColorNormal = {255,255,255,255};
	fontColorHighlight = {255,220,0,255};
	shotFrameNum = 4;
	shotAnimDelay = 200;
	weaponFrameNum = 4;
	weaponAnimDelay = 200;
	explFrameNum = 9;
	explAnimDelay = 50;
	fontSmallName = "fsmall.otf";
	fontSmallSize = 70; /* percentage of brick height */
	fontNormalName = "fnormal.otf";
	fontNormalSize = 90;
	if (fileExists(path + "/theme.ini")) {
		oldTheme = false;
		FileParser fp(path + "/theme.ini");
		fp.get("title",title);
		fp.get("author",author);
		fp.get("version",version);
		fp.get("brickWidth",brickFileWidth);
		fp.get("brickHeight",brickFileHeight);
		fp.get("shadowOffset",shadowOffset);
		fp.get("fontSmall.name",fontSmallName);
		fp.get("fontSmall.size",fontSmallSize);
		fp.get("fontNormal.name",fontNormalName);
		fp.get("fontNormal.size",fontNormalSize);
		fp.get("fontColorNormal.r",fontColorNormal.r);
		fp.get("fontColorNormal.g",fontColorNormal.g);
		fp.get("fontColorNormal.b",fontColorNormal.b);
		fp.get("fontColorNormal.a",fontColorNormal.a);
		fp.get("fontColorHighlight.r",fontColorHighlight.r);
		fp.get("fontColorHighlight.g",fontColorHighlight.g);
		fp.get("fontColorHighlight.b",fontColorHighlight.b);
		fp.get("fontColorHighlight.a",fontColorHighlight.a);
		fp.get("shotAnim.frames",shotFrameNum);
		fp.get("shotAnim.delay",shotAnimDelay);
		fp.get("weaponAnim.frames",weaponFrameNum);
		fp.get("weaponAnim.delay",weaponAnimDelay);
		fp.get("explAnim.frames",explFrameNum);
		fp.get("explAnim.delay",explAnimDelay);
	}

	if (oldTheme) {
		Image::setRenderScaleQuality(0);
		Image::useColorKeyBlack = true;
	} else {
		Image::setRenderScaleQuality(1);
		Image::useColorKeyBlack = false;
	}

	/* adjust shadow offset to screen geometry */
	shadowOffset = shadowOffset * brickScreenHeight / brickFileHeight;

	/* bricks and extras have exactly brick size */
	if (fileExists(path + "/bricks.png")) {
		bricks.load(path + "/bricks.png",brickFileWidth,brickFileHeight);
		bricks.scale(brickScreenWidth,brickScreenHeight);
	}
	if (fileExists(path + "/extras.png")) {
		if (oldTheme && (Image::getWidth(path + "/extras.png") & 1)) {
			/* last extra line is for color key */
			SDL_Surface *surf = IMG_Load(string(path + "/extras.png").c_str());
			if (surf) {
				Uint32 ckey = Image::getSurfacePixel(surf,surf->w-1,0);
				SDL_SetColorKey(surf,SDL_TRUE,ckey);
				extras.load(surf, brickFileWidth, brickFileHeight);
				SDL_FreeSurface(surf);
			}
		} else
			extras.load(path + "/extras.png",brickFileWidth,brickFileHeight);
		extras.scale(brickScreenWidth,brickScreenHeight);
	}

	/* load and scale up to 10 wallpapers */
	if (fileExists(path + "/back0.png") || fileExists(path + "/back0.jpg")) {
		numWallpapers = 0;
		for (int i = 0; i < MAXWALLPAPERS; i++) {
			string wfname = path + "/back" + to_string(i);
			if (fileExists(wfname + ".png"))
				wfname += ".png";
			else if (fileExists(wfname + ".jpg"))
				wfname += ".jpg";
			else
				break;
			wallpapers[i].load(wfname);
			if (brickFileHeight != brickScreenHeight) {
				int nw, nh;
				nw = wallpapers[i].getWidth() * brickScreenWidth / brickFileWidth;
				nh = wallpapers[i].getHeight() * brickScreenHeight / brickFileHeight;
				wallpapers[i].scale(nw, nh);
			}
			wallpapers[i].setBlendMode(0);
			numWallpapers++;
		}
	}

	/* load frame or create for old themes */
	if (fileExists(path + "/frame.png")) {
		frame.load(path + "/frame.png");
		if (brickFileHeight != brickScreenHeight) {
			int nw = frame.getWidth() * brickScreenWidth / brickFileWidth;
			int nh = frame.getHeight() * brickScreenHeight / brickFileHeight;
			frame.scale(nw, nh);
		}
	} else if (fileExists(path + "/fr_left.png")) {
		frame.create(screenWidth,screenHeight);
		frame.fill(0,0,0,0);

		/* we need to set fr_right:w-1,0 as colorkey so we have to use
		 * surfaces here... */
		SDL_Surface *sfleft = IMG_Load(string(path + "/fr_left.png").c_str());
		SDL_Surface *sfright = IMG_Load(string(path + "/fr_right.png").c_str());
		SDL_Surface *sftop = IMG_Load(string(path + "/fr_top.png").c_str());
		if (sfleft == NULL || sfright == NULL || sftop == NULL)
			_logsdlerr();
		else {
			/* convert all surfaces to same pixel format so ckey matches */
			SDL_Surface *oldsurf = sfleft;
			sfleft = SDL_ConvertSurface(sfleft,sfright->format,0);
			SDL_FreeSurface(oldsurf);
			oldsurf = sftop;
			sftop = SDL_ConvertSurface(sftop,sfright->format,0);
			SDL_FreeSurface(oldsurf);

			Uint32 ckey = Image::getSurfacePixel(sfright, sfright->w - 1, 0 );
			SDL_SetColorKey( sfleft, SDL_TRUE, ckey );
			SDL_SetColorKey( sftop, SDL_TRUE, ckey );
			SDL_SetColorKey( sfright, SDL_TRUE, ckey );
			Image fleft, ftop, fright;
			fleft.load(sfleft);
			fright.load(sfright);
			ftop.load(sftop);

			SDL_Texture *oldTex = SDL_GetRenderTarget(mrc);
			SDL_SetRenderTarget(mrc,frame.getTex());

			fleft.copy(0,0,brickScreenWidth,screenHeight);
			ftop.copy(brickScreenWidth,0,(MAPWIDTH-2)*brickScreenWidth,brickScreenHeight);
			fright.copy((MAPWIDTH-1)*brickScreenWidth,0,brickScreenWidth,screenHeight);

			int boardWidth = screenWidth - (MAPWIDTH+1)*brickScreenWidth;
			int boardX = MAPWIDTH*brickScreenWidth;
			addBox(frame, boardX, brickScreenHeight, boardWidth,
							12*brickScreenHeight);
			addBox(frame, boardX, brickScreenHeight*14, boardWidth,
							3*brickScreenHeight);
			addBox(frame, boardX, brickScreenHeight*18, boardWidth,
							5*brickScreenHeight);

			SDL_SetRenderTarget(mrc,oldTex);
		}
		if (sfleft)
			SDL_FreeSurface(sfleft);
		if (sfright)
			SDL_FreeSurface(sfright);
		if (sftop)
			SDL_FreeSurface(sftop);
	} else {
		/* do it straight in screen resolution */
		frame.create(screenWidth,screenHeight);
		frame.fill(0,0,0,0);

		SDL_Texture *oldTex = SDL_GetRenderTarget(mrc);
		SDL_SetRenderTarget(mrc,frame.getTex());

		/* darken board for better reading text */
		SDL_SetRenderDrawColor(mrc,0,0,0,160);
		SDL_Rect boardRect = {
				(int)(MAPWIDTH*brickScreenWidth),
				0,
				(int)(screenWidth - MAPWIDTH*brickScreenWidth),
				(int)(screenHeight) };
		SDL_RenderFillRect(mrc,&boardRect);

		/* use bricks for frame */
		for (int i = 0; i < MAPWIDTH; i++)
			for (int j = 0; j < MAPHEIGHT; j++)
				if (j == 0 || i == 0 || i == MAPWIDTH-1)
					bricks.copy(0, 0, i*brickScreenWidth, j*brickScreenHeight);
		for (int i = MAPWIDTH; i < MAPWIDTH + 6; i++) {
			bricks.copy(0, 0, i*brickScreenWidth, 0);
			bricks.copy(0, 0, i*brickScreenWidth, MAPHEIGHT*brickScreenHeight-brickScreenHeight);
			bricks.copy(0, 0, i*brickScreenWidth, (MAPHEIGHT-7)*brickScreenHeight);
			bricks.copy(0, 0, i*brickScreenWidth, (MAPHEIGHT-11)*brickScreenHeight);
		}
		for (int j = 0; j < MAPHEIGHT; j++) {
			bricks.copy(0, 0, screenWidth - brickScreenWidth, j*brickScreenHeight);
		}

		SDL_SetRenderTarget(mrc,oldTex);
	}

	/* paddle is 90% of brick height */
	if (fileExists(path + "/paddle.png")) {
		int psize = Image::getHeight(path + "/paddle.png") / 4;
		if (oldTheme) {
			/* color key wasn't quite 0x0... */
			SDL_Surface *surf = IMG_Load(string(path + "/paddle.png").c_str());
			if (surf) {
				Uint32 ckey = Image::getSurfacePixel(surf,0,0);
				SDL_SetColorKey(surf,SDL_TRUE,ckey);
				paddles.load(surf, psize, psize);
				SDL_FreeSurface(surf);
			}
		} else
			paddles.load(path + "/paddle.png", psize, psize);
		paddles.scale(9*brickScreenHeight/10, 9*brickScreenHeight/10);
	}

	/* balls are 60% of brick height */
	if (fileExists(path + "/ball.png")) {
		int bsize = Image::getHeight(path + "/ball.png");
		balls.load(path + "/ball.png",bsize,bsize);
		balls.scale(6*brickScreenHeight/10,6*brickScreenHeight/10);
	}

	/* shots are 50% of brick height */
	if (fileExists(path + "/shot.png")) {
		int shtw = Image::getWidth(path + "/shot.png") / shotFrameNum;
		int shth = Image::getHeight(path + "/shot.png");
		if (oldTheme) {
			/* color key wasn't quite 0x0... */
			SDL_Surface *surf = IMG_Load(string(path + "/shot.png").c_str());
			if (surf) {
				Uint32 ckey = Image::getSurfacePixel(surf,0,0);
				SDL_SetColorKey(surf,SDL_TRUE,ckey);
				shot.load(surf, shtw, shth);
				SDL_FreeSurface(surf);
			}
		} else
			shot.load(path + "/shot.png",shtw,shth);
		shot.scale(5*brickScreenHeight/10,5*brickScreenHeight/10);
	}

	/* weapon is 90% brick height (old weapons get scaled as width was 70%) */
	if (fileExists(path + "/weapon.png")) {
		int wpnw = Image::getWidth(path + "/weapon.png") / weaponFrameNum;
		int wpnh = Image::getHeight(path + "/weapon.png");
		weapon.load(path + "/weapon.png",wpnw,wpnh);
		weapon.scale(9*brickScreenHeight/10,9*brickScreenHeight/10);
	}

	/* create life symbol from paddle as 80% brick size */
	/* XXX Image::scale does not work (it just crops) so use SDL directly */
	life.create(brickScreenWidth,brickScreenHeight);
	int lw = 8*brickScreenWidth/10, lh = 6*brickScreenHeight/10;
	SDL_SetRenderTarget(mrc,life.getTex());
	SDL_Rect srect = { 0, 0, paddles.getGridWidth()*3, paddles.getGridHeight() };
	SDL_Rect drect = {
		(int)(brickScreenWidth - lw)/2, (int)(brickScreenHeight - lh)/2, lw, lh };
	SDL_RenderCopy(mrc,paddles.getTex(),&srect,&drect);
	SDL_SetRenderTarget(mrc,NULL);

	/* explosions are square, scaled according to brick ratio */
	if (fileExists(path + "/explosions.png")) {
		uint sz = Image::getWidth(path + "/explosions.png") / explFrameNum;
		explosions.load(path + "/explosions.png",sz,sz);
		uint nw = explosions.getGridWidth() * brickScreenWidth / brickFileWidth;
		uint nh = explosions.getGridHeight() * brickScreenHeight / brickFileHeight;
		explosions.scale(nw,nh);
	}

	/* create shadow images */
	frameShadow.createShadow(frame);
	bricksShadow.createShadow(bricks);
	paddlesShadow.createShadow(paddles);
	ballsShadow.createShadow(balls);
	extrasShadow.createShadow(extras);
	shotShadow.createShadow(shot);

	/* fonts; size is percent of brick height */
	if (fileExists(path + "/" + fontSmallName))
		fSmall.load(path + "/" + fontSmallName,
					fontSmallSize*brickScreenHeight/100);
	if (fileExists(path + "/" + fontNormalName))
		fNormal.load(path + "/" + fontNormalName,
					fontNormalSize*brickScreenHeight/100);
	fNormal.setColor(fontColorNormal);
	fSmall.setColor(fontColorNormal);

	/* sounds */
	sReflectBrick.load(path + "/reflectbrick.wav");
	sReflectPaddle.load(path + "/reflectpaddle.wav");
	sBrickHit.load(path + "/brickhit.wav");
	sExplosion.load(path + "/explosion.wav");
	sEnergyHit.load(path + "/energyhit.wav");
	sShot.load(path + "/shot.wav");
	sAttach.load(path + "/attach.wav");
	sClick.load(path + "/click.wav");
	sDamn.load(path + "/damn.wav");
	sDammit.load(path + "/dammit.wav");
	sExcellent.load(path + "/excellent.wav");
	sVeryGood.load(path + "/verygood.wav");
	sMenuClick.load(path + "/menuclick.wav");
	sMenuMotion.load(path + "/menumotion.wav");
	sExtras[EX_SCORE200].load(path + "/score.wav");
	sExtras[EX_SCORE500].load(path + "/score.wav");
	sExtras[EX_SCORE1000].load(path + "/score.wav");
	sExtras[EX_SCORE2000].load(path + "/score.wav");
	sExtras[EX_SCORE5000].load(path + "/score.wav");
	sExtras[EX_SCORE10000].load(path + "/score.wav");
	sExtras[EX_GOLDSHOWER].load(path + "/score.wav");
	sExtras[EX_SHORTEN].load(path + "/expand.wav");
	sExtras[EX_LENGTHEN].load(path + "/shrink.wav");
	sExtras[EX_LIFE].load(path + "/gainlife.wav");
	sExtras[EX_SLIME].load(path + "/attach.wav");
	sExtras[EX_METAL].load(path + "/energyhit.wav");
	sExtras[EX_BALL].load(path + "/extraball.wav");
	sExtras[EX_WALL].load(path + "/wall.wav");
	sExtras[EX_FROZEN].load(path + "/freeze.wav");
	sExtras[EX_WEAPON].load(path + "/standard.wav");
	sExtras[EX_RANDOM].load(path + "/standard.wav");
	sExtras[EX_FAST].load(path + "/speedup.wav");
	sExtras[EX_SLOW].load(path + "/speeddown.wav");
	sExtras[EX_JOKER].load(path + "/joker.wav");
	sExtras[EX_DARKNESS].load(path + "/darkness.wav");
	sExtras[EX_CHAOS].load(path + "/chaos.wav");
	sExtras[EX_GHOST_PADDLE].load(path + "/ghost.wav");
	sExtras[EX_DISABLE].load(path + "/disable.wav");
	sExtras[EX_TIME_ADD].load(path + "/timeadd.wav");
	sExtras[EX_EXPL_BALL].load(path + "/explball.wav");
	sExtras[EX_BONUS_MAGNET].load(path + "/bonusmagnet.wav");
	sExtras[EX_MALUS_MAGNET].load(path + "/malusmagnet.wav");
	sExtras[EX_WEAK_BALL].load(path + "/weakball.wav");
	sLooseLife.load(path + "/looselife.wav");
}

void Theme::addBox(Image &img, int x, int y, int w, int h)
{
	SDL_Texture *oldTex = SDL_GetRenderTarget(mrc);

	SDL_SetRenderTarget(mrc,img.getTex());

	SDL_SetRenderDrawColor(mrc,0,0,0,160);
	SDL_Rect drect = {x,y,w,h};
	SDL_RenderFillRect(mrc,&drect);

	SDL_SetRenderDrawColor(mrc,fontColorNormal.r,fontColorNormal.g,
					fontColorNormal.b,fontColorNormal.a);
	SDL_Point pts[5] = { {x,y} , {x+w-1,y}, {x+w-1,y+h-1}, {x,y+h-1}, {x,y} };
	SDL_RenderDrawLines(mrc,pts,5);

	SDL_SetRenderTarget(mrc,oldTex);
}
