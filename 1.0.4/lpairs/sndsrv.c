/***************************************************************************
                          sndsrv.c  -  description
                             -------------------
    begin                : Thu Apr 20 2000
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
#include "dynlist.h"
#include "sndsrv.h"

#ifdef SOUND

// Wave //

/*
    load a wave from file
*/
Wv* Wv_Ld(char *fname)
{
    Wv *w;
    char path[strlen(SRC_DIR) + strlen(fname) + 1];
    sprintf(path,"%s%s",SRC_DIR,fname);
    w = (Wv*)malloc(sizeof(Wv));
    if (w == 0) {
        fprintf(stderr, "ERR: wv_load: not enough memory\n");
        exit(1);
    }	
    w->buf = 0;
    w->len = 0;
    if (SDL_LoadWAV(path, &w->spc, &w->buf, &w->len) == 0) {
        fprintf(stderr, "ERR: wv_load: %s\n", SDL_GetError());
        exit(1);
    }
    w->spc.callback = 0;
    w->spc.userdata = 0;
    return w;
}

/*
    free wave
*/
void Wv_Fr(Wv *w)
{
    if (w->buf) SDL_FreeWAV(w->buf);
    free(w);
}

/*
    format a wave
    IS NOT WORKING YET!!!
*/

void Wv_Fmt(Wv *w, SDL_AudioSpec dest)
{
    SDL_AudioCVT cvt;

    printf("dst:\n");
    printf("ch: %i | smpls: %i | fmt: %i\n", dest.channels, dest.format, dest.samples);
    printf("src:\n");
    printf("ch: %i | smpls: %i | fmt: %i\n", w->spc.channels, w->spc.format, w->spc.samples);

    if (dest.channels != w->spc.channels || dest.format != w->spc.format || dest.samples != w->spc.samples) {
        SDL_BuildAudioCVT(&cvt, w->spc.format, w->spc.channels, w->spc.samples, dest.format, dest.channels, dest.samples);
        cvt.len = w->len;
        cvt.buf = (char*)malloc(cvt.len * cvt.len_mult);
        memcpy(cvt.buf, w->buf, w->len);
        SDL_ConvertAudio(&cvt);
        SDL_FreeWAV(w->buf);
        w->buf = cvt.buf;
    }
}

// Soundserver //

SSrv ssrv;

/*
    initialize soundserver with this format, frequency, channels and samples
    lmt is the limit of tracks mixed at one time
*/
int SSrv_Ini(int fmt, int frq, int ch, int smpls, int lmt)
{
    ssrv.spc.format = fmt;
    ssrv.spc.freq = frq;
    ssrv.spc.channels = ch;
    ssrv.spc.samples = smpls;
    ssrv.spc.callback = SSrv_CB;
    ssrv.spc.userdata = 0;
    ssrv.lmt = lmt;

    dl_init(&ssrv.trks, AUTO_DELETE | NO_CALLBACK, 0 );

    SSrv_StV(8);

    ssrv.ok = 1;
    if (SDL_OpenAudio(&ssrv.spc, 0) < 0) {
    	fprintf(stderr, "ERR: ssrv_open: %s\n", SDL_GetError());
    	ssrv.ok = 0;
    	return 1;
    }
    SSrv_Ps(0);

    ssrv.ply = 0;
    ssrv.slp = 0;
    return 0;
}

/*
    close soundserver
*/
void SSrv_Trm()
{
    if (ssrv.ok)
        SDL_CloseAudio();
    dl_clear(&ssrv.trks);
}

/*
    pause soundserver
*/
void SSrv_Ps(int p)
{
    if (ssrv.ok)
        SDL_PauseAudio(p);
    if (p) ssrv.ply = 0;
}

/*
    play a wave
    creates or overwrites a track
*/
void SSrv_Ply(Wv *w, int p)
{
    DL_Entry    *e = ssrv.trks.head.next;
    Trck    *t;
    if (!ssrv.ok || ssrv.slp) return;

    if (ssrv.trks.coun < ssrv.lmt) {
        // can create a new track //
        t = malloc(sizeof(Trck));
        t->wv = w;
        t->a_pos = w->buf;
        t->len = w->len;
        t->pri = p;
        dl_add(&ssrv.trks, t);
        ssrv.ply++;
        SSrv_Ps(0);
        return;
    }
    while (e != &ssrv.trks.tail) {
        // overwrite a track //
        t = (Trck*)e->data;
        if (t->pri < p) {
            t->wv = w;
            t->a_pos = w->buf;
            t->len = w->len;
            t->pri = p;
            ssrv.ply++;
            SSrv_Ps(0);
            return;
        }
        e = e->next;
    }
    return; //cannot play sound, no track available
}

/*
    set volume
*/
void SSrv_StV(char v)
{
    ssrv.vol = v < 0 ? 0 : v > 8 ? 8 : v;
    if (ssrv.vol) ssrv.vol = ssrv.vol * 16 - 1;
}

/*
    activate / deavctivate soundserver
*/
void SSrv_StA(int a)
{
    ssrv.slp = !a;
}

/*
    callback which mixes tracks and deletes finished tracks
*/
void SSrv_CB(void *udata, unsigned char *stream, int str_len)
{
    int     len;
    Trck    *t;
    DL_Entry    *e = ssrv.trks.head.next, *n;
    if (!ssrv.ok || ssrv.slp) return;
    while (e != &ssrv.trks.tail) {
        n = e->next;
        t = (Trck*)e->data;
        len = str_len < t->len ? str_len : t->len;
        SDL_MixAudio(stream, t->a_pos, len, ssrv.vol);
        t->len -= len;
        t->a_pos += len;
        if (t->len == 0) {
            dl_delete_entry(&ssrv.trks, e);
            ssrv.ply--;
        }
        e = n;
    }
    if (!ssrv.ply) SSrv_Ps(1);
}

#endif
