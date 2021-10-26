/**
 *
 * Copyright (C) 2020, HENSOLDT Cyber GmbH
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

#include "lib_utils/CharFifo.h"

// #define FIFO_DATAPORT_PROFILING

typedef struct
{
    CharFifo dataStruct;
    char     data[];
}
FifoDataport;


//------------------------------------------------------------------------------
/**
 * @brief FifoDataport constructor
 *
 * @param self (required) pointer to the FifoDataport context
 * @param capacity (required) capacity in bytes of the FIFO in the dataport
 *
 * @retval true if succeeded
 */
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
/**
 * @brief returns the amount of bytes that are currently in the FIFO in the
 * dataport
 *
 * @param self (required) pointer to the FifoDataport context
 *
 * @return amount of bytes available in the FIFO in the dataport
 */
static inline size_t
FifoDataport_getSize(
    FifoDataport* self)
{
    return CharFifo_getSize(&self->dataStruct);
}


//------------------------------------------------------------------------------
/**
 * @brief returns the maximum amount of bytes that could be pushed into the FIFO
 * in the dataport
 *
 * @param self (required) pointer to the FifoDataport context
 *
 * @return maximum amount of bytes that could be put into the FIFO in the
 * dataport
 */
static inline size_t
FifoDataport_getCapacity(
    FifoDataport* self)
{
    return CharFifo_getCapacity(&self->dataStruct);
}


//------------------------------------------------------------------------------
/**
 * @brief returns the amount of bytes that could be still pushed into the FIFO
 * in the dataport
 *
 * @param self (required) pointer to the FifoDataport context
 *
 * @return amount of bytes that could be still pushed into the FIFO in the
 * dataport
 */
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
/**
 * @brief checks whether the FIFO in the dataport is empty
 *
 * @param self (required) pointer to the FifoDataport context
 *
 * @retval true if empty
 */
static inline bool
FifoDataport_isEmpty(
    FifoDataport* self)
{
    return CharFifo_isEmpty(&self->dataStruct);
}


//------------------------------------------------------------------------------
/**
 * @brief checks whether the FIFO in the dataport is full
 *
 * @param self (required) pointer to the FifoDataport context
 *
 * @retval true if full
 */
static inline bool
FifoDataport_isFull(
    FifoDataport* self)
{
    return CharFifo_isFull(&self->dataStruct);
}


//------------------------------------------------------------------------------
/**
 * @brief provides a pointer to the FIFO buffer in the dataport to the location
 * of the first available byte according to the FIFO policy and returns the
 * amount of available bytes from there until the buffer wrap around
 *
 * @note this is useful for 0-copy operations as data could be extracted using,
 * for example, memcpy() or DMA
 *
 * @note the amount of contiguous bytes is not in necessarily the same as
 * returned by FifoDataport_getSize(), it can be less
 *
 * @param self (required) pointer to the FifoDataport context
 * @param buffer (optional) pointer to a pointer that will be set to the
 * location of the first available byte according to the FIFO policy, it could
 * be set to NULL by the caller if not interested in getting this information
 *
 * @return amount of available bytes from the location of the first available
 * byte until the buffer wrap around
 */
static inline size_t
FifoDataport_getContiguous(
    FifoDataport* self,
    void** buffer)
{
    // +-----------+----------+-----------+
    // |<--free2-->|<--used-->|<--free1-->|
    // +-----------+----------+-----------+
    //           first       last
    //
    // +-----------+-----------+
    // |<--used2-->|<--used1-->|  isFull
    // |<--free2-->|<--free1-->|  isEmpty
    // +-----------+-----------+
    //        first/last
    //
    // +-----------+----------+-----------+
    // |<--used2-->|<--free-->|<--used1-->|
    // +-----------+----------+-----------+
    // |          last      first         |
    //
    // The caller wants to get ("lock") a buffer with data in the FIFO, so it
    // can access it without immediately removing it from the FIFO. This can be
    // useful for zero-copy operations. In parallel another thread may add data
    // to the FIFO. This is not a problem as long as we keep working on the
    // snapshot taken from the FIFO.
    size_t in = self->dataStruct.in;
    size_t out = self->dataStruct.out;

    // FIFO empty?
    if (in == out)
    {
        if (buffer)
        {
            *buffer = NULL;
        }
        return 0;
    }

    // Reading "first" is safe, because a thread putting data into the FIFO in
    // parallel will not modify it.
    size_t first = self->dataStruct.first;

    if (buffer)
    {
        *buffer = &self->data[first];
    }

    // self->dataStruct.last may have changed already, so we can't use it here
    size_t capacity = FifoDataport_getCapacity(self);
    size_t last = in % capacity;

#ifdef FIFO_DATAPORT_PROFILING

    size_t mem_last = self->dataStruct.last;
    if (last != mem_last)
    {
        Debug_LOG_INFO(
            "change due to concurrency: index 'last': %zu -> %zu, data 'in' %zu -> %zu",
            last, mem_last, in, self->dataStruct.in);
    }

#endif

    return (first < last) ? last - first : capacity - first;
}


//------------------------------------------------------------------------------
// This is deprecated, use FifoDataport_getContiguous() directly to get the
// buffer and the size atomically and avoid race conditions.
static inline void const*
FifoDataport_getFirst(
    FifoDataport* self)
{
    void* buffer = NULL;
    FifoDataport_getContiguous(self, &buffer);
    return buffer; // NULL if FIFO is empty
}


//------------------------------------------------------------------------------
// This is deprecated, use FifoDataport_getContiguous() directly to get the
// buffer and the size atomically and avoid race conditions.
static inline size_t
FifoDataport_getAmountConsecutives(
    FifoDataport* self)
{
    return FifoDataport_getContiguous(self, NULL);
}


//------------------------------------------------------------------------------
/**
 * @brief provides a pointer to the FIFO buffer in the dataport to the first
 * available location for new bytes according to the FIFO policy and returns the
 * amount of available byte locations from there until the buffer wrap around
 *
 * @note this is useful for 0-copy operations as that memory space could be
 * filled using, for example, memcpy() or DMA
 *
 * @note the amount of contiguous available locations is not necessarily the
 * same as returned by FifoDataport_getCapacity(), it can be less
 *
 * @param self (required) pointer to the FifoDataport context
 * @param buffer (optional) pointer to a pointer that will be set to the first
 * available location for new bytes according to the FIFO policy, it could be
 * set to NULL by the caller if not interested in getting this information
 *
 * @return amount of available byte locations from the first available location
 * for new bytes until the buffer wrap around
 */
static inline size_t
FifoDataport_getContiguousFree(
    FifoDataport* self,
    void** buffer)
{
    // +-----------+----------+-----------+
    // |<--free2-->|<--used-->|<--free1-->|
    // +-----------+----------+-----------+
    //           first       last
    //
    // +-----------+-----------+
    // |<--used2-->|<--used1-->|  isFull
    // |<--free2-->|<--free1-->|  isEmpty
    // +-----------+-----------+
    //        first/last
    //
    // +-----------+----------+-----------+
    // |<--used2-->|<--free-->|<--used1-->|
    // +-----------+----------+-----------+
    // |          last      first         |
    //
    // The caller wants to get ("lock") an empty part of the FIFO, so it can
    // put data there as a block, e.g. for DMA or avoiding intermediate buffers
    // and memcpy(). In parallel another thread may remove data to the FIFO.
    // This is not a problem as long as we keep working on the snapshot taken
    // from the FIFO.
    size_t capacity = FifoDataport_getCapacity(self);
    size_t in = self->dataStruct.in;
    size_t out = self->dataStruct.out;

    // FIFO full ?
    if ((out + capacity) == in)
    {
        if (buffer)
        {
            *buffer = NULL;
        }
        return 0;
    }

    // Reading "last" is safe, because a thread removing data from the FIFO in
    // parallel will not modify it.
    size_t last = self->dataStruct.last;

    if (buffer)
    {
        *buffer = &self->data[last];
    }

    // self->dataStruct.first may have changed already, so we can't use it here
    size_t first = out % capacity;

#ifdef FIFO_DATAPORT_PROFILING

    size_t mem_first = self->dataStruct.first;
    if (first != mem_first)
    {
        Debug_LOG_INFO(
            "change due to concurrency: index 'first': %zu -> %zu, data 'out' %zu -> %zu",
            first, mem_first, out, self->dataStruct.out);
    }

#endif

    return (first > last) ? first - last : capacity - last;
}


//------------------------------------------------------------------------------
/**
 * @brief pops out a certain amount of bytes from the FIFO in the dataport
 *
 * This is to be called after some or all data from the buffer returned by
 * FifoDataport_getContiguous() has been processed
 *
 * @param self (required) pointer to the FifoDataport context
 * @param amount (required) amount to be removed
 *
 */
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
/**
 * @brief counts as pushed a certain amount of bytes in the FIFO in the dataport
 *
 * This is to be called after some data is pushed into the buffer returned by
 * FifoDataport_getContiguousFree()
 *
 *
 * @param self (required) pointer to the FifoDataport context
 * @param amount (required) amount pushed
 *
 */
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
/**
 * @brief moves (copy and pop out) a certain amount of bytes from the FIFO in
 * the dataport to a given buffer
 *
 * @param self (required) pointer to the FifoDataport context
 * @param buf (required) pointer to the destination buffer
 * @param len (required) maximum amount of bytes that buf could take
 *
 * @return the amount of bytes which have been actually moved (shall be always
 * less or equal than len)
 */
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
/**
 * @brief copies a certain amount of bytes from a given buffer into the FIFO in
 * the dataport
 *
 * @param self (required) pointer to the FifoDataport context
 * @param buf (required) pointer to the source buffer
 * @param len (required) maximum amount of bytes that could be taken from buf
 *
 * @return the amount of bytes which have been actually copied (shall be always
 * less or equal than len)
 */
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
/**
 * @brief FifoDataport destructor
 *
 * @param self (required) pointer to the FifoDataport context
 */
static inline void
FifoDataport_dtor(
    FifoDataport* self)
{
    CharFifo_dtor(&self->dataStruct);
}
