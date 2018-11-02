/***************************************************************************
                          dynlist.c  -  description
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "dynlist.h"

/* initialize list */
void dl_init(Dyn_List *dlst, int flags, void (*callback)(void*))
{
    dlst->coun = 0;
    dlst->head.prev = dlst->tail.next = 0;
    dlst->head.next = &dlst->tail;
    dlst->tail.prev = &dlst->head;
    dlst->head.data = dlst->tail.data = 0;
    dlst->flags = flags;
    dlst->callback = callback;
    dlst->cur_entry = &dlst->head;

    /* check */
    if ( !(flags & NO_CALLBACK) && callback == 0 )
        fprintf( stderr, "dynamic list is asked to use callback but no function is specified...\n");
}

/* insert an item at index */
int dl_insert(Dyn_List *dlst, unsigned int index, void *item)
{
    int i;
    DL_Entry    *cur = &dlst->head;
    DL_Entry    *new_entry;

    if (index > dlst->coun) {
        fprintf(stderr, "ERR: dl_insert: index %i out of range...\n", index);
        return 1;
    }
    if (item == 0) {
        fprintf(stderr, "ERR: dl_insert: item is NULL...\n");
        return 1;
    }

    for (i = 0; i < index; i++)
        cur = cur->next;
    new_entry = (DL_Entry*)malloc(sizeof(DL_Entry));
    new_entry->data = item;
    new_entry->next = cur->next;
    new_entry->prev = cur;
    cur->next->prev = new_entry;
    cur->next = new_entry;
    dlst->coun++;

    return 0;
}

/* insert at the end of the list */
int dl_add(Dyn_List *dlst, void *item)
{
    return dl_insert(dlst, dlst->coun, item);
}

/* delete an entry */
int  dl_delete_entry(Dyn_List *dlst, DL_Entry *e)
{
    if (e == 0) {
        fprintf(stderr, "ERR: dl_delete: entry is NULL...\n");
        return 1;
    }
    if (dlst->coun == 0) {
        fprintf(stderr, "ERR: dl_delete: list is empty...\n");
        return 1;
    }
    if (e == &dlst->head || e == &dlst->tail) {
        fprintf(stderr, "ERR: dl_delete: trying to delete hd or tl..\n");
        return 1;
    }

    e->prev->next = e->next;
    e->next->prev = e->prev;
    dlst->coun--;

    if (dlst->flags & AUTO_DELETE) {
        if (dlst->flags & NO_CALLBACK)
            free(e->data);
        else {
            if (dlst->callback == 0) {
                fprintf(stderr, "ERR: dl_delete: no destroy callback installed...\n");
                free(e);
                return 1;
            }
            (dlst->callback)(e->data);
        }
    }
    free(e);

    return 0;
}

/* delete entry containing this item */
int dl_delete_poi(Dyn_List *dlst, void *item)
{
    int         i;
    DL_Entry    *cur = &dlst->head;

    if (item == 0) {
        fprintf(stderr, "ERR: dl_delete: item is NULL...\n");
        return 1;
    }
    if (dlst->coun == 0) {
        fprintf(stderr, "ERR: dl_delete: list is empty...\n");
        return 1;
    }

    for (i = 0; i <= dlst->coun; i++)
        if (cur->next != &dlst->tail) {
            cur = cur->next;
            if (cur->data == item)
                break;
        }
        else {
            fprintf(stderr, "ERR: dl_delete: list does not contain item 0x%lu...\n", (uintptr_t)item);
            return 1;
        }

    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    dlst->coun--;
    cur->next = cur->prev = 0;

    if (dlst->flags & AUTO_DELETE) {
        if (dlst->flags & NO_CALLBACK)
            free(cur->data);
        else {
            if (dlst->callback == 0) {
                fprintf(stderr, "ERR: dl_delete: no destroy callback installed...\n");
                free(cur);
                return 1;
            }
            (dlst->callback)(cur->data);
        }
    }
    free(cur);

    return 0;
}

/* delete item at index */
int dl_delete(Dyn_List *dlst, unsigned int index)
{
    int         i;
    DL_Entry    *cur = &dlst->head;

    if (index >= dlst->coun) {
        fprintf(stderr, "ERR: dl_delete: index %i out of range...\n", index);
        return 1;
    }
    if (dlst->coun == 0) {
        fprintf(stderr, "ERR: dl_delete: list is empty...\n");
        return 1;
    }

    for (i = 0; i <= index; i++)
        cur = cur->next;

    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    dlst->coun--;
    cur->next = cur->prev = 0;

    if (dlst->flags & AUTO_DELETE) {
        if (dlst->flags & NO_CALLBACK)
            free(cur->data);
        else {
            if (dlst->callback == 0) {
                fprintf(stderr, "ERR: dl_delete: no destroy callback installed...\n");
                free(cur);
                return 1;
            }
            (dlst->callback)(cur->data);
        }
    }
    free(cur);

    return 0;
}

/* get the item with index 'index' */
void* dl_get(Dyn_List *dlst, int index)
{
    unsigned int i;
    DL_Entry *cur = &dlst->head;

    if (index >= dlst->coun) {
        fprintf(stderr, "ERR: dl_get: index %i out of range...\n", index);
        return 0;
    }

    for (i = 0; i <= (unsigned)index; i++)
        cur = cur->next;

    dlst->cur_entry = cur;
    if ( cur->data == 0 )
        fprintf( stderr, "uhhh... this data pointer is 0! hope you'll check this...\n" );
    return cur->data;
}

/* get the current entry from a pointer */
DL_Entry *dl_get_entry(Dyn_List *dlst, void *item)
{
    int         i;
    DL_Entry    *cur = &dlst->head;

    if (item == 0) {
        fprintf(stderr, "ERR: DL_GetE: item is NULL...\n");
        return 0;
    }
    if (dlst->coun == 0) {
        fprintf(stderr, "ERR: DL_GetE: list is empty...\n");
        return 0;
    }

    for (i = 0; i <= dlst->coun; i++)
        if (cur->next != &dlst->tail) {
            cur = cur->next;
            if (cur->data == item)
                break;
        }
        else {
            fprintf(stderr, "ERR: DL_GetE: list does not contain item 0x%lu...\n", (uintptr_t)item);
            return 0;
        }

    dlst->cur_entry = cur;
    if ( cur == 0 )
        fprintf( stderr, "uhhh... this entry pointer is 0! hope you'll check this...\n" );
    return cur;
}

/* clear all entries of list */
void dl_clear(Dyn_List *dlst)
{
    DL_Entry *entry = dlst->head.next;
    DL_Entry *next_entry;

    while ( entry != &dlst->tail) {

        next_entry = entry->next;
        dl_delete_entry( dlst, entry );
        entry = next_entry;

    }

}

/* these functions might be used after dl_get[_entry] was used or Dyn_List::cur_entry was set */
DL_Entry* dl_last_entry(Dyn_List *dlst)
{
    DL_Entry *entry;

    if ( dlst->cur_entry == &dlst->head ) {

        fprintf( stderr, "uhhh... this entry pointer is 0! hope you'll check this...\n" );
        return 0;

    }

    dlst->cur_entry = dlst->cur_entry->prev;
    if ( dlst->cur_entry == &dlst->head )
        entry = 0;
    else
        entry = dlst->cur_entry;

    if ( entry == 0 )
        fprintf( stderr, "uhhh... this entry pointer is 0! hope you'll check this...\n" );

    return entry;
}

DL_Entry* dl_next_entry(Dyn_List *dlst)
{
    DL_Entry *entry;

    if ( dlst->cur_entry == &dlst->tail ) {

        fprintf( stderr, "uhhh... this entry pointer is 0! hope you'll check this...\n" );
        return 0;

    }

    dlst->cur_entry = dlst->cur_entry->next;
    if ( dlst->cur_entry == &dlst->tail )
        entry = 0;
    else
        entry = dlst->cur_entry;

    if ( entry == 0 )
        fprintf( stderr, "uhhh... this entry pointer is 0! hope you'll check this...\n" );

    return entry;
}

