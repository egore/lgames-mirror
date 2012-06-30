/***************************************************************************
                          menu.c  -  description
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
#include <stdlib.h>
#include <string.h>
#include "sdl.h"
#include "dynlist.h"
#include "sndsrv.h"
#include "menu.h"
#include "cfg.h"


extern Sdl sdl;
MMng mm;
extern Cfg cfg;

// menu entry //
/*
    free entry
*/
void ME_Fr(void *e)
{
    dl_clear(&((MEnt*)e)->sw);
    free(e);
}

/*
    all entries have these init steps in common
*/
void ME_Ini(MEnt *e, int t, char *n)
{
    dl_init(&e->sw, AUTO_DELETE | NO_CALLBACK, 0 );
    e->a = 255;
    strcpy(e->nm, n);
    e->t = t;
    e->dx = e->dy = e->dw = e->dh = 0;
    e->x = e->y = 0;
    e->cb = 0;
}

/*
    set range
*/
void ME_StRng(MEnt* e, int *p, int min, int max, int stp)
{
    e->p = p;
    e->min = min;
    e->max = max;
    e->stp = stp;
}

/*
    create an empty entry
*/
MEnt* ME_Crt()
{
    MEnt *e;
    e = malloc(sizeof(MEnt));
    if (e == 0) {
        printf("ME_Crt: Ups... not enough memory to create menu entry...\n");
        exit(1);
    }
    return e;
}

/*
    create separator
*/
MEnt* ME_CrtSep(char *s)
{
    MEnt *e = ME_Crt();

    ME_Ini(e, ME_SEP, s);

    return e;
}

/*
    create two switches group
*/
MEnt* ME_CrtSw2(char *n, int *p, char *cap1, char *cap2)
{
    char *sw;
    MEnt *e = ME_Crt();

    ME_Ini(e, ME_SWT, n);
    ME_StRng(e, p, 0, 1, 1);

    sw = malloc(32);
    strcpy(sw, cap1);
    dl_add(&e->sw, sw);
    sw = malloc(32);
    strcpy(sw, cap2);
    dl_add(&e->sw, sw);

    return e;
}

/*
    create multi switches group
*/
MEnt* ME_CrtSwX(char *n, int *p, char **caps, int num)
{
    int i;
    char *sw;
    MEnt *e = ME_Crt();

    ME_Ini(e, ME_SWT, n);
    ME_StRng(e, p, 0, num - 1, 1);

    for (i = 0; i < num; i++) {
        sw = malloc(32);
        strcpy(sw, caps[i]);
        dl_add(&e->sw, sw);
    }

    return e;
}

/*
    create range
*/
MEnt* ME_CrtRng(char *n, int *p, int min, int max, int stp)
{
    MEnt *e = ME_Crt();

    ME_Ini(e, ME_RNG, n);
    ME_StRng(e, p, min, max, stp);

    return e;
}

/*
    create action
*/
MEnt* ME_CrtAct(char *n, int a_id)
{
    MEnt *e = ME_Crt();

    ME_Ini(e, ME_ACT, n);
    e->act = a_id;

    return e;
}

/*
    create an edit
    min is the current length
    max the max length
    and p the char pointer
*/
MEnt* ME_CrtStr(char *n, char *p, int m)
{
    MEnt *e = ME_Crt();

    ME_Ini(e, ME_STR, n);

    e->max = m;
    e->min = strlen(p);
    e->p = (int*)p;

	return e;
}

/*
    create a submenu
*/
MEnt* ME_CrtSub(char *n, void *m)
{
    MEnt *e = ME_Crt();

    ME_Ini(e, ME_SUB, n);

    e->smn = m;

    return e;
}

/*
    create a key
*/
MEnt* ME_CrtKey(char *n, int *p)
{
    MEnt *e = ME_Crt();

    ME_Ini(e, ME_KEY, n);
    e->p = p;

    return e;
}

/*
    update the string and compute size
*/
void ME_SUpd(MEnt* e)
{
    char c;
	switch (e->t) {
		case ME_STR:
			sprintf(e->str, "%s: %s", e->nm, (char*)e->p);
			break;
		case ME_SWT:
			sprintf(e->str, "%s %s", e->nm, (char*)dl_get(&e->sw, *e->p));
			break;
		case ME_RNG:
			sprintf(e->str, "%s %i", e->nm, *e->p);
			break;
		case ME_KEY:
		    c = *((char*)e->p);
			if (c > 32 && c < 127)
				sprintf(e->str, "%s: '%c'", e->nm, c);
			else
				if (*e->p == 0)
					sprintf(e->str, "%s: ???", e->nm);
				else
					switch (*e->p){
						case SDLK_UP:
							sprintf(e->str, "%s: Up", e->nm);
							break;
						case SDLK_DOWN:
							sprintf(e->str, "%s: Down", e->nm);
							break;
						case SDLK_LEFT:
							sprintf(e->str, "%s: Left", e->nm);
							break;
						case SDLK_RIGHT:
							sprintf(e->str, "%s: Right", e->nm);
							break;
						case SDLK_SPACE:
							sprintf(e->str, "%s: Space", e->nm);
							break;
						default:
							sprintf(e->str, "%s: %i", e->nm, c);
					}
			break;
		default:
			strcpy(e->str, e->nm);
			break;
	}

	e->dw = text_width(mm.ft_nml, e->str);
	e->dh = mm.ft_nml->height;
	e->dx = e->x - (e->dw >> 1);
	e->dy = e->y - (e->dh >> 1);
}

/*
    edit string
*/
void ME_Edt(MEnt *e, int c, int u)
{
    if (e->t != ME_STR) return;
	if (c == SDLK_BACKSPACE && e->min > 0) {
		//clear last char
		((char*)e->p)[--e->min] = 0;
	}
	else
		if (c >= 32 && c < 128 && e->min < e->max) {
			//letter
			((char*)e->p)[e->min++] = u;
		}
}

/*
    change multi switches group
*/
void ME_CngSwX(MEnt *e, int *p, char **caps, int num)
{
    int i;
    char *sw;

    dl_clear(&e->sw);
    ME_StRng(e, p, 0, num - 1, 1);
    for (i = 0; i < num; i++) {
        sw = malloc(32);
        strcpy(sw, caps[i]);
        dl_add(&e->sw, sw);
    }
    *e->p = 0;
}

// menu //
/*
    create an empty menu
*/
Menu* M_Crt()
{
    Menu *m;
    m = malloc(sizeof(Menu));
    if (m == 0) {
        printf("M_Crt(): Ups... not enough memory to create a menu\n");
        exit(1);
    }
    m->c_e = 0;
    dl_init(&m->ent, AUTO_DELETE, ME_Fr );
    return m;
}

/*
    clear a menu
*/
void M_Fr(void *m)
{
    dl_clear(&((Menu*)m)->ent);
    free(m);
}

/*
    add an entry and select first entry as current
*/
void M_Add(Menu *m, MEnt *e)
{
    dl_add(&m->ent, e);
    if (m->c_e == 0 && e->t != ME_SEP) {
        m->c_e = e;
        m->c_e->a = 0;
    }
}

/*
    hide a menu
*/
void M_Hd(Menu *m)
{
    DL_Entry *e = m->ent.head.next;
    MEnt *me;

    while (e != &m->ent.tail) {
        me = (MEnt*)e->data;
        DEST(sdl.screen, me->dx, me->dy, me->dw, me->dh);
        SOURCE(mm.ss_bkgd, me->dx, me->dy);
        blit_surf();
    	add_refresh_rect(me->dx, me->dy, me->dw, me->dh);
        e = e->next;
    }
}

/*
    show a menu by drawing each entry
*/
void M_Shw(Menu *m)
{
    DL_Entry *e = m->ent.head.next;
    MEnt *me;

    M_SUpd(m);

    while (e != &m->ent.tail) {
        me = (MEnt*)e->data;
    	if (me == m->c_e) {
    		if (me->a > 0) {
    		    write_text(mm.ft_nml, sdl.screen, me->x, me->y, me->str, 0);
    		    write_text(mm.ft_sel, sdl.screen, me->x, me->y, me->str, (int)me->a);
    		}
    		else
    		    write_text(mm.ft_sel, sdl.screen, me->x, me->y, me->str, 0);
    	}
    	else {
    	    write_text(mm.ft_nml, sdl.screen, me->x, me->y, me->str, 0);
    		if (me->a < 255)
    		    write_text(mm.ft_sel, sdl.screen, me->x, me->y, me->str, (int)me->a);
    	}
    	add_refresh_rect(me->dx, me->dy, me->dw, me->dh);
    	e = e->next;
    }
}

/*
    do an ME_SUpd() for all entries
*/
void M_SUpd(Menu *m)
{
    DL_Entry  *e = m->ent.head.next;

    while (e != &m->ent.tail) {
        ME_SUpd((MEnt*)e->data);
        e = e->next;
    }
}

/*
    compute new alpha value for all entries
*/
void M_CmA(Menu *m, int ms)
{
    DL_Entry  *e = m->ent.head.next;
    MEnt    *me;

    while (e != &m->ent.tail) {
        me = (MEnt*)e->data;
    	if (me->a < 255  && me != m->c_e) {
		    me->a += mm.a_c * ms;
		    if (me->a > 255)
		        me->a = 255;
        }
        e = e->next;
    }
}

// menu manager //
/*
    initialize graphics, menu and positions of menu and logo
*/
void MM_Ini(int x, int y, int ly, SDL_Surface *ss_bk, SDL_Surface *ss_lg, Font *fn, Font *fs)
{
    mm.ss_bkgd = ss_bk;
    mm.ss_logo = ss_lg;
    mm.ft_nml = fn;
    mm.ft_sel = fs;
    mm.ft_nml->align = mm.ft_sel->align = ALIGN_X_CENTER | ALIGN_Y_CENTER;
    mm.x = x;
    mm.y = y + 10;
    if (ss_lg) {
        mm.ly = ly;
        mm.lx = (ss_bk->w - ss_lg->w) / 2;
    }
    dl_init( &mm.mn, AUTO_DELETE, M_Fr );
    mm.c_mn = 0;
    mm.a_c = 0.28;
#ifdef SOUND
    mm.s_clk = Wv_Ld("sound/click.wav");
#endif
}

/*
    show the menu
    sets a new video mode if wanted
*/
void MM_Shw(int rsz)
{
    if (rsz)
        set_video_mode(mm.ss_bkgd->w, mm.ss_bkgd->h, 16, SDL_HWSURFACE);
    FULL_DEST(sdl.screen);
    FULL_SOURCE(mm.ss_bkgd);
    blit_surf();
    if (mm.ss_logo) {
	    DEST(sdl.screen, mm.lx, mm.ly, mm.ss_logo->w, mm.ss_logo->h);
	    SOURCE(mm.ss_logo, 0, 0);
	    blit_surf();
    }
    if (cfg.dim)
        UNDIM_SCREEN();
    else
        refresh_screen( 0, 0, 0, 0 );
    if (mm.c_mn == 0 || mm.c_mn->ent.coun == 0) {
        fprintf(stderr, "MM_Shw(): Ups... no current or empty menu defined!\n");
        exit(1);
    }
    mm.c_mn->c_e->a = 0;
//    M_Shw(mm.c_mn);
    refresh_rects();
}

/*
    free memory used
*/
void MM_Trm()
{
    if (mm.ss_bkgd) SDL_FreeSurface(mm.ss_bkgd);
    if (mm.ss_logo) SDL_FreeSurface(mm.ss_logo);
    if (mm.ft_nml) free_font(mm.ft_nml);
    if (mm.ft_sel) free_font(mm.ft_sel);
    dl_clear(&mm.mn);
#ifdef SOUND
    if (mm.s_clk) Wv_Fr(mm.s_clk);
#endif
}

/*
    add a menu
*/
void MM_Add(Menu *m)
{
    dl_add(&mm.mn, m);
    if (mm.mn.coun == 1)
        mm.c_mn = m;
}

/*
    adjust position of all entries in all menus relative to mm.x | mm.y as centre
*/
void MM_AdjP()
{
    DL_Entry    *m = mm.mn.head.next;
    DL_Entry    *e;
    MEnt    *me;
    int     i;

    while (m != &mm.mn.tail) {
        e = ((Menu*)m->data)->ent.head.next;
        i = 0;
        while (e != &((Menu*)m->data)->ent.tail) {
            me = (MEnt*)e->data;
            me->x = mm.x;
            me->y = (mm.y - (((Menu*)m->data)->ent.coun * mm.ft_nml->height) / 2) + i * mm.ft_nml->height + mm.ft_nml->height / 2;
            i++;
            e = e->next;
        }
        m = m->next;
    }
}

/*
    allow default keys: alphanumerical and arrows
*/
void MM_DfVKys()
{
    int i;
	memset(mm.vkys, 0, sizeof(mm.vkys));
	for (i = 32; i < 128; i++)
		mm.vkys[i] = 1;
	mm.vkys[SDLK_UP] = 1;
	mm.vkys[SDLK_DOWN] = 1;
	mm.vkys[SDLK_RIGHT] = 1;
	mm.vkys[SDLK_LEFT] = 1;
	mm.vkys[SDLK_p] = 0;
	mm.vkys[SDLK_f] = 0;
	mm.vkys[SDLK_r] = 0;
}

/*
    checks if all menus contain at least one entry
    and if every menu has an current entry selected

    stuff like submenu in an entry is not checked
*/
void MM_Ck()
{
#ifdef DEBUG
    DL_E    *e = mm.mn.head.n;
    Menu    *m;

    printf("checking menu... ");

    // no menu at all? //
    if (mm.mn.cntr == 0) {
        printf("NOT ONE MENU FOUND!\n");
        exit(1);
    }
    if (mm.c_mn == 0) {
        printf("NO CURRENT MENU SELECTED!\n");
        exit(1);
    }

    while (e != &mm.mn.tail) {
        m = (Menu*)e->d;
        if (m->ent.cntr == 0) {
            printf("YOU'VE ADDED AN EMPTY MENU!\n");
            exit(1);
        }
        if (m->c_e == 0) {
            printf("NO CURRENT ENTRY SELECTED!\n");
            exit(1);
        }
        e = e->n;
    }
    printf("OK\n");
#endif
}

/*
    process incoming events
*/
int MM_Evt(SDL_Event *e)
{
    DL_Entry    *m, *le;
    MEnt    *me;
    int     ex = 0;

    // waiting for a key ? //
	if (mm.c_mn->c_e->t == ME_KEY && *mm.c_mn->c_e->p == 0) {
	    if (e->type == SDL_KEYUP && mm.vkys[e->key.keysym.sym]) {
	        // check other key entries for their value //
	        m = mm.mn.head.next;
	        while (!ex && m != &mm.mn.tail) {
	            le = ((Menu*)m->data)->ent.head.next;
	            while (!ex && le != &((Menu*)m->data)->ent.tail) {
	                me = (MEnt*)le->data;
	                if (me->t == ME_KEY && *me->p == e->key.keysym.sym)
	                    ex = 1;
	                le = le->next;
	            }
	            m = m->next;
	        }
	        // set key
	        if (!ex) {
                *mm.c_mn->c_e->p = e->key.keysym.sym;
                if (mm.c_mn->c_e->cb != 0)
                    mm.c_mn->c_e->cb();
            }
        }
        return MM_NONE;
    }
    // normal event
    switch (e->type) {
        case SDL_MOUSEMOTION:
            return MM_SelE(e->motion.x, e->motion.y);
        case SDL_MOUSEBUTTONUP:
            if (e->button.button == 1)
                return MM_UseE(MM_INC);
            else
                return MM_UseE(MM_DEC);
        case SDL_KEYUP:
            switch (e->key.keysym.sym) {
                case SDLK_RETURN:
                    return MM_UseE(MM_NONE);
                case SDLK_UP:
                    return MM_PrvE();
                case SDLK_DOWN:
                    return MM_NxtE();
                case SDLK_LEFT:
                    return MM_UseE(MM_DEC);
                case SDLK_RIGHT:
                    return MM_UseE(MM_INC);
                default:
                    if (mm.c_mn->c_e->t == ME_STR)
                        ME_Edt(mm.c_mn->c_e, e->key.keysym.sym, e->key.keysym.unicode);
                    return MM_NONE;
            }
        return MM_NONE;
    }
    return MM_NONE;
}

/*
    use an entry
    submenu: set submenu
    action: return action
    range: change pos due to c (inc or dec)
    str: add or delete char
*/
int MM_UseE(int c)
{
    MEnt *me = mm.c_mn->c_e;
    switch (me->t) {
        case ME_SUB:
#ifdef SOUND
            SSrv_Ply(mm.s_clk, 0);
#endif
            if (mm.c_mn->c_e->cb != 0)
                mm.c_mn->c_e->cb();
            mm.c_mn = (Menu*)me->smn;
            break;
        case ME_ACT:
#ifdef SOUND
            SSrv_Ply(mm.s_clk, 0);
#endif
            if (mm.c_mn->c_e->cb != 0)
                mm.c_mn->c_e->cb();
            return me->act;
        case ME_SWT:
        case ME_RNG:
#ifdef SOUND
            SSrv_Ply(mm.s_clk, 0);
#endif
            if (c == MM_DEC) {
        		*me->p -= me->stp;
            	if (*me->p < me->min)
           	        *me->p = me->max - ((me->max - me->min) % me->stp);
        	}	
        	else
        	    if (c == MM_INC) {
        			*me->p += me->stp;
        			if (*me->p > me->max)
        			    *me->p = me->min;
        		}	
            if (mm.c_mn->c_e->cb != 0)
                mm.c_mn->c_e->cb();
            break;
        case ME_KEY:
            *me->p = 0;
            if (mm.c_mn->c_e->cb != 0)
                mm.c_mn->c_e->cb();
            break;
    }
    return MM_NONE;
}

/*
    select prev entry
*/
int MM_PrvE()
{
    DL_Entry *e = dl_get_entry(&mm.c_mn->ent, mm.c_mn->c_e)->prev;

    while (e != &mm.c_mn->ent.head && ((MEnt*)e->data)->t == ME_SEP) e = e->prev;
    if (e != &mm.c_mn->ent.head) {
        mm.c_mn->c_e = (MEnt*)e->data;
        mm.c_mn->c_e->a = 0;
    }

#ifdef SOUND
    SSrv_Ply(mm.s_clk, 0);
#endif
    return MM_NONE;
}

/*
    select next entry
*/
int MM_NxtE()
{
    DL_Entry *e = dl_get_entry(&mm.c_mn->ent, mm.c_mn->c_e)->next;

    while (e != &mm.c_mn->ent.tail && ((MEnt*)e->data)->t == ME_SEP) e = e->next;
    if (e != &mm.c_mn->ent.tail) {
        mm.c_mn->c_e = (MEnt*)e->data;
        mm.c_mn->c_e->a = 0;
    }

#ifdef SOUND
    SSrv_Ply(mm.s_clk, 0);
#endif
    return MM_NONE;
}

/*
    select an entry at pos x,y if valid
*/
int MM_SelE(int x, int y)
{
    DL_Entry    *e = mm.c_mn->ent.head.next;
    MEnt    *me;
#ifdef SOUND
    MEnt    *old = mm.c_mn->c_e;
#endif

    while (e != &mm.c_mn->ent.tail) {
        me = (MEnt*)e->data;
        if (me->t != ME_SEP && x >= me->dx && x < me->dx + me->dw && y >= me->dy && y < me->dy + me->dh) {
            mm.c_mn->c_e = me;
            mm.c_mn->c_e->a = 0;
#ifdef SOUND
            if (mm.c_mn->c_e != old)
                SSrv_Ply(mm.s_clk, 0);
#endif
            return MM_NONE;
        }
        e = e->next;
    }
    return MM_NONE;
}

void MM_CB()
{
    DL_Entry    *e = mm.c_mn->ent.head.next;
    MEnt    *me;

    while (e != &mm.c_mn->ent.tail) {
        me = (MEnt*)e->data;
        if (me->cb != 0)
            me->cb();
        e = e->next;
    }
}
