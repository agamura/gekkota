/******************************************************************************
 * @file    gekkota_lzf.c
 * @date    22-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <math.h>
#include <string.h>
#include "gekkota.h"
#include "gekkota_errors.h"
#include "gekkota_memory.h"
#include "gekkota_lzf.h"

#define GEKKOTA_MAX_LITERAL     (1 << 5)
#define GEKKOTA_MAX_OFFSET      (1 << 13)
#define GEKKOTA_MAX_REFERENCE   (1 << 8) + (1 << 3)

struct _GekkotaLZF
{
    GekkotaCompressionLevel compressionLevel;
    GekkotaBuffer           hashTable;
    uint32_t                refCount;
};

static inline int32_t
gekkota_lzf_index(const GekkotaLZF *lzf, int32_t value);

static inline int32_t
gekkota_lzf_first(byte_t *data, int32_t index);

static inline int32_t
gekkota_lzf_next(int32_t value, byte_t *data, int32_t index);

GekkotaLZF *
gekkota_lzf_new_0(GekkotaLZF *restrict lzf)
{
    if (lzf == NULL)
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
    else
        ++lzf->refCount;

    return lzf;
}

GekkotaLZF *
gekkota_lzf_new_1(GekkotaCompressionLevel compressionLevel)
{
    GekkotaLZF *lzf;

    if (compressionLevel < GEKKOTA_COMPRESSION_LEVEL_FASTEST ||
            compressionLevel > GEKKOTA_COMPRESSION_LEVEL_BEST)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return NULL;
    }

    if ((lzf = gekkota_memory_alloc(sizeof(GekkotaLZF), TRUE)) == NULL)
        return NULL;

    if (gekkota_lzf_set_compression_level(lzf, compressionLevel) != 0)
    {
        gekkota_memory_free(lzf);
        return NULL;
    }

    lzf->refCount = 1;

    return lzf;
}

int32_t
gekkota_lzf_destroy(GekkotaLZF *lzf)
{
    if (lzf == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (--lzf->refCount == 0)
    {
        gekkota_memory_free(lzf->hashTable.data);
        gekkota_memory_free(lzf);
    }
    else
        return lzf->refCount;

    return 0;
}

GekkotaCompressionLevel
gekkota_lzf_get_compression_level(const GekkotaLZF *lzf)
{
    if (lzf == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return GEKKOTA_COMPRESSION_LEVEL_UNDEFINED;
    }

    return lzf->compressionLevel;
}

int32_t
gekkota_lzf_set_compression_level(
        GekkotaLZF *restrict lzf,
        GekkotaCompressionLevel compressionLevel)
{
    GekkotaBuffer hashTable;

    if (lzf == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (compressionLevel < GEKKOTA_COMPRESSION_LEVEL_FASTEST ||
            compressionLevel > GEKKOTA_COMPRESSION_LEVEL_BEST)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    if (compressionLevel != lzf->compressionLevel)
    {
        hashTable.length = (1 << compressionLevel) * sizeof(int32_t);

        if ((hashTable.data = gekkota_memory_alloc(hashTable.length, FALSE)) == NULL)
            return -1;

        if (lzf->hashTable.data != NULL)
            gekkota_memory_free(lzf->hashTable.data);

        lzf->compressionLevel = compressionLevel;
        lzf->hashTable.data = hashTable.data;
        lzf->hashTable.length = hashTable.length;
    }

    return 0;
}

int32_t
gekkota_lzf_get_inflated_length(
        const GekkotaLZF *lzf,
        const GekkotaBuffer *deflated)
{
    if (lzf == NULL || deflated == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (deflated->data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_BUFFER;
        return -1;
    }

    if (deflated->length == 0)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return -1;
    }

    if (deflated->length < sizeof(size_t) + 1)
    {
        errno = GEKKOTA_ERROR_BUFFER_TOO_SMALL;
        return -1;
    }
        
    return gekkota_net_to_host_32(
            *(int32_t *) (((byte_t *) deflated->data)
                + deflated->length - sizeof(size_t)));
}

int32_t
gekkota_lzf_get_max_deflated_length(
        const GekkotaLZF *lzf,
        const GekkotaBuffer *source)
{
    if (lzf == NULL || source == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (source->data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_BUFFER;
        return -1;
    }

    if (source->length == 0)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return -1;
    }

    /*
     * The buffer that stores deflated data must be at least 0.1 % larger than
     * the source buffer + 12 bytes.
     */
    return (int32_t) ceil((source->length * 1.001) + 12.0);
}

int32_t
gekkota_lzf_deflate(
        const GekkotaLZF *lzf,
        const GekkotaBuffer *source,
        GekkotaBuffer *restrict deflated)
{
    byte_t *sourceData, *deflatedData;
    size_t deflatedLength;
    int32_t literal = 0, offset, reference;
    int32_t i = 0, j = 0, k;
    int32_t value;

    /*
     * [source] is validated by function gekkota_lzf_get_max_deflated_length().
     */

    if (lzf == NULL || deflated == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (deflated->data != NULL && deflated->length == 0)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return -1;
    }

    deflatedLength = (size_t) gekkota_lzf_get_max_deflated_length(lzf, source);

    if (deflated->data != NULL && deflated->length < deflatedLength)
    {
        errno = GEKKOTA_ERROR_BUFFER_TOO_SMALL;
        return -1;
    }
    else if (deflated->data == NULL)
    {
        if (gekkota_buffer_malloc(deflated, deflatedLength, FALSE) != 0)
            return -1;
    }

    memset(lzf->hashTable.data, 0x00, lzf->hashTable.length);
    sourceData = (byte_t *) source->data;
    deflatedData = (byte_t *) deflated->data;

    value = gekkota_lzf_first(sourceData, i);

    while (TRUE)
    {
        if (i < (int32_t) source->length - 2)
        {
            value = gekkota_lzf_next(value, sourceData, i);
            k = gekkota_lzf_index(lzf, value);
            reference = ((int32_t *) lzf->hashTable.data)[k];
            ((int32_t *) lzf->hashTable.data)[k] = i;

            if ((offset = i - reference - 1) < GEKKOTA_MAX_OFFSET &&
                    i + 4 < (int32_t) source->length && reference > 0 &&
                    sourceData[reference] == sourceData[i] &&
                    sourceData[reference + 1] == sourceData[i + 1] &&
                    sourceData[reference + 2] == sourceData[i + 2])
            {
                int32_t length = 2;
                int32_t maxLength = (int32_t) source->length - i - length;

                if (maxLength > GEKKOTA_MAX_REFERENCE) maxLength = GEKKOTA_MAX_REFERENCE;

                do
                {
                    length++;
                }
                while (length < maxLength &&
                        sourceData[reference + length] ==
                        sourceData[i + length]);

                if (literal != 0)
                {
                    deflatedData[j++] = (byte_t) (literal - 1);
                    literal = -literal;

                    do
                    {
                        deflatedData[j++] = sourceData[i + literal];
                    }
                    while (++literal != 0);
                }

                length -= 2;
                ++i;

                if (length < 7)
                    deflatedData[j++] = (byte_t) ((offset >> 8) + (length << 5));
                else
                {
                    deflatedData[j++] = (byte_t) ((offset >> 8) + (7 << 5));
                    deflatedData[j++] = (byte_t) (length - 7);
                }

                deflatedData[j++] = (byte_t) offset;

                i += length - 1;
                value = gekkota_lzf_first(sourceData, i);

                value = gekkota_lzf_next(value, sourceData, i);
                ((int32_t *) lzf->hashTable.data)[gekkota_lzf_index(lzf, value)] = i;
                ++i;

                value = gekkota_lzf_next(value, sourceData, i);
                ((int32_t *) lzf->hashTable.data)[gekkota_lzf_index(lzf, value)] = i;
                ++i;

                continue;
            }
        }
        else if (i == source->length) break;

        ++literal;
        ++i;

        if (literal == GEKKOTA_MAX_LITERAL)
        {
            deflatedData[j++] = (byte_t) (GEKKOTA_MAX_LITERAL - 1);
            literal = -literal;

            do
            {
                deflatedData[j++] = sourceData[i + literal];
            }
            while (++literal != 0);
        }
    }

    if (literal != 0) {
        deflatedData[j++] = (byte_t) (literal - 1);
        literal = -literal;

        do
        {
            deflatedData[j++] = sourceData[i + literal];
        }
        while (++literal != 0);
    }

    /*
     * Embed original data length.
     */
    *((size_t *) &deflatedData[j]) = (size_t) gekkota_host_to_net_32(
            (uint32_t) source->length);

    return j + sizeof(size_t);
}

int32_t
gekkota_lzf_inflate(
        const GekkotaLZF *lzf,
        const GekkotaBuffer *source,
        GekkotaBuffer *restrict inflated)
{
    byte_t *sourceData, *inflatedData;
    size_t inflatedLength;
    int i = 0, j = 0;

    /*
     * [source] is validated by function gekkota_lzf_get_inflated_length().
     */

    if (lzf == NULL || inflated == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (inflated->data != NULL && inflated->length == 0)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return -1;
    }

    inflatedLength = gekkota_lzf_get_inflated_length(lzf, source);

    if (inflated->data != NULL && inflated->length < inflatedLength)
    {
        errno = GEKKOTA_ERROR_BUFFER_OVERFLOW;
        return -1;
    }
    else if (inflated->data == NULL)
    {
        if (gekkota_buffer_malloc(inflated, inflatedLength, FALSE) != 0)
            return -1;
    }

    sourceData = (byte_t *) source->data;
    inflatedData = (byte_t *) inflated->data;

    do
    {
        int32_t value = sourceData[i++];

        if (value < (1 << 5))
        {
            ++value;

            do
            {
                inflatedData[j++] = sourceData[i++];
            }
            while (--value != 0);
        }
        else
        {
            int32_t length = value >> 5;
            int32_t reference = (int32_t) (j - ((value & 0x1F) << 8) - 1);

            if (length == 7)
                length += sourceData[i++];

            reference -= sourceData[i++];

            inflatedData[j++] = inflatedData[reference++];
            inflatedData[j++] = inflatedData[reference++];

            do
            {
                inflatedData[j++] = inflatedData[reference++];
            }
            while (--length != 0);
        }
    }
    while (j < (int32_t) inflated->length && i < (int32_t) source->length);

    return (int32_t) inflatedLength;
}

static inline int32_t
gekkota_lzf_index(const GekkotaLZF *lzf, int32_t value)
{
    return ((value ^ (value << 5)) >> (((3 * 8 - lzf->compressionLevel)) - value * 5)
            & (((int32_t) lzf->hashTable.length / sizeof(int32_t)) - 1));
}

static inline int32_t
gekkota_lzf_first(byte_t *data, int32_t index)
{
    return (data[index] << 8) | data[index + 1];
}

static inline int32_t
gekkota_lzf_next(int32_t value, byte_t *data, int32_t index)
{
    return (value << 8) | data[index + 2];
}
