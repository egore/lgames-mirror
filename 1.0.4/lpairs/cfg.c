/***************************************************************************
                          cfg.c  -  description
                             -------------------
    begin                : Sat Aug 5 2000
    copyright            : (C) 2000 by Michael Speck
    email                : kulkanie@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "sdl.h"

extern char *home_dir;

char c_pth[256];
Cfg cfg;

/*
    get the full path of the cfg file
*/
void C_StPth(char *p)
{
    memset( c_pth, 0, sizeof( c_pth ) );
    if (p[0] == '~') {
        snprintf(c_pth, sizeof(c_pth)-1, "%s%s",
            home_dir, p+1);
    }
    else
        strncpy(c_pth, p, sizeof(c_pth)-1);
}

/*
    load it
*/
void C_Ld()
{
    FILE	*f;

    printf("loading configuration...\n");

    // load init //
    if ((f = fopen(c_pth, "r")) == 0) {
        printf("cfg file '%s' not found; using defaults\n", c_pth);
        C_Def();
	}
	else {

  		fread(&cfg, sizeof(Cfg), 1, f);
		fclose(f);
	}
	
}

/*
    save it
*/
void C_Sv()
{
    //save init //
    FILE	*f = fopen(c_pth, "w");

    fwrite(&cfg, sizeof(Cfg), 1, f);

    fclose(f);
}

/*
    default values
*/
void C_Def()
{
    cfg.dim = 1;
    cfg.fullscreen = 0;
    cfg.sound = 1;
    cfg.sound_vol = 6;
    cfg.map_size = _6x4;
    cfg.reveal = NO_REVEAL;
}
