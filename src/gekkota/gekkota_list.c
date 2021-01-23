/******************************************************************************
 * @file    gekkota_list.c
 * @date    03-Nov-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include "gekkota_errors.h"
#include "gekkota_list.h"

GekkotaListIterator
gekkota_list_insert(GekkotaListIterator iterator, void_t *data)
{
    GekkotaListIterator element = (GekkotaListIterator) data;

    if (iterator == NULL || data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (element->previous != NULL || element->next != NULL)
    {
        errno = GEKKOTA_ERROR_OPERATION_NOT_VALID;
        return NULL;
    }

    element->previous = iterator->previous;
    element->next = iterator;
    element->previous->next = element;
    iterator->previous = element;

    return element;
}

void_t *
gekkota_list_remove(GekkotaListIterator iterator)
{
    if (iterator == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    iterator->previous->next = iterator->next;
    iterator->next->previous = iterator->previous;
    iterator->previous = iterator->next = NULL;

    return iterator;
}

size_t
gekkota_list_get_count(const GekkotaList *list)
{
    size_t count = 0;
    GekkotaListIterator iterator;

    if (list == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    for (iterator = gekkota_list_first(list);
        iterator != gekkota_list_tail(list);
        iterator = gekkota_list_next(iterator))
        ++count;
   
    return count;
}
