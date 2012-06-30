/***************************************************************************
                          pairs.h  -  description
                             -------------------
    begin                : Fri Mar 16 2001
    copyright            : (C) 2001 by Michael Speck
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

#ifndef __PAIRS_H
#define __PAIRS_H

/* i18n */
#if ENABLE_NLS
#define _(str) gettext (str)
#else
#define _(str) (str)
#endif

/* misc definitions */
enum {
    ICON_NUMBER = 20,
    MAX_MAP_HEIGHT = 5,
    BUTTON_WIDTH = 64,
    BUTTON_HEIGHT = 64,
    BORDER = 16, /* distance between buttons */
    DELAY = 1000
};

/* draw map tile open or closed? */
enum {
    CLOSED = 0,
    OPEN
};

typedef struct {
    int x, y;
} Pos;

/* game stats */
enum {
    NO_SEL = 0,
    ONE_SEL,
    BOTH_SEL,
    CLOSE_SEL,
    DONE
};

/* game struct */
typedef struct {
    /* resources */
    SDL_Surface *back; /* background */
    SDL_Surface *button; /* button */
    SDL_Surface *shadow; /* button's shadow */
    int shadow_offset;
    SDL_Surface **icons; /* icons displayed on buttons */
    int icon_number;
    Font *font;
#ifdef SOUND
    Wv *click_sound;
    Wv *remove_sound;
    Wv *fail_sound;
#endif
    /* pair map */
    int width, height;
    int *map; /* map of pair indexes */
    /* offsets */
    int x_offset, y_offset;
    /* misc */
    int status;
    int delay; /* delay until both cards will disappear */
    Pos first_sel, sec_sel; /* selections */
    int pairs_left; /* counter of pairs left */
    int time; /* time in millseconds till start */
    int tries; /* turns needed */
} Pairs;

/* load resources -- button, icons, background */
void load_res( Pairs *pairs );

/* free resources */
void delete_res( Pairs *pairs );

/* open game */
int open_game( Pairs *pairs, int width, int height );

/* close game */
void close_game( Pairs *pairs );

/* run game */
void run_game( Pairs *pairs );

/* draw single button -- either OPEN or CLOSED */
void draw_button( Pairs *pairs, int map_x, int map_y, int type );

/* get map contents at position */
inline int get_map_cont( Pairs *pairs, int map_x, int map_y );

/* set map contents at position */
inline void set_map_cont( Pairs *pairs, int map_x, int map_y, int cont );

/* draw all buttons */
void draw_all_buttons( Pairs *pairs, int type );

/* get map pos */
int get_map_pos( Pairs *pairs, int screen_x, int screen_y, int *map_x, int *map_y );

/* convert time in seconds into string */
void time_to_str( char *str, int time );

/* draw info about game */
void draw_info( Pairs *pairs );

#endif
