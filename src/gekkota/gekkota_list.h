/******************************************************************************
 * @file    gekkota_list.h
 * @date    03-Nov-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_LIST_H__
#define __GEKKOTA_LIST_H__

#include "gekkota/gekkota_types.h"

typedef struct _GekkotaListNode
{
     struct _GekkotaListNode    *next;
     struct _GekkotaListNode    *previous;
} GekkotaListNode, *GekkotaListIterator;

typedef struct _GekkotaList
{
    struct _GekkotaListNode     sentinel;
} GekkotaList;

extern GekkotaListIterator
gekkota_list_insert(GekkotaListIterator iterator, void_t *data);

extern void_t *
gekkota_list_remove(GekkotaListIterator iterator);

extern size_t
gekkota_list_get_count(const GekkotaList *list);

#define gekkota_list_clear(list) \
    ((list)->sentinel.next = &(list)->sentinel); \
    ((list)->sentinel.previous = &(list)->sentinel);

#define gekkota_list_add(list, data) \
    (gekkota_list_insert(gekkota_list_tail(list), data))

#define gekkota_list_is_empty(list) \
    ((list)->sentinel.next == &(list)->sentinel)

#define gekkota_list_head(list) \
    ((list)->sentinel.next)

#define gekkota_list_tail(list) \
    (&(list)->sentinel)

#define gekkota_list_next(iterator) \
    ((void_t *) (iterator)->next)

#define gekkota_list_previous(iterator) \
    ((void_t *) (iterator)->previous)

#define gekkota_list_first(list) \
    ((void_t *) (list)->sentinel.next)

#define gekkota_list_last(list) \
    ((void_t *) (list)->sentinel.previous)

#endif /* !__GEKKOTA_LIST_H__ */
