/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Fre Mär 16 10:16:42 CET 2001
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

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include "gettext.h"
#include "sdl.h"
#include "dynlist.h"
#include "cfg.h"
#include "sndsrv.h"
#include "menu.h"
#include "pairs.h"

int term_game = 0; /* terminate game quickly? */

char *home_dir; /* home directory */

extern MMng mm;
#ifdef SOUND
// sound server -- sndsrv.c //
extern SSrv ssrv;
#endif

/* config --config.c */
extern Cfg cfg;

/*
    activate / deactivate sound
*/
void set_sound()
{
#ifdef SOUND
    SSrv_StA(cfg.sound);
#endif
}

/*
    set sound volume
*/
void set_volume()
{
#ifdef SOUND
    SSrv_StV(cfg.sound_vol);
#endif
}

void setup_menu()
{
    SDL_Surface *back;
    Font        *font_y, *font_w, *small_font;
    Menu        *_main, *gfx, *audio;
    MEnt        *entry;
    char str[128];
    char *map_str[] = {
        "4x4",
        "5x4",
        "6x4",
        "6x5",
        "8x5"
    };
    char *reveal_str[] = {
        _("Off"),
        _("For 5 Seconds"),
        _("Until Key Pressed")
    };

    // load and assign gfx (back now includes logo) //
    back = load_surf("title.bmp", SDL_HWSURFACE);
    SDL_SetColorKey( back, 0, 0 );
    font_y = load_fixed_font("f_yellow.bmp", 32, 96, 10);
    font_w = load_fixed_font("f_white.bmp", 32, 96, 10);
    MM_Ini(back->w / 2, back->h - 100, 50, back, NULL, font_y, font_w);

    /* add copyright and homepage */
    small_font = load_fixed_font( "f_small_white.bmp" , 32, 96, 8);
    small_font->align = ALIGN_X_LEFT | ALIGN_Y_BOTTOM;
    sprintf( str, "http://lgames.sourceforge.net" );
    write_text( small_font, back, 2, back->h - 2, str, OPAQUE );
    small_font->align = ALIGN_X_RIGHT | ALIGN_Y_BOTTOM;
    sprintf( str, "Copyright 2001 Michael Speck" );
    write_text( small_font, back, back->w - 2, back->h - 2, str, OPAQUE );
    free_font( small_font );

    // create and add entrys //
    _main = M_Crt(); MM_Add(_main);
    gfx = M_Crt(); MM_Add(gfx);
    audio = M_Crt(); MM_Add(audio);

    // main //
    M_Add( _main, ME_CrtAct( _("Start Game"), START_GAME ) );
    M_Add( _main, ME_CrtSep( "" ) );
    M_Add( _main, ME_CrtSwX( _("Cards:"), &cfg.map_size, map_str, 5 ) );
    M_Add( _main, ME_CrtSwX( _("Reveal Cards At Beginning:"), &cfg.reveal, reveal_str, 3 ) );
    M_Add( _main, ME_CrtSep( "" ) );
    M_Add( _main, ME_CrtSub( _("Graphics"), gfx ) );
#ifdef SOUND
    M_Add( _main, ME_CrtSub( _("Audio"), audio ) );
#else
    M_Add( _main, ME_CrtSep( _("Audio") ) );
#endif
    M_Add( _main, ME_CrtSep( "" ) );
    M_Add( _main, ME_CrtAct( _("Quit"), QUIT_MENU ) );
    // graphics
    M_Add( gfx, ME_CrtSw2( _("Fullscreen:"), &cfg.fullscreen, _("Off"), _("On") ) );
    M_Add( gfx, ME_CrtSw2( _("Dim Effect:"), &cfg.dim, _("Off"), _("On") ) );
    M_Add( gfx, ME_CrtSep( "" ) );
    M_Add( gfx, ME_CrtSub( _("Back"), _main ) );
    // audio
    entry = ME_CrtSw2( _("Sound:"), &cfg.sound, _("Off"), _("On") );
    entry->cb = set_sound;
    M_Add( audio, entry );
    entry = ME_CrtRng( _("Volume:"), &cfg.sound_vol, 1, 8, 1 );
    entry->cb = set_volume;
    M_Add( audio, entry );
    M_Add( audio, ME_CrtSep( "" ) );
    M_Add( audio, ME_CrtSub( _("Back"), _main ) );

    // adjust position of all entries in all menus //
    MM_AdjP();

    // check for errors
    MM_Ck();
}

int main(int argc, char *argv[])
{
    Pairs pairs;
    int go_on = 1;
    SDL_Event event;
    int ms;
    int width, height;

    /* i18n */
#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    
    /* get home directory */
    home_dir = getenv( "HOME" );

    /* set random seed */
    srand( (unsigned int)time( 0 ) );

    /* load config */
    C_StPth( "~/.lpairs.cfg" );
    C_Ld();

    /* clear struct */
    memset( &pairs, 0, sizeof( Pairs ) );

    /* init sdl */
#ifdef SOUND
    init_sdl( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER );
#else
    init_sdl( SDL_INIT_VIDEO | SDL_INIT_TIMER );
#endif
    set_video_mode( 640, 480, 16, SDL_SWSURFACE );
    SDL_WM_SetCaption( "LPairs", 0 );

    /* load resources */
    load_res( &pairs );

#ifdef SOUND
    // sound server //
    SSrv_Ini(AUDIO_U8, 22050, 1, 128, 16);
    if (!cfg.sound)
        ssrv.slp = 1;
    SSrv_StV(cfg.sound_vol);
#endif

    // init and show menu//
    setup_menu();
    MM_Shw(MM_KP);

    //menu loop
    reset_timer(); // reset time //
    while (go_on && !term_game) {
        M_Hd(mm.c_mn);
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                term_game = 1;
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
                go_on = 0;
            switch (MM_Evt(&event)) {
                case QUIT_MENU:
                    go_on = 0;
                    break;
                case START_GAME:
                    switch ( cfg.map_size ) {

                        case _4x4: width = 4; height = 4; break;
                        case _5x4: width = 5; height = 4; break;
                        case _6x4: width = 6; height = 4; break;
                        case _6x5: width = 6; height = 5; break;
                        case _8x5: width = 8; height = 5; break;

                    }
                    if ( cfg.dim )
                        DIM_SCREEN();
                    /* switch to fullscreen */
                    if ( cfg.fullscreen )
                        set_video_mode( 640, 480, 16, SDL_FULLSCREEN | SDL_SWSURFACE );
                    if ( open_game( &pairs, width, height ) ) {

                        run_game( &pairs );
                        close_game( &pairs );
                        MM_Shw(MM_KP);

                    }
                    break;
            }
        }
        ms = get_time();
        M_CmA(mm.c_mn, ms);
        M_Shw(mm.c_mn);
        refresh_rects();
        SDL_Delay( 5 );
    }

    // terminate menu //
    MM_Trm();

#ifdef SOUND
    SSrv_Trm();
#endif

    /* free resources */
    delete_res( &pairs );
    sa_free_background();

    /* save config */
    C_Sv();

    return EXIT_SUCCESS;
}
