/***************************************************************************
                          dynlist.h  -  description
                             -------------------
    begin                : Sat Apr 8 2000
    copyright            : (C) 2000 by
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __DYNLIST_H
#define __DYNLIST_H

/*
this file provides a dynamic list.
*/

#ifdef __cplusplus
extern "C" {
#endif

/* flags indicating in which way entries are treated when its time to delete them... */
enum {
    NO_AUTO_DELETE = (0),
    AUTO_DELETE = (1L<<0),
    NO_CALLBACK = (1L<<1)
};

/* dynamic list entry */
typedef struct _DL_Entry {
    struct _DL_Entry    *next;
    struct _DL_Entry    *prev;
    void            *data;
} DL_Entry;

/* dynamic list */
typedef struct {
    unsigned int    flags;
    unsigned int    coun; //don't edit
    DL_Entry        head; //don't edit
    DL_Entry        tail; //don't edit
    void            (*callback)(void*);
    DL_Entry        *cur_entry;
} Dyn_List;

/* the following functions are used to work with dynamic lists.
there names are quite self-explaining, aren't they? */
void dl_init(Dyn_List *dlst, int flags, void (*callback)(void*));
int  dl_insert(Dyn_List *dlst, unsigned int i, void *item);
int  dl_add(Dyn_List *dlst, void *item);
int  dl_delete_entry(Dyn_List *dlst, DL_Entry *e);
int  dl_delete_poi(Dyn_List *dlst, void *item);
int  dl_delete_index(Dyn_List *dlst, unsigned int i);
void* dl_get(Dyn_List *dlst, int i);
DL_Entry* dl_get_entry(Dyn_List *dlst, void *item);
void dl_clear(Dyn_List *dlst);
/* these functions might be used after dl_get[_entry] was used or Dyn_List::cur_entry was set */
DL_Entry* dl_last_entry(Dyn_List *dlst);
DL_Entry* dl_next_entry(Dyn_List *dlst);

#ifdef __cplusplus
};
#endif

#endif
