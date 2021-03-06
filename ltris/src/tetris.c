/***************************************************************************
                          tetris.c  -  description
                             -------------------
    begin                : Tue Dec 25 2001
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

#include "ltris.h"
#include "event.h"
#include "chart.h"
#include "bowl.h"
#include "shrapnells.h"
#include "tetris.h"

SDL_Surface *blocks = 0;
SDL_Surface *previewpieces = 0;
SDL_Surface *logo = 0;
Font *font = 0, *large_font = 0;
SDL_Surface *qmark = 0; /* question mark */
SDL_Surface *bkgnd = 0; /* background + 3dframes */
SDL_Surface *offscreen = 0; /* offscreen: background + blocks */
int last_bkgnd_id = -99; /* last background chosen */
#ifdef SOUND
Sound_Chunk *wav_click = 0;
#endif
Bowl *bowls[BOWL_COUNT]; /* all bowls */
int  *next_blocks = NULL, next_blocks_size = 0; /* all receive same blocks */
int last_generated_block = -1; /* block id of last block generated when filling bag */

extern Sdl sdl;
extern int keystate[SDLK_LAST];
extern Config config;
extern int term_game;
extern char gametype_ids[8][64];

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Draw the tetris logo to the offscreen.
====================================================================
*/
void tetris_draw_logo()
{
    int x = 460, y = 100;
    draw_3dframe( offscreen, x - logo->w / 2 - 4, y - logo->h / 2 - 4, logo->w + 8, logo->h + 8, 6 );
    DEST( offscreen, x - logo->w / 2, y - logo->h / 2, logo->w, logo->h );
    FULL_SOURCE( logo );
    blit_surf();
}

/*
====================================================================
Request confirmation.
====================================================================
*/
enum{ CONFIRM_YES_NO, CONFIRM_ANY_KEY, CONFIRM_PAUSE };
void draw_confirm_screen( Font *font, SDL_Surface *buffer, char *str )
{
    FULL_DEST(sdl.screen);
    fill_surf(0x0);
    font->align = ALIGN_X_CENTER | ALIGN_Y_CENTER;
    write_text(font, sdl.screen, sdl.screen->w / 2, sdl.screen->h / 2, str, 0);
}
int confirm( Font *font, char *str, int type )
{
    SDL_Event event;
    int go_on = 1;
    int ret = 0;
    SDL_Surface *buffer = create_surf(sdl.screen->w, sdl.screen->h, SDL_SWSURFACE);
    SDL_SetColorKey(buffer, 0, 0);

#ifdef SOUND
    sound_play( wav_click );
#endif

    FULL_DEST(buffer);
    FULL_SOURCE(sdl.screen);
    blit_surf();
    draw_confirm_screen( font, buffer, str );
    refresh_screen( 0, 0, 0, 0 );

    while (go_on && !term_game) {
        SDL_WaitEvent(&event);
        /* TEST */
        switch ( event.type ) {
            case SDL_QUIT: term_game = 1; break;
            case SDL_KEYUP:
                if ( type == CONFIRM_ANY_KEY ) {
                    ret = 1; go_on = 0;
                    break;
                }
                else
                if ( type == CONFIRM_PAUSE ) {
                    if ( event.key.keysym.sym == config.pause_key ) {
                        ret = 1; go_on = 0;
                        break;
                    }
					else
					if ( event.key.keysym.sym == SDLK_f ) {
#ifndef WIN32
						config.fullscreen = !config.fullscreen;
						set_video_mode( config.fullscreen );
						draw_confirm_screen( font, buffer, str );
						refresh_screen( 0, 0, 0, 0 );
#endif
					}
                }
                else
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        // ESC always returns false
                        go_on = 0;
                        ret = 0;					
                    } else {
                        // check pressed key against yes/no letters
                        char *keyName = SDL_GetKeyName(event.key.keysym.sym);
                        char *yesLetter = _("y");
                        char *noLetter = _("n");
                        if (strcmp(keyName, yesLetter) == 0) {
                            go_on = 0;
                            ret = 1;
                        }
                        if (strcmp(keyName, noLetter) == 0) {
                            go_on = 0;
                            ret = 0;
                        }
#ifdef WIN32
                        /* y/z not recognized properly in windows */
                        if (event.key.keysym.sym == SDLK_y ||
                        		event.key.keysym.sym == SDLK_z) {
                        	go_on = 0;
                        	ret = 1;
                        }
#endif
                    }
                break;
        }
    }
#ifdef SOUND
    sound_play( wav_click );
#endif
    FULL_DEST(sdl.screen);
    FULL_SOURCE(buffer);
    blit_surf();
    refresh_screen( 0, 0, 0, 0 );
    SDL_FreeSurface(buffer);
    reset_timer();
    
    return ret;
}
/*
====================================================================
Do a new background to bkgnd and add all nescessary frames for the
specified gametype in config::gametype.
Id == -1 means to use a random background.
Copy result to offscreen and screen. Draws bowl contents.
====================================================================
*/
enum { BACK_COUNT = 4 };
void tetris_recreate_bkgnd( int id )
{
    SDL_Surface *pic = 0;
    char name[32];
    int i, j;

    /* load and wallpaper background */
    if ( id == -1 ) {
        do {
            id = rand() % BACK_COUNT;
        } while ( id == last_bkgnd_id );
        last_bkgnd_id = id;
    }
    FULL_DEST( bkgnd ); fill_surf( 0x0 );
    /* load background */
    sprintf( name, "back%i.bmp", id );
    if ( ( pic = load_surf( name, SDL_SWSURFACE | SDL_NONFATAL ) ) != 0 ) {
        for ( i = 0; i < bkgnd->w; i += pic->w )
            for ( j = 0; j < bkgnd->h; j += pic->h ) {
                DEST( bkgnd, i, j, pic->w, pic->h );
                SOURCE( pic, 0, 0 );
                blit_surf();
            }
        SDL_FreeSurface( pic );
    }
    /* let the bowls contribute to the background :) */
    for ( i = 0; i < BOWL_COUNT; i++ )
        if ( bowls[i] )
            bowl_draw_frames( bowls[i] );
    /* draw to offscreen */
    FULL_DEST( offscreen ); FULL_SOURCE( bkgnd ); blit_surf();
    /* add logo if place
    if ( config.gametype <= 2 )
        tetris_draw_logo(); */
    /* draw bowl contents */
    for ( i = 0; i < BOWL_COUNT; i++ )
        if ( bowls[i] )
            bowl_draw_contents( bowls[i] );
    /* put offscreen to screen */
    FULL_DEST( sdl.screen ); FULL_SOURCE( offscreen ); blit_surf();
}

extern Block_Mask block_masks[BLOCK_COUNT];

/** Render preview of all pieces to previewpieces */
static void render_preview_pieces()
{
	int bs = BOWL_BLOCK_SIZE;
	int xoff[7] = {-10,-10,-10,0,-10,-10,0};
	int yoff[7] = {-40,-40,-40,-40,-40,-40,-30};

	for (int k = 0; k < 7; k++)
		for (int j = 0; j < 4; j++)
			for (int i = 0; i < 4; i++) {
				if (!block_masks[k].mask[block_masks[k].rstart][i][j])
					continue;
	                        DEST( previewpieces,
	                        	xoff[k] + i*bs,
	                        	yoff[k] + j*bs + k*bs*2,
	                        	bs, bs);
	                        SOURCE( blocks, block_masks[k].blockid * bs, 0);
	                        blit_surf();
			}
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Load/delete all tetris resources.
====================================================================
*/
void tetris_create()
{
    logo = load_surf( "logo.bmp", SDL_SWSURFACE );
    SDL_SetColorKey( logo, SDL_SRCCOLORKEY, get_pixel( logo, 0, 0 ) );
    blocks = load_surf( "blocks.bmp", SDL_SWSURFACE );
    qmark = load_surf( "quest.bmp", SDL_SWSURFACE );
    SDL_SetColorKey( qmark, SDL_SRCCOLORKEY, get_pixel( qmark, 0, 0 ) );
    font = load_fixed_font( "f_small_yellow.bmp", 32, 96, 8 );
    large_font = load_fixed_font( "f_white.bmp", 32, 96, 10 );
    bkgnd = create_surf( sdl.screen->w, sdl.screen->h, SDL_SWSURFACE );
    SDL_SetColorKey( bkgnd, 0, 0 );
    offscreen = create_surf( sdl.screen->w, sdl.screen->h, SDL_SWSURFACE );
    SDL_SetColorKey( offscreen, 0, 0 );
    bowl_load_figures();
    bowl_init_block_masks();

    /* pre-render pieces for preview (7x4x2 in one column) */
    previewpieces = create_surf(BOWL_BLOCK_SIZE*4,7*BOWL_BLOCK_SIZE*2,SDL_SWSURFACE);
    render_preview_pieces();

#ifdef SOUND
    wav_click = sound_chunk_load( "click.wav" );
#endif
}
void tetris_delete()
{
    if ( logo )
	    SDL_FreeSurface( logo );
    logo = 0;
    if ( blocks )
	    SDL_FreeSurface( blocks );
    blocks = 0;
    if ( previewpieces )
	    SDL_FreeSurface( previewpieces );
    previewpieces = 0;
    if ( qmark )
	    SDL_FreeSurface( qmark );
    qmark = 0;
    free_font( &font );
    free_font( &large_font );
    if ( offscreen )
	    SDL_FreeSurface( offscreen );
    offscreen = 0;
    if ( bkgnd )
	    SDL_FreeSurface( bkgnd );
    bkgnd = 0;
#ifdef SOUND        
    if ( wav_click )
	    sound_chunk_free( wav_click );
    wav_click = 0;
#endif        
}

/*
====================================================================
Initiate/clear a new game from config data.
After tetris_init() the screen is drawn completely though not
updated to use the fade effect.
====================================================================
*/
int  tetris_init()
{
	/* create block buffer with several bags each containing all 7 
	 * tetrominoes permuted randomly */
	last_generated_block = -1;
	next_blocks_size = BLOCK_BAG_COUNT * BLOCK_COUNT;
	next_blocks = calloc( next_blocks_size, sizeof(int) );
	fill_random_block_bags( next_blocks, BLOCK_BAG_COUNT, config.modern );

    /* create bowls according to the gametype */
    switch ( config.gametype ) {
        case GAME_DEMO:
            bowls[0] = bowl_create( 220, 0, 490, 160, -1,-1, blocks, qmark, "Demo", 0 );
            break;
        case GAME_CLASSIC:
        case GAME_TRAINING:
        case GAME_FIGURES:
            bowls[0] = bowl_create( 220, 0, 490, config.modern?100:160, 490, 290, blocks, qmark, config.player1.name, &config.player1.controls );
            break;
        case GAME_VS_HUMAN:
        case GAME_VS_CPU:
        	bowls[0] = bowl_create( 20, 0, 233, config.modern?100:160, 233,290,
        			blocks, qmark, config.player1.name,
				&config.player1.controls );
        	if ( config.gametype == GAME_VS_HUMAN )
        		bowls[1] = bowl_create( 420, 0, 327, config.modern?100:160,
        				327,290, blocks, qmark, config.player2.name,
					&config.player2.controls );
        	else
        		bowls[1] = bowl_create( 420, 0, 327, config.modern?100:160,
        				327,290, blocks, qmark, "CPU-1", 0 );
            break;
        case GAME_VS_HUMAN_HUMAN:
        case GAME_VS_HUMAN_CPU:
        case GAME_VS_CPU_CPU:
            bowls[0] = bowl_create( 10, 0, -1, -1, -1,-1, blocks, qmark, config.player1.name, &config.player1.controls );
            if ( config.gametype != GAME_VS_CPU_CPU )
                bowls[1] = bowl_create( 220, 0, -1, -1, -1,-1, blocks, qmark, config.player2.name, &config.player2.controls );
            else
                bowls[1] = bowl_create( 220, 0, -1, -1, -1,-1, blocks, qmark, "CPU-1", 0 );
            if ( config.gametype == GAME_VS_HUMAN_HUMAN )
                bowls[2] = bowl_create( 430, 0, -1, -1, -1,-1, blocks, qmark, config.player3.name, &config.player3.controls );
            else
                bowls[2] = bowl_create( 430, 0, -1, -1, -1,-1, blocks, qmark, "CPU-2", 0 );
            break;
    }
    /* enable stats for one bowl games */
    if (config.gametype <= GAME_FIGURES)
        bowls[0]->show_stats = 1;
    /* background */
    tetris_recreate_bkgnd(1);

    /* shrapnells */
    shrapnells_init();
    return 1;
}
void tetris_clear()
{
    int i;

	if (next_blocks) {
		next_blocks_size = 0;
		free( next_blocks );
		next_blocks = NULL;
	}

    for ( i = 0; i < BOWL_COUNT; i++ ) {
        if ( bowls[i] ) 
            bowl_delete( bowls[i] );
        bowls[i] = 0;
    }
    /* shrapnells */
    shrapnells_delete();
}

/** Display FPS in console */
void show_fps(int fps) {
	if (!config.show_fps)
		return;
	SDL_Rect r = {0,0,40,20};
	char str[32];
	sprintf(str,"%d",fps);
	if (bowls[0]->font && sdl.screen) {
		DEST( sdl.screen, r.x, r.y , r.w, r.h );
		SOURCE( offscreen, r.x, r.y );
		blit_surf();

		bowls[0]->font->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
		write_text( bowls[0]->font, sdl.screen, 4, 4, str, OPAQUE );
		add_refresh_rect(r.x,r.y,r.w,r.h); /* XXX fixed size */
	}
}

/* Set bow controls by checking pressed/released keys and gamepad */
void tetris_set_bowl_controls(int i, SDL_Event *ev, BowlControls *bc)
{
	Bowl *b = bowls[i];
	Controls *ctrl = 0;

	memset(bc,0,sizeof(BowlControls));

	/* TODO improve this by using an array in config ... */
	if (i == 0)
		ctrl = &config.player1.controls;
	else if (i == 1)
		ctrl = &config.player2.controls;
	else if (i == 2)
		ctrl = &config.player3.controls;
	else return;

	if (!b->cpu_player) {
		if (keystate[ctrl->left])
			bc->lshift = CS_PRESSED;
		if (keystate[ctrl->right])
			bc->rshift = CS_PRESSED;
		if (keystate[ctrl->down])
			bc->sdrop = CS_PRESSED;

		if (ev->type == SDL_KEYDOWN) {
			if (ev->key.keysym.sym == ctrl->left)
				bc->lshift = CS_DOWN;
			if (ev->key.keysym.sym == ctrl->right)
				bc->rshift = CS_DOWN;
			if (ev->key.keysym.sym == ctrl->rot_left)
				bc->lrot = CS_DOWN;
			if (ev->key.keysym.sym == ctrl->rot_right)
				bc->rrot = CS_DOWN;
			if (ev->key.keysym.sym == ctrl->drop)
				bc->hdrop = CS_DOWN;
			if (ev->key.keysym.sym == ctrl->hold)
				bc->hold = CS_DOWN;
		}

		/* allow gamepad for bowl 0 */
		if (i == 0 && config.gp_enabled) {
			if (gamepad_ctrl_isdown(GPAD_LEFT))
				bc->lshift = CS_DOWN;
			else if (gamepad_ctrl_ispressed(GPAD_LEFT))
				bc->lshift = CS_PRESSED;
			if (gamepad_ctrl_isdown(GPAD_RIGHT))
				bc->rshift = CS_DOWN;
			else if (gamepad_ctrl_ispressed(GPAD_RIGHT))
				bc->rshift = CS_PRESSED;
			if (gamepad_ctrl_isactive(GPAD_DOWN))
				bc->sdrop = CS_PRESSED;

			if (ev->type == SDL_JOYBUTTONDOWN) {
				if (ev->jbutton.button == config.gp_lrot)
					bc->lrot = CS_DOWN;
				if (ev->jbutton.button == config.gp_rrot)
					bc->rrot = CS_DOWN;
				if (ev->jbutton.button == config.gp_hdrop)
					bc->hdrop = CS_DOWN;
				if (ev->jbutton.button == config.gp_hold)
					bc->hold = CS_DOWN;
			}
		}
	}

	/* do cpu move, soft drop is done in bowl_update because of timer */
	if (b->cpu_player) {
		/* left/right shift
		 * XXX we always set PRESSED skipping DOWN ... cheater! */
		if (b->cpu_dest_x > b->block.x)
			bc->rshift = CS_PRESSED;
		else if (b->cpu_dest_x < b->block.x)
			bc->lshift = CS_PRESSED;
		/* rotation
		 * XXX instant, no delay, but CPU is just damn fast isn't it? */
		if (b->cpu_dest_rot != b->block.rot_id)
			//if (delay_timed_out( &bowl->cpu_rot_delay, ms))
			bc->lrot = CS_DOWN;
		/* if CPU may drop in one p-cycle set key */
		if (bowl_cpu_may_drop(b))
			bc->hdrop = CS_DOWN;
	}
}

/*
====================================================================
Run a successfully initated game.
====================================================================
*/
void tetris_run()
{
    SDL_Event event;
    int leave = 0;
    int i;
    int ms;
    int request_pause = 0;
    int game_over = 0;
    int bkgnd_level = 0;
    int bowl_count; /* number of active bowls */
    char sshot_str[128];
    int screenshot_id = 0;
    int fps = 0, fpsStart, fpsCycles;
    int maxDelay;
    Uint32 cycle_start, delay;
    
    /* count number of bowls */
    bowl_count = 0;
    for ( i = 0; i < BOWL_COUNT; i++ )
        if ( bowls[i] ) 
            bowl_count++;
    
    SDL_ShowCursor( 0 );

	fpsStart = SDL_GetTicks();
	fpsCycles = 0;
	if (config.fps == 1)
		maxDelay = 20;
	else
		maxDelay = 17;

    /* main loop */
    fade_screen( FADE_IN, FADE_DEF_TIME );
    reset_timer();
    event_reset();
    while ( !leave && !term_game ) {
	    cycle_start = SDL_GetTicks();

	    event.type = SDL_NOEVENT;
        if ( event_poll( &event ) ) {
            switch ( event.type ) {
		case SDL_QUIT:
			term_game = 1;
		    break;
                case SDL_KEYUP:
                    if (game_over)
                        leave = 1;
                    else if (event.key.keysym.sym == config.pause_key)
                        request_pause = 1;
                    else switch ( event.key.keysym.sym ) {
                        case SDLK_F5:
                        	gamepad_close();
                        	gamepad_open();
                        	break;
                        case SDLK_ESCAPE: 
                            if ( confirm( large_font, _("End Game? y/n"), CONFIRM_YES_NO ) ) 
                                for ( i = 0; i < BOWL_COUNT; i++ )
                                    if ( bowls[i] && !bowls[i]->game_over )
                                        bowl_finish_game( bowls[i] );
                            break;
                         case SDLK_f:
#ifndef WIN32
                             /* switch fullscreen */
                            config.fullscreen = !config.fullscreen;
                            set_video_mode( config.fullscreen );
                            FULL_DEST( sdl.screen ); FULL_SOURCE( offscreen ); blit_surf();
                            refresh_screen( 0, 0, 0, 0 );
#endif
                            break;
                        case SDLK_TAB:
                            sprintf( sshot_str, "ss%i.bmp", screenshot_id++ );
                            SDL_SaveBMP( sdl.screen, sshot_str );
                            break;
                        default: break;
                    }
                    break;
                default: break;
            }
        }
        ms = get_time();
        for ( i = 0; i < BOWL_COUNT; i++ )
            if ( bowls[i] )
                bowl_hide( bowls[i] );
        shrapnells_hide();

        if ( request_pause && !game_over ) {
            for ( i = 0; i < BOWL_COUNT; i++ )
                if ( bowls[i] )
                    bowl_toggle_pause( bowls[i] );
            request_pause = 0;
        }
            
        gamepad_update();
        for ( i = 0; i < BOWL_COUNT; i++ )
            if ( bowls[i] ) {
                BowlControls bc;
                tetris_set_bowl_controls(i, &event, &bc);
                bowl_update( bowls[i], ms, &bc, game_over );
            }
        
        /* check if any of the bowls entered a new level and change background if so */
        if ( !config.keep_bkgnd ) {
            for ( i = 0; i < BOWL_COUNT; i++ )
                if ( bowls[i] )
                    if ( bowls[i]->level > bkgnd_level ) {
                        bkgnd_level = bowls[i]->level;
                        /* recreate background */
                        tetris_recreate_bkgnd( -1 );
                        refresh_screen( 0, 0, 0, 0 );
                        reset_timer();
                    }
        }
        
        shrapnells_update( ms );
        for ( i = 0; i < BOWL_COUNT; i++ )
            if ( bowls[i] )
                bowl_show( bowls[i] );
        shrapnells_show();

        refresh_rects();

        /* check if game's over */
        if ( !game_over ) {
            /* count number of finished bowls */
            game_over = 0;
            for ( i = 0; i < BOWL_COUNT; i++ )
                if ( bowls[i] && bowls[i]->game_over )
                    game_over++;
            if ( ( game_over == 1 && bowl_count == 1 ) || ( bowl_count > 1 && bowl_count - game_over <= 1 ) )
                game_over = 1;
            else
                game_over = 0;
        }

	/* stats */
	fpsCycles++;
	if (fpsStart < SDL_GetTicks())
		fps = ((10000 * fpsCycles / (SDL_GetTicks() - fpsStart))+5)/10;
	if (fpsCycles > 100) {
		fpsCycles = 0;
		fpsStart = SDL_GetTicks();
	}
	show_fps(fps);

	/* limit frame rate */
	delay = maxDelay - (SDL_GetTicks() - cycle_start);
	if (delay < 1)
		delay = 1;
	if (delay > maxDelay)
		delay = maxDelay;
	SDL_Delay(delay);
    }
    fade_screen( FADE_OUT, FADE_DEF_TIME );
    /* highscore entries */
    chart_clear_new_entries();
    for ( i = 0; i < BOWL_COUNT; i++ )
        if ( bowls[i] ) 
            chart_add( chart_set_query( gametype_ids[config.gametype] ),
        		    bowls[i]->name, bowls[i]->level,
			    counter_get( bowls[i]->score ) );
    SDL_ShowCursor( 1 );
}

/*
====================================================================
Run a number of CPU games to get an average score gained so you'll
see if your analyze algorithm in cpu.c cpu_analyze_bowl() sucks
or rocks!
====================================================================
*/
extern int CPU_SCORE_HOLE;
extern int CPU_SCORE_ALT;
extern int CPU_SCORE_LINE;
extern int CPU_SCORE_STEEP;
extern int CPU_SCORE_ABYSS;
extern int CPU_SCORE_BLOCK;
extern int count_occ( int *array, int size, int min, int max )
{
    int i, count;
    count = 0;
    for ( i = 0; i < size; i++ )
        if ( array[i] >= min && array[i] <= max )
            count++;
    return count;
}
void tetris_make_stat()
{
    int i;
    int game_count = 50;
    double total = 0;
    int total_lines = 0;
    double scores[1024];
    int lines[1024];
    Bowl *bowl = 0;
    SDL_Event event;
    int leave = 0;
    FILE *file = 0;
    
    printf( "*****\n" );
    
    bowl = bowl_create( 0, 0, -1, -1, -1, -1, blocks, qmark, "Demo", 0 );
    
    /* reset counters */
    total = 0; total_lines = 0;
    memset( lines, 0, sizeof( lines ) );
    memset( scores, 0, sizeof( scores ) );
        
    printf( "Computing: %3i %3i %3i %3i %3i %3i\n", CPU_SCORE_HOLE, CPU_SCORE_ALT, CPU_SCORE_LINE, CPU_SCORE_STEEP, CPU_SCORE_ABYSS, CPU_SCORE_BLOCK );
        
    for ( i = 0; i < game_count; i++ ) {
        if ( SDL_PollEvent( &event ) && event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE )
            leave = 1;
        bowl_quick_game( bowl, 1 );
        lines[i] = bowl->lines;
        total_lines += lines[i];
        scores[i] = bowl->score.value;
        total += scores[i];
        printf( "%3i: %5i: %14.0f\n", i, lines[i], scores[i] );
        if ( leave ) return;
    }
    
    if ( i != game_count ) game_count = i;
    if ( game_count <= 0 ) return;
    
    /* write to file */
    file = fopen( "stats", "a+" );
    fprintf( file, "SETTING: %3i %3i %3i %3i %3i %3i\n", CPU_SCORE_HOLE, CPU_SCORE_ALT, CPU_SCORE_LINE, CPU_SCORE_STEEP, CPU_SCORE_ABYSS, CPU_SCORE_BLOCK );
    for (i = 0; i < game_count;i++)
        fprintf( file, "%3i: %5i: %14.0f\n", i, lines[i], scores[i] );
    fprintf( file, "-----\n" );
    fprintf( file, "  0-100: %4i  101-200: %4i  201-400: %4i\n", 
             count_occ( lines, game_count, 0, 100 ), count_occ( lines, game_count, 101, 200 ), count_occ( lines, game_count, 201, 400 ) );
    fprintf( file, "401-600: %4i  601-800: %4i  rest:    %4i\n", 
             count_occ( lines, game_count, 401, 600 ), count_occ( lines, game_count, 601, 800 ), count_occ( lines, game_count, 801, 100000 ) );
    fprintf( file, "-----\n" );
    fprintf( file, "Avg.Lines: %i Avg.Score: %i\n", total_lines / game_count, (int)(total / game_count) );
    fprintf( file, "\n" );
    fclose( file );
    
    bowl_delete( bowl );
}
#ifdef _1
void tetris_make_stat()
{
    int i;
    int game_count = 10;
    double total = 0;
    int total_lines = 0;
    int scores[1024];
    int lines[1024];
    Bowl *bowl = 0;
    int ms = 4;
    SDL_Event event;
    int leave = 0;
    char str[256];
    
    font->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
    tetris_recreate_bkgnd( 0 );
    DEST( sdl.screen, 300, 0, 320, 480 ); fill_surf( 0x0 );
    refresh_screen( 0, 0, 0, 0 );
    if ( config.visualize )
        shrapnells_init();
    
    for ( i = 0; i < game_count; i++ ) {
        bowl = bowl_create( 0, 0, -1, -1, blocks, qmark, "Demo", 0 );
        if ( config.visualize )
            bowl->blind = 0;
        else
            bowl->blind = 1;
        bowl->mute = 1;
        while ( !bowl->game_over && !leave ) {
            if ( SDL_PollEvent( &event ) && event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE )
                leave = 1;
            bowl_update( bowl, ms, 0 );
            if ( config.visualize )
                refresh_rects();
        }
        lines[i] = bowl->lines;
        scores[i] = counter_get_target( bowl->score );
        sprintf( str, "%4i\n", lines[i] );
        write_text( font, sdl.screen, 310 + ( i % 7 ) * 42, 10 + ( i / 7 ) * 16, str, OPAQUE );
        refresh_screen( 0, 0, 0, 0 );
        total += scores[i];
        total_lines += bowl->lines;
        bowl_delete( bowl );
        printf( "%3i: %12i\n", i, scores[i] ); /* DEBUG */
        if ( leave ) break;
    }
    
    if ( config.visualize ) 
        shrapnells_delete();
    
    if ( i != game_count )
        game_count = i;
    if ( game_count <= 0 ) return;
    
    sprintf( str, "Avg Score: %i Avg. Lines: %i\n", (int)(total / game_count), total_lines / game_count );
    write_text( font, sdl.screen, 330, 428, str, OPAQUE );
    
    sprintf( str, "0-200: %i 201-400:: %i\n", count_occ( lines, game_count, 0, 200 ), count_occ( lines, game_count, 201, 400 ) );
    write_text( font, sdl.screen, 330, 444, str, OPAQUE );
    sprintf( str, "401-600: %i >601: %i\n", count_occ( lines, game_count, 401, 600 ), count_occ( lines, game_count, 601, 10000 ) );
    write_text( font, sdl.screen, 330, 460, str, OPAQUE );

    refresh_screen( 0, 0, 0, 0 );
        
    wait_for_click();
}
#endif
