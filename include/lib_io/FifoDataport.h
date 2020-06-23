/**
 *
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 *
 * @brief Fifo policy class for bytes in a dataport.
 * The implementation behind this support class does implement Single Producer
 * Single Consumer thread safety without mutex. In this way the FifoDataport
 * can be shared between two components. The owner component will create the
 * object to produce while the other side can safely use it and consume. This
 * wrap around class is needed especially to take care of the fact that will be
 * used by two different address spaces (see FifoDataport_getFirstByte()).
 * The dataport buffer containing a FifoDataport is therefore organised in the
 * following way:
 *  __________________________________________________________________________
 * | ------------|------------------------------------------------------------|
 * || dataStruct |                       data                                ||
 * | ------------|------------------------------------------------------------|
 * |__________________________________________________________________________|
 *
 * @note The FifoDataport is supposed to be created by the Producer.
 *
 */
#pragma once

#include "lib_util/CharFifo.h"

typedef struct
{
    CharFifo dataStruct;
    char     data[];
}
FifoDataport;

static inline bool
FifoDataport_ctor(FifoDataport* self, size_t capacity)
{
    return CharFifo_ctor(&self->dataStruct,
                         self->data,
                         capacity);
}

static inline size_t
FifoDataport_getSize(FifoDataport* self)
{
    return CharFifo_getSize(&self->dataStruct);
}

static inline size_t
FifoDataport_getCapacity(FifoDataport* self)
{
    return CharFifo_getCapacity(&self->dataStruct);
}

static inline void const*
FifoDataport_getFirst(FifoDataport* self)
{
    if (CharFifo_isEmpty(&self->dataStruct))
    {
        return NULL;
    }
    return &self->data[self->dataStruct.first];
}

static inline size_t
FifoDataport_getAmountConsecutives(FifoDataport* self)
{
    size_t capacity = FifoDataport_getCapacity(self);
    size_t size     = FifoDataport_getSize(self);
    size_t first    = self->dataStruct.first;
    size_t last     = self->dataStruct.last;

    return (first > last) ? capacity - first : size;
}

static inline void
FifoDataport_remove(FifoDataport* self, size_t amount)
{
    while (amount--)
    {
        CharFifo_pop(&self->dataStruct);
    }
}

static inline size_t
FifoDataport_read(FifoDataport* self, void* buf, size_t len)
{
    size_t      read   = 0;
    char*       target = buf;
    char const* source = NULL;

    if (target != NULL)
    {
        while (read < len)
        {
            CharFifo* cFifo = &self->dataStruct;
            source = CharFifo_getFirst(cFifo);
            if (NULL == source)
            {
                break;
            }
            target  = &target[read++];
            *target = *source;
            CharFifo_pop(cFifo);
        }
    }
    return read;
}

static inline size_t
FifoDataport_write(FifoDataport* self, void const* buf, size_t len)
{
    size_t written      = 0;
    char const* target  = buf;

    if (target != NULL)
    {
        while (written < len
               && CharFifo_push(&self->dataStruct, &target[written]))
        {
            written++;
        }
    }
    return written;
}

static inline void
FifoDataport_dtor(FifoDataport* self)
{
    CharFifo_dtor(&self->dataStruct);
}
