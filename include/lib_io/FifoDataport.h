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


//------------------------------------------------------------------------------
static inline bool
FifoDataport_ctor(
    FifoDataport* self,
    size_t capacity)
{
    return CharFifo_ctor(&self->dataStruct,
                         self->data,
                         capacity);
}


//------------------------------------------------------------------------------
static inline size_t
FifoDataport_getSize(
    FifoDataport* self)
{
    return CharFifo_getSize(&self->dataStruct);
}


//------------------------------------------------------------------------------
static inline size_t
FifoDataport_getCapacity(
    FifoDataport* self)
{
    return CharFifo_getCapacity(&self->dataStruct);
}


//------------------------------------------------------------------------------
static inline size_t
FifoDataport_getFree(
    FifoDataport* self)
{
    size_t capacity = FifoDataport_getCapacity(self);
    size_t used = FifoDataport_getSize(self);

    assert(used <= capacity);
    return capacity - used;
}


//------------------------------------------------------------------------------
static inline bool
FifoDataport_isEmpty(
    FifoDataport* self)
{
    return CharFifo_isEmpty(&self->dataStruct);
}


//------------------------------------------------------------------------------
static inline bool
FifoDataport_isFull(
    FifoDataport* self)
{
    return CharFifo_isFull(&self->dataStruct);
}


//------------------------------------------------------------------------------
static inline void const*
FifoDataport_getFirst(
    FifoDataport* self)
{
    if (FifoDataport_isEmpty(self))
    {
        return NULL;
    }
    return &self->data[self->dataStruct.first];
}


//------------------------------------------------------------------------------
static inline size_t
FifoDataport_getAmountConsecutives(
    FifoDataport* self)
{
    size_t capacity = FifoDataport_getCapacity(self);
    size_t size     = FifoDataport_getSize(self);
    size_t first    = self->dataStruct.first;
    size_t last     = self->dataStruct.last;

    return (first > last) ? capacity - first : size;
}


//------------------------------------------------------------------------------
static inline void
FifoDataport_remove(
    FifoDataport* self,
    size_t amount)
{
    size_t used = FifoDataport_getSize(self);
    if (amount > used)
    {
        Debug_LOG_ERROR("FifoDataport_remove() amount %zu > used %zu", amount, used);
        assert(0);
    }

    // We don't loop over calling the FifoT's function CharFifo_pop(), because
    // we know that char_dtor() does nothing for a char FIFO. So we can just
    // modify the fields directly.
    // The used bytes in the FIFO (aka "size") are calculated based on the
    // fields "in" and "out". The fields "first" and "last" are used only for
    // addressing data. Since we know they are always less than the FIFO
    // capacity, we can avoid a potentially expensive modulo operation.
    size_t capacity = FifoDataport_getCapacity(self);
    size_t updated_first = self->dataStruct.first + amount;
    if (updated_first >= capacity)
    {
        updated_first -= capacity;
        assert(updated_first < capacity);
    }
    self->dataStruct.first = updated_first;
    self->dataStruct.out += amount;
}


//------------------------------------------------------------------------------
static inline void
FifoDataport_add(
    FifoDataport* self,
    size_t amount)
{
    size_t free = FifoDataport_getFree(self);
    if (amount > free)
    {
        Debug_LOG_ERROR("FifoDataport_add() amount %zu > free %zu", amount, free);
        assert(0);
    }

    // The used bytes in the FIFO (aka "size") are calculated based on the
    // fields "in" and "out". The fields "first" and "last" are used only for
    // addressing data. Since we know they are always less than the FIFO
    // capacity, we can avoid a potentially expensive modulo operation. There
    // is no need to call CharFifo_push() here, because chars aren't objects
    // that would require special handing.
    size_t capacity = FifoDataport_getCapacity(self);
    size_t updated_last = self->dataStruct.last + amount;
    if (updated_last >= capacity)
    {
        updated_last -= capacity;
        assert(updated_last < capacity);
    }
    self->dataStruct.last = updated_last;
    self->dataStruct.in += amount;

}


//------------------------------------------------------------------------------
static inline size_t
FifoDataport_read(
    FifoDataport* self,
    void* buf,
    size_t len)
{
    size_t  read   = 0;
    char*   target = buf;

    if (target != NULL)
    {
        while (read < len)
        {
            const char* source = FifoDataport_getFirst(self);
            if (NULL == source)
            {
                break;
            }
            target[read++] = *source;
            CharFifo_pop(&self->dataStruct);
        }
    }
    return read;
}


//------------------------------------------------------------------------------
static inline size_t
FifoDataport_write(
    FifoDataport* self,
    void const* buf,
    size_t len)
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


//------------------------------------------------------------------------------
static inline void
FifoDataport_dtor(
    FifoDataport* self)
{
    CharFifo_dtor(&self->dataStruct);
}
