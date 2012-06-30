/***************************************************************************
                          cfg.h  -  description
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

#ifndef CFG_H
#define CFG_H

/* map_sizes */
enum {
    _4x4,
    _5x4,
    _6x4,
    _6x5,
    _8x5
};

/* reveal? */
enum {
    NO_REVEAL,
    FOR_5_SECONDS,
    UNTIL_KEY_PRESSED
};

// config file //
typedef struct {
    //gfx
    int dim;
    int fullscreen;
    // game
    int sound; /* sound? */
    int sound_vol;
    int map_size;
    int reveal;
} Cfg;

void C_StPth(char *p);
void C_Ld();
void C_Sv();
void C_Def();

#endif
