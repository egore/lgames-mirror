/***************************************************************************
                          sndsrv.h  -  description
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

#ifndef SNDSRV_H
#define SNDSRV_H

#ifdef __cplusplus
extern "C" {
#endif

/*
  *@author Michael Speck
  */

#ifdef SOUND

// Wave //
typedef struct {
    unsigned char   *buf;
    unsigned int    len;
    SDL_AudioSpec   spc;
} Wv;
Wv* Wv_Ld(char *fname);
void Wv_Fr(Wv *w);
void Wv_Fmt(Wv *w, SDL_AudioSpec sp);

// Track //
typedef struct {
    Wv            *wv;
    unsigned char	*a_pos;
    int             len;
    unsigned char   pri;
} Trck;

// Soundserver //
typedef struct {
    SDL_AudioSpec   spc; // spec //
    Dyn_List            trks; // tracks //
    char            vol;   // volume //
    int             ply; // playing somthing ? //
    int             slp; // sleeping ? //
    int             lmt; // lmt of tracks
    int             ok; // got sound ? //
} SSrv;
int  SSrv_Ini(int fmt, int frq, int ch, int smpls, int lmt);
void SSrv_Trm();
void SSrv_Ps(int p);
void SSrv_Ply(Wv *w, int p);
void SSrv_StV(char v);
void SSrv_StA(int s);
void SSrv_CB(void *udata, unsigned char *stream, int len);

#endif

#ifdef __cplusplus
};
#endif

#endif
