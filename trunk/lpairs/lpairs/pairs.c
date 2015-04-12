/***************************************************************************
                          pairs.c  -  description
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

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "gettext.h"
#include "sdl.h"
#include "cfg.h"
#include "dynlist.h"
#include "sndsrv.h"
#include "pairs.h"

extern Sdl sdl;
extern int term_game;
extern Cfg cfg;

/* load resources -- button, icons, background */
void load_res( Pairs *pairs )
{
    int i;
    char str[64];
    Uint32 key;
    
    /* background */
    pairs->back = load_surf( "back.bmp", SDL_SWSURFACE );
    SDL_SetColorKey( pairs->back, 0, 0 );

    /* button */
    pairs->button = load_surf( "button.bmp", SDL_SWSURFACE );
    pairs->shadow = create_surf( BUTTON_WIDTH, BUTTON_HEIGHT, SDL_SWSURFACE );
    FULL_DEST( pairs->shadow );
    fill_surf( 0x0 );
    SDL_SetColorKey( pairs->shadow, 0, 0 );
    pairs->shadow_offset = 8;

    /* icons */
    pairs->icon_number = ICON_NUMBER;
    pairs->icons = calloc( ICON_NUMBER, sizeof( SDL_Surface* ) );
    for ( i = 0; i < pairs->icon_number; i++ ) {

        sprintf( str, "%i.bmp", i );
        pairs->icons[i] = load_surf( str, SDL_SWSURFACE );
        key = 0;
        memcpy( &key, pairs->icons[i]->pixels, pairs->icons[i]->format->BytesPerPixel );
        SDL_SetColorKey( pairs->icons[i], SDL_SRCCOLORKEY, key );

    }

    /* save cards (button+icons) to current directory
    for ( i = 1; i <= 15; i++ ) {
        SDL_Surface *surf = create_surf( 64, 64, SDL_SWSURFACE );
        FULL_DEST( surf );
        SOURCE( pairs->button, 0, 0 );
        blit_surf();
        DEST( surf, 
              ( surf->w - pairs->icons[i]->w ) / 2,
              ( surf->h - pairs->icons[i]->h ) / 2,
              pairs->icons[i]->w, pairs->icons[i]->h );
        SOURCE( pairs->icons[i], 0, 0 );
        blit_surf();
        sprintf( str, "card%i.bmp", i );
        SDL_SaveBMP( surf, str );
        SDL_FreeSurface( surf ); 
    } */
    
    /* font */
    pairs->font = load_fixed_font( "f_standard.bmp" , 32, 96, 10);

    /* sounds */
#ifdef SOUND
    pairs->click_sound = Wv_Ld("sound/click.wav");
    pairs->remove_sound = Wv_Ld("sound/remove.wav");
    pairs->fail_sound = Wv_Ld("sound/fail.wav");
#endif
}

/* free resources */
void delete_res( Pairs *pairs )
{
    int i;

    if ( pairs->back ) SDL_FreeSurface( pairs->back );
    if ( pairs->button ) SDL_FreeSurface( pairs->button );
    if ( pairs->shadow ) SDL_FreeSurface( pairs->shadow );
    if ( pairs->icon_number > 0 )
        for ( i = 0; i < pairs->icon_number; i++ )
            if ( pairs->icons[i] ) SDL_FreeSurface( pairs->icons[i] );
    if ( pairs->icons ) free( pairs->icons );
    if ( pairs->font ) free_font( pairs->font );
#ifdef SOUND
    if ( pairs->click_sound ) Wv_Fr( pairs->click_sound );
    if ( pairs->remove_sound ) Wv_Fr( pairs->remove_sound );
    if ( pairs->fail_sound ) Wv_Fr( pairs->fail_sound );
#endif
}

/* check surrounding if an identical card is already there */
int id_card_found( Pairs *pairs, int pos, int new_card )
{
    int x, y;

    y = pos / pairs->width;
    x = pos % pairs->width;

    /* horizontal and vertical neighbors */
    if ( x - 1 >= 0 && pairs->map[pos - 1] == new_card )
        return 1;
    if ( x + 1 < pairs->width && pairs->map[pos + 1] == new_card )
        return 1;
    if ( y - 1 >= 0 && pairs->map[pos - pairs->width] == new_card )
        return 1;
    if ( y + 1 < pairs->height && pairs->map[pos + pairs->width] == new_card )
        return 1;
    /* left upper */
    if ( y - 1 >= 0 && x - 1 >= 0 && pairs->map[pos - pairs->width - 1] == new_card )
        return 1;
    /* right upper */
    if ( y - 1 >= 0 && x + 1 < pairs->width && pairs->map[pos - pairs->width + 1] == new_card )
        return 1;
    /* left lower */
    if ( y + 1 < pairs->height && x - 1 >= 0 && pairs->map[pos + pairs->width - 1] == new_card )
        return 1;
    /* right lower */
    if ( y + 1 < pairs->height && x + 1 < pairs->width && pairs->map[pos + pairs->width + 1] == new_card )
        return 1;

    return 0;
}

/* open game */
int open_game( Pairs *pairs, int width, int height )
{
    int i, dummy, pos1, pos2;
    int map_size = width * height;
    int pair_num = map_size / 2;
    int pair_ids[ICON_NUMBER]; /* all ids, first will be selected */
    int leave = 0;
    SDL_Event event;
    int reveal_time = 0;

    /* free old map */
    if ( pairs->map ) free( pairs->map );

    /* check if new size is valid */
    if ( ( map_size & 1 ) || map_size > ICON_NUMBER * 2 || height > MAX_MAP_HEIGHT ) {

        fprintf( stderr, "new map size (%ix%i) is invalid\n", width, height );
        return 0;

    }

    /* set new size */
    pairs->width = width;
    pairs->height = height;

    /* pairs to go */
    pairs->pairs_left = map_size / 2;

	/* randomly choose pairnum pairs */
	for (i = 0; i < ICON_NUMBER; i++)
		pair_ids[i] = i;
	for (i = 0; i < ICON_NUMBER*20; i++) {
		pos1 = rand() % ICON_NUMBER;
		pos2 = rand() % ICON_NUMBER;
		dummy = pair_ids[pos1];
		pair_ids[pos1] = pair_ids[pos2];
		pair_ids[pos2] = dummy;
	}

    /* recreate map */
    pairs->map = calloc( map_size, sizeof( int ) );
    /* add sorted pairs */
    i = 0;
    while ( i < pair_num ) {
        pairs->map[i] = pair_ids[i];
        pairs->map[i + map_size / 2] = pair_ids[i];
        i++;
    }
    /* unsort these pairs */
    for ( i = 0; i < map_size * 20; i++ ) {

        pos1 = rand() % map_size;
        pos2 = rand() % map_size;

        /* only allow switch if not two identical cards beside each other */
        if ( !id_card_found( pairs, pos1, pairs->map[pos2] ) &&
             !id_card_found( pairs, pos2, pairs->map[pos1] ) ) {

            dummy = pairs->map[pos1];
            pairs->map[pos1] = pairs->map[pos2];
            pairs->map[pos2] = dummy;

        }

    }

    /* compute grid offset */
    pairs->x_offset = ( sdl.screen->w - ( pairs->width * ( BUTTON_WIDTH + BORDER ) - BORDER ) ) >> 1;
    pairs->y_offset = ( sdl.screen->h - ( pairs->height * ( BUTTON_HEIGHT + BORDER ) - BORDER ) ) >> 1;

    /* draw background */
    FULL_DEST( sdl.screen );
    FULL_SOURCE( pairs->back );
    blit_surf();

    /* undim */
    if ( cfg.dim )
        UNDIM_SCREEN();

    /* update screen */
    refresh_screen( 0, 0, 0, 0 );

    /* draw all buttons */
    draw_all_buttons( pairs, CLOSED );

    /* reset time&turns */
    pairs->time = 0;
    pairs->tries = 0;

    pairs->status = NO_SEL;

    /* reveal cards first? */
    if ( cfg.reveal != NO_REVEAL ) {

        draw_all_buttons( pairs, OPEN );
        refresh_rects();

        reset_timer();

        while ( !leave ) {

            if ( cfg.reveal == FOR_5_SECONDS ) {

                reveal_time += get_time();
                if ( reveal_time > 5000 ) leave = 1;

            }
            if ( SDL_PollEvent( &event ) )
                switch ( event.type ) {

                    case SDL_QUIT:
                        term_game = 1;
                    case SDL_KEYUP:
                    case SDL_MOUSEBUTTONUP:
                        leave = 1;
                        break;

                }

            SDL_Delay( 5 );

        }

        draw_all_buttons( pairs, CLOSED );
        refresh_rects();

    }

    return 1;
}

/* close game */
void close_game( Pairs *pairs )
{
    if ( cfg.fullscreen )
        set_video_mode( 640, 480, 16, SDL_SWSURFACE );
}

/* init card removal animation */
void init_card_animation(Pairs *pairs, int mx1, int my1, int mx2, int my2)
{
	int sx1 = pairs->x_offset + mx1 * ( BUTTON_WIDTH + BORDER );
	int sy1 = pairs->y_offset + my1 * ( BUTTON_HEIGHT + BORDER );
	int sx2 = pairs->x_offset + mx2 * ( BUTTON_WIDTH + BORDER );
	int sy2 = pairs->y_offset + my2 * ( BUTTON_HEIGHT + BORDER );
	int dx = (sx1 + sx2)/2, dy = (sy1 + sy2)/2;
	int sa = 0, da = 192;
	SDL_Surface *img = 0;
	
	img = create_surf(BUTTON_WIDTH, BUTTON_HEIGHT, SDL_SWSURFACE);
	DEST(img,0,0,img->w,img->h);
	SOURCE(sdl.screen,sx1,sy1)
	blit_surf();
	sa_init(&pairs->anim1,img,sx1,sy1,sa,dx,dy,da,300);
	
	img = create_surf(BUTTON_WIDTH, BUTTON_HEIGHT, SDL_SWSURFACE);
	DEST(img,0,0,img->w,img->h);
	SOURCE(sdl.screen,sx2,sy2)
	blit_surf();
	sa_init(&pairs->anim2,img,sx2,sy2,sa,dx,dy,da,300);
}

/* run game */
void run_game( Pairs *pairs )
{
    SDL_Event event;
    int leave = 0;
    int x, y;
    int ms;
    char str[128];
    int restart = 0;

    reset_timer();

    while ( !leave && !term_game ) {

        if ( SDL_PollEvent( &event ) ) {

            switch ( event.type ) {

                case SDL_KEYUP:
                    if ( event.key.keysym.sym == SDLK_ESCAPE ) leave = 1;
                    if ( event.key.keysym.sym == SDLK_r ) restart = 1;
                    break;

                case SDL_MOUSEBUTTONUP:
                    if ( event.button.button == LEFT_BUTTON && 
										pairs->status != BOTH_SEL && 
										pairs->status != REMOVE_CARDS )
                        if ( get_map_pos( pairs, event.button.x, event.button.y, &x, &y ) ) {

                            /* same tile? */
                            if ( pairs->status == ONE_SEL &&
                                 pairs->first_sel.x == x && pairs->first_sel.y == y )
                                break;

                            draw_button( pairs, x, y, OPEN );
                            if ( pairs->status == NO_SEL ) {

                                pairs->status = ONE_SEL;
                                pairs->first_sel.x = x;
                                pairs->first_sel.y = y;
#ifdef SOUND
                                SSrv_Ply(pairs->click_sound, 0);
#endif

                            }
                            else {

                                pairs->status = BOTH_SEL;
                                pairs->delay = DELAY;
                                pairs->sec_sel.x = x;
                                pairs->sec_sel.y = y;
#ifdef SOUND
                                SSrv_Ply(pairs->click_sound, 0);
#endif

                            }
                            break;

                        }
                    if ( pairs->status == BOTH_SEL ) {

                        pairs->status = CLOSE_SEL;
#ifdef SOUND
                        SSrv_Ply(pairs->click_sound, 0);
#endif
                        break;

                    }
                    break;

            }

        }

        /* get milliseconds */
        ms = get_time();

        /* add time */
        if (pairs->status != REMOVE_CARDS)
			pairs->time += ms;
		else {
			if (sa_update(&pairs->anim1,ms) || sa_update(&pairs->anim2,ms)) {
				/* both reach destination at the same time */
				pairs->pairs_left--;
                if ( pairs->pairs_left <= 0 )
                    pairs->status = DONE;
                else
					pairs->status = NO_SEL;

				sa_finalize(&pairs->anim1);
				sa_finalize(&pairs->anim2);
				sa_draw_background();
				sdl.rect_count = RECT_LIMIT; /* XXX fake full update */
			} else {
				sa_draw_background();
				sa_draw(&pairs->anim1);
				sa_draw(&pairs->anim2);
				sdl.rect_count = RECT_LIMIT; /* XXX fake full update */
			}
		}

        /* check if buttons must be closed */
        if ( pairs->status == BOTH_SEL ) {

            pairs->delay -= ms;
            if ( pairs->delay <= 0 ) {

                pairs->delay = 0;
                pairs->status = CLOSE_SEL;

            }

        }

        /* close buttons? */
        if ( pairs->status == CLOSE_SEL ) {

            /* remove cards? */
            if (get_map_cont(pairs, pairs->first_sel.x, pairs->first_sel.y) ==
                get_map_cont(pairs, pairs->sec_sel.x, pairs->sec_sel.y )) {

				pairs->status = REMOVE_CARDS;
                init_card_animation(pairs, 
								pairs->first_sel.x, pairs->first_sel.y,
								pairs->sec_sel.x, pairs->sec_sel.y);
                
                set_map_cont( pairs, pairs->first_sel.x, pairs->first_sel.y, -1 );
                set_map_cont( pairs, pairs->sec_sel.x, pairs->sec_sel.y, -1 );
            }

            draw_button( pairs, pairs->first_sel.x, pairs->first_sel.y, CLOSED );
            draw_button( pairs, pairs->sec_sel.x, pairs->sec_sel.y, CLOSED );
            if (pairs->status != REMOVE_CARDS)
				pairs->status = NO_SEL;
			else
				sa_get_background();

#ifdef SOUND
            if ( pairs->status == REMOVE_CARDS )
                SSrv_Ply(pairs->remove_sound, 0);
            else
                SSrv_Ply(pairs->fail_sound, 0);
#endif

            pairs->tries++;

        }
        
        draw_info( pairs );

        refresh_rects();

        if ( restart ) {

            restart = 0;

            if ( cfg.dim )
                DIM_SCREEN();

            open_game( pairs, pairs->width, pairs->height );

            reset_timer();

        }
        else
            if ( pairs->status == DONE ) {

                pairs->font->align = ALIGN_X_CENTER | ALIGN_Y_TOP;
                y = ( sdl.screen->h >> 1 ) - 50;

                sprintf( str, _("Congratulations! You have resolved this puzzle!") );
                write_text( pairs->font, sdl.screen, sdl.screen->w >> 1, y, str, OPAQUE );
                sprintf( str, _("It now will restart with the same settings.") );
                write_text( pairs->font, sdl.screen, sdl.screen->w >> 1, y + 20, str, OPAQUE );
                sprintf( str, _("If you want to change these press <Escape> after restart.") );
                write_text( pairs->font, sdl.screen, sdl.screen->w >> 1, y + 40, str, OPAQUE );
                sprintf( str, _("(Press any key to continue...)") );
                write_text( pairs->font, sdl.screen, sdl.screen->w >> 1, y + 60, str, OPAQUE );

                refresh_screen( 0, 0, 0, 0 );

                wait_for_click();

                if ( cfg.dim )
                    DIM_SCREEN();

                open_game( pairs, pairs->width, pairs->height );

                reset_timer();

            }

        SDL_Delay( 5 );
    }

    if ( cfg.dim )
        DIM_SCREEN();
}

/* draw single button -- either OPEN or CLOSED */
void draw_button( Pairs *pairs, int map_x, int map_y, int type )
{
    int x, y, i;

    x = pairs->x_offset + map_x * ( BUTTON_WIDTH + BORDER );
    y = pairs->y_offset + map_y * ( BUTTON_HEIGHT + BORDER );

    DEST( sdl.screen, x, y, BUTTON_WIDTH + pairs->shadow_offset, BUTTON_HEIGHT + pairs->shadow_offset );
    SOURCE( pairs->back, x, y );
    blit_surf();
    add_refresh_rect( x, y, BUTTON_WIDTH + pairs->shadow_offset, BUTTON_HEIGHT + pairs->shadow_offset );

    if ( get_map_cont( pairs, map_x, map_y) == -1 )
        return ;

    /* shadow */
    DEST( sdl.screen, x + pairs->shadow_offset, y + pairs->shadow_offset, BUTTON_WIDTH, BUTTON_HEIGHT );
    SOURCE( pairs->shadow, 0, 0 );
    alpha_blit_surf( 128 );

    if ( type == OPEN ) {

        DEST( sdl.screen, x, y, BUTTON_WIDTH, BUTTON_HEIGHT );
        SOURCE( pairs->button, 0, 0 );
        blit_surf();

        i = get_map_cont( pairs, map_x, map_y );
        DEST( sdl.screen,
              x + ( ( BUTTON_WIDTH - pairs->icons[i]->w ) >> 1 ),
              y + ( ( BUTTON_HEIGHT - pairs->icons[i]->h ) >> 1 ),
              pairs->icons[i]->w, pairs->icons[i]->h );
        SOURCE( pairs->icons[i], 0, 0 );
        blit_surf();

    }
    else {

        DEST( sdl.screen, x, y, BUTTON_WIDTH, BUTTON_HEIGHT );
        SOURCE( pairs->button, BUTTON_WIDTH, 0 );
        blit_surf();

    }
}

/* get map contents at position */
inline int get_map_cont( Pairs *pairs, int map_x, int map_y )
{
    return ( pairs->map[map_y * pairs->width + map_x] );
}

/* set map contents at position */
inline void set_map_cont( Pairs *pairs, int map_x, int map_y, int cont )
{
    pairs->map[map_y * pairs->width + map_x] = cont;
}

/* draw all buttons */
void draw_all_buttons( Pairs *pairs, int type )
{
    int i, j;

    for ( i = 0; i < pairs->width; i++)
        for ( j = 0; j < pairs->height; j++ )
            draw_button( pairs, i, j, type );

    refresh_rects();

}

/* get map pos */
int get_map_pos( Pairs *pairs, int screen_x, int screen_y, int *map_x, int *map_y )
{
    int old_x = screen_x, old_y = screen_y;

    screen_x -= pairs->x_offset;
    screen_x /= BUTTON_WIDTH + BORDER;
    screen_y -= pairs->y_offset;
    screen_y /= BUTTON_HEIGHT + BORDER;

    (*map_x) = screen_x; (*map_y) = screen_y;

    if ( old_x - ( pairs->x_offset + screen_x * ( BUTTON_WIDTH + BORDER ) ) > BUTTON_WIDTH ) return 0;
    if ( old_y - ( pairs->y_offset + screen_y * ( BUTTON_HEIGHT + BORDER ) ) > BUTTON_HEIGHT ) return 0;

    if ( get_map_cont( pairs, *map_x, *map_y ) == -1 ) return 0;

    if ( screen_x < 0 || screen_y < 0 || screen_x >= pairs->width || screen_y >= pairs->height )
        return 0;

    return 1;
}

/* convert time in seconds into string */
void time_to_str( char *str, int time )
{
    int min = time / 60;
    int sec = time % 60;
    if ( sec < 10 )
        sprintf( str, "%i:0%i", min, sec );
    else
        sprintf( str, "%i:%i", min, sec );
}

/* draw info about game */
void draw_info( Pairs *pairs )
{
    int width = 200, height = 40;
    char str[128], time_str[64];

    /* restore upper background */
    DEST( sdl.screen, sdl.screen->w - width, 0, width, height );
    SOURCE( pairs->back, sdl.screen->w - width, 0 );
    blit_surf();

    /* restore lower background */
    DEST( sdl.screen, sdl.screen->w - width, sdl.screen->h - height, width, height );
    SOURCE( pairs->back, sdl.screen->w - width, sdl.screen->h - height );
    blit_surf();

    /* draw upper info */
    pairs->font->align = ALIGN_X_RIGHT | ALIGN_Y_TOP;
    sprintf( str, _("Pairs left: %i"), pairs->pairs_left );
    write_text( pairs->font, sdl.screen, sdl.screen->w - 2, 12, str, OPAQUE );

    /* draw lower info */
    pairs->font->align = ALIGN_X_RIGHT | ALIGN_Y_BOTTOM;
    sprintf( str, _("Tries: %i"), pairs->tries );
    write_text( pairs->font, sdl.screen, sdl.screen->w - 2, sdl.screen->h - 8, str, OPAQUE );
    time_to_str( time_str, pairs->time / 1000 );
    sprintf( str, _("Time: %s"), time_str );
    write_text( pairs->font, sdl.screen, sdl.screen->w - 2, sdl.screen->h - 24, str, OPAQUE );

    /* mark for update */
    add_refresh_rect( sdl.screen->w - width, 0, width, height );
    add_refresh_rect( sdl.screen->w - width, sdl.screen->h - height, width, height );
}
