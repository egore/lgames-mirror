/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "manager.h"
#include "../game/game.h"
#include "file.h"
#include "chart.h"
#include "event.h"
#include "config.h"
#include "../gui/stk.h"
#include "theme.h"
#include "slot.h"

/** Basic information about levelset and screenshot of first level */
typedef struct {
	char		*name;
	char		*version;
	char		*author;
	int		num_levels;
	SDL_Surface	*thumbnail;
} set_info_t;

/** Button displaying a levelsetname or up/down */
typedef struct {
	SDL_Rect	region;
#define SELECTID_UNUSED -4
#define	SELECTID_PREV -3
#define SELECTID_NEXT -2
#define SELECTID_EXIT -1
	int		id; /* special id or index in set_infos */
#define MAXLABELLEN 20
	char		label[MAXLABELLEN];
	int		focus;
	uint32_t	last_focus_time; /* for animation; not used now */
} select_button_t;

/** Select dialog object */
typedef struct {
	int		initialized; /* resources loaded */
	SDL_Surface	*background; /* background of dialog */
	StkFont		*standard_font;
	StkFont		*caption_font;
	StkFont		*highlight_font;
	int		num_set_infos;
	set_info_t	*set_infos; /* information about all levelsets */
#define NUMSELECTBUTTONS 22
#define SETBUTTON_START_ID 1
#define SETBUTTON_END_ID 19
	select_button_t	select_buttons[NUMSELECTBUTTONS];
	char		*selected_set; /* pointer to name in set_infos */
} setselect_dlg_t;
setselect_dlg_t ssd;

extern char **levelset_names_local;
extern int levelset_count_local;
extern SDL_Surface *stk_display;
extern SDL_Surface *extra_pic;
extern SDL_Surface *brick_pic;
extern int stk_quit_request;
extern Config config;

/** Load basic information of levelset @sname (preceded by ~ for set in 
 * home directory) into struct @si. Also generate a small preview thumbnail
 * of first level. */
static void load_set_info( set_info_t *si, const char *sname )
{
	/* FIXME for now just put in name */
	memset( si, 0, sizeof( set_info_t ));
	si->name = strdup(sname);
}

/** Update set select buttons using set infos starting at index @set_id.
 * The buttons outside range (prev, next, exit) are not touched. */
static void update_select_buttons( int set_id )
{
	int i;
	
	for (i = SETBUTTON_START_ID; i <= SETBUTTON_END_ID; i++) {
		select_button_t *btn = &ssd.select_buttons[i];
		
		btn->focus = 0;
		btn->last_focus_time = 0;
		
		if (set_id >= ssd.num_set_infos) {
			btn->id = SELECTID_UNUSED;
			strcpy(btn->label,"");
			continue;
		}
		
		btn->id = set_id;
		snprintf(btn->label, MAXLABELLEN, "%s",
					ssd.set_infos[set_id].name);
		set_id++;
	}
}

/** Load/Free resources. */
void setselect_create()
{
	int i, x, y;
	char *ptr;
	
	if (ssd.initialized)
		return; /* already done */
	
	memset(&ssd, 0, sizeof(ssd));
	
	ssd.standard_font = stk_font_load( SDL_SWSURFACE, "f_small_yellow.png" );
	SDL_SetColorKey( ssd.standard_font->surface, SDL_SRCCOLORKEY, 
		stk_surface_get_pixel( ssd.standard_font->surface, 0,0 ) );	
	ssd.highlight_font = stk_font_load( SDL_SWSURFACE, "f_small_white.png" );
	SDL_SetColorKey( ssd.highlight_font->surface, SDL_SRCCOLORKEY, 
		stk_surface_get_pixel( ssd.highlight_font->surface, 0,0 ) );	
	ssd.caption_font = stk_font_load( SDL_SWSURFACE, "f_yellow.png" );
	SDL_SetColorKey( ssd.caption_font->surface, SDL_SRCCOLORKEY, 
		stk_surface_get_pixel( ssd.caption_font->surface, 0,0 ) );
	
	/* background -- will be filled when running dialog */
	ssd.background = stk_surface_create( SDL_SWSURFACE, stk_display->w, 
							stk_display->h );
	SDL_SetColorKey( ssd.background, 0, 0 );
	
	/* position select buttons */
	x = 50; y = 50;
	for (i = 0; i < NUMSELECTBUTTONS; i++) {
		select_button_t *sb = &ssd.select_buttons[i];
		
		sb->region.x = x;
		sb->region.y = y;
		sb->region.w = 120;
		sb->region.h = ssd.standard_font->height;
		
		if (i == 0) {
			sb->id = SELECTID_PREV;
			strcpy(sb->label,_("<previous>"));
		} else if (i == NUMSELECTBUTTONS - 2) {
			sb->id = SELECTID_NEXT;
			strcpy(sb->label,_("<next>"));
		} else if (i == NUMSELECTBUTTONS - 1) {
			sb->id = SELECTID_EXIT;
			strcpy(sb->label,_("<exit>"));
		} else {
			sb->id = SELECTID_UNUSED;
			strcpy(sb->label,_("<not set>"));
		}
		
		sb->focus = 0;
		sb->last_focus_time = 0;
		
		y += ssd.standard_font->height + 5;
	}
	
	/* levelset infos */
	ssd.num_set_infos = levelset_count_local;
	ssd.set_infos = calloc( ssd.num_set_infos, sizeof(set_info_t) );
	for (i = 0; i < ssd.num_set_infos; i++) {
		set_info_t *si = &ssd.set_infos[i];
		load_set_info( si, levelset_names_local[i] );
	}
	update_select_buttons(0);

	ssd.initialized = 1;
}
void setselect_delete()
{
	int i;
	
	if (!ssd.initialized)
		return;
	
	stk_font_free( &ssd.standard_font );
	stk_font_free( &ssd.caption_font );
	stk_font_free( &ssd.highlight_font );
	stk_surface_free( &ssd.background );
	
	if (ssd.set_infos) {
		for (i = 0; i < ssd.num_set_infos; i++) {
			set_info_t *si = &ssd.set_infos[i];
			if (si->name)
				free(si->name);
			if (si->author)
				free(si->author);
			if (si->version)
				free(si->version);
			if (si->thumbnail)
				SDL_FreeSurface(si->thumbnail);
		}
		free(ssd.set_infos);
		ssd.set_infos = NULL;
	}
	
	ssd.initialized = 0;
}

/** Set background from current screen */
static void set_background()
{
	SDL_Surface *buffer = stk_surface_create(SDL_SWSURFACE,
					stk_display->w, stk_display->h);
	
	stk_surface_blit( stk_display, 0,0,-1,-1, buffer, 0,0 );
	SDL_SetColorKey(buffer, 0, 0);
	stk_surface_gray( stk_display, 0,0,-1,-1, 1 );
	stk_surface_blit( stk_display, 0,0,-1,-1, ssd.background, 0,0 );
	
	SDL_FreeSurface( buffer );
	
}

/** Draw buttons list. If @refresh is True update screen. */
static void draw_buttons( int refresh )
{
	int i;
	StkFont *font = NULL;
	
	for (i = 0; i < NUMSELECTBUTTONS; i++) {
		select_button_t *btn = &ssd.select_buttons[i];
		stk_surface_blit( ssd.background, btn->region.x, btn->region.y,
				btn->region.w, btn->region.h,
				stk_display, btn->region.x, btn->region.y);
		font = ssd.standard_font;
		if (btn->focus)
			font = ssd.highlight_font;
		stk_font_write(font, stk_display, btn->region.x, btn->region.y,
						STK_OPAQUE, btn->label);
	}
	
	if (refresh) {
		SDL_Rect region = { 
			ssd.select_buttons[0].region.x, 
			ssd.select_buttons[0].region.y,
			ssd.select_buttons[NUMSELECTBUTTONS-1].region.w,
			ssd.select_buttons[NUMSELECTBUTTONS-1].region.y +
			ssd.select_buttons[NUMSELECTBUTTONS-1].region.h - 
			ssd.select_buttons[0].region.y };
		stk_display_store_rect( &region );
		stk_display_update( STK_UPDATE_RECTS );
	}
}

/** Draw everything. */
static void draw_all()
{
	stk_surface_blit( ssd.background, 0,0,-1,-1, stk_display, 0,0 );
	draw_buttons(0);
	stk_display_update( STK_UPDATE_ALL );
}

/** Handle mouse motion to position @x,@y. Redraw all buttons. */
static void handle_motion( int x, int y )
{
	int i;
	
	/* check button focus */
	for (i = 0; i < NUMSELECTBUTTONS; i++) {
		select_button_t *sb = &ssd.select_buttons[i];
		
		if (FOCUS_RECT(x,y,sb->region))
			sb->focus = 1;
		else
			sb->focus = 0;
	}
	/* redraw */
	draw_buttons(1);
}

/** Handle mouse button click on position @x,@y. Return 1 if either Quit
 * button or levelset has been clicked, 0 otherwise. If set has been 
 * selected store it in ssd::selected_set. */
static int handle_click( int x, int y)
{
	int i, id;
	int num_setbuttons = SETBUTTON_END_ID - SETBUTTON_START_ID + 1;
	select_button_t *sb = NULL;
	
	/* get clicked button */
	for (i = 0; i < NUMSELECTBUTTONS; i++) {
		if (FOCUS_RECT(x,y,ssd.select_buttons[i].region)) {
			sb = &ssd.select_buttons[i];
			break;
		}
	}
	if (sb == NULL)
		return 0; /* no button clicked */
	
	if (sb->id == SELECTID_EXIT)
		return 1;
	if (sb->id == SELECTID_PREV) {
		/* scroll list up */
		id = ssd.select_buttons[SETBUTTON_START_ID].id;
		if (id == 0) {
			/* go to end of list */
			id = ssd.num_set_infos - num_setbuttons;
		} else {
			id -= num_setbuttons;
			if (id < 0)
				id = 0;
		}
		update_select_buttons(id);
		draw_buttons(1);
		return 0;
	}
	if (sb->id == SELECTID_NEXT) {
		/* scroll list down */
		id = ssd.select_buttons[SETBUTTON_START_ID].id;
		if (id == ssd.num_set_infos - num_setbuttons) {
			/* go to begin of list */
			id = 0;
		} else {
			id += num_setbuttons;
			if (id > ssd.num_set_infos - num_setbuttons)
				id = ssd.num_set_infos - num_setbuttons;
		}
		update_select_buttons(id);
		draw_buttons(1);
		return 0;
	}
	ssd.selected_set = ssd.set_infos[sb->id].name;
	return 1;
}

/** Main loop, run dialog and return pointer to name of selected set or
 * NULL if none selected. */
const char * setselect_run()
{
	SDL_Event event;
	int leave = 0;
	SDL_EventFilter old_filter;
	
	/* backup current filter (which takes out motion events) as we work	
	 * with WaitEvent() for the moment; see manager_run() how it is done
	 * for polling events. */
	old_filter = SDL_GetEventFilter();
	SDL_SetEventFilter( 0 );
	
	/* TODO: reload set infos if necessary */
	
	ssd.selected_set = NULL;
	set_background();
	handle_motion(0,0); /* clear old highlighting */
	draw_all();
		
	while ( !leave && !stk_quit_request ) {
		SDL_WaitEvent( &event );
		switch ( event.type ) {
			case SDL_QUIT: 
				stk_quit_request = 1;
				break;
			case SDL_MOUSEMOTION:
				handle_motion( event.motion.x,
							event.motion.y );
				break;
			case SDL_MOUSEBUTTONUP:
				if (handle_click( event.button.x,
							event.button.y ))
					leave = 1;
				break;
		}
	}
	
	/* restore event filter */
	SDL_SetEventFilter(old_filter);
	
	return ssd.selected_set; /* is a pointer to ssd.set_infos */
}
