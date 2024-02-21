/*
 * Copyright (C) 2020-2024, HENSOLDT Cyber GmbH
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * For commercial licensing, contact: info.cyber@hensoldt.net
 */

/* Includes ------------------------------------------------------------------*/

#include "lib_io/FifoStream.h"

#include "lib_debug/Debug.h"
#include <stdbool.h>


/* Defines -------------------------------------------------------------------*/

/* Private functions prototypes ----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static const Stream_Vtable FifoStream_vtable =
{
    .read       = InputFifoStream_read,
    .get        = InputFifoStream_get,
    .write      = FifoStream_write,
    .available  = InputFifoStream_available,
    .flush      = FifoStream_flush,
    .skip       = InputFifoStream_skip,
    .close      = FifoStream_flush,
    .dtor       = FifoStream_dtor
};


/* Public functions ----------------------------------------------------------*/

bool
FifoStream_ctor(FifoStream* self,
                void* writeBuf, size_t writeBufSize,
                void* readBuf, size_t readBufSize)
{
    Debug_ASSERT_SELF(self);

    bool retval = true;
    Stream* stream =
        InputFifoStream_TO_STREAM(
            FifoStream_TO_INPUT_FIFO_STREAM(self));

    if (!CharFifo_ctor(&self->writeFifo, writeBuf, writeBufSize))
    {
        goto error1;
    }
    if (!InputFifoStream_ctor(&self->parent, readBuf, readBufSize))
    {
        goto error2;
    }
    stream->vtable = &FifoStream_vtable;
    goto exit;

error2:
    InputFifoStream_dtor(stream);
error1:
    retval = false;
exit:
    return retval;
}

size_t
FifoStream_write(Stream* stream, char const* buffer, size_t length)
{
    FifoStream* self = (FifoStream*) stream;
    Debug_ASSERT_SELF(self);
    Debug_ASSERT(buffer != NULL);

    size_t capacity = 0, size = 0, free = 0, written = 0;

    if (length > 0)
    {
        capacity    = CharFifo_getCapacity(&self->writeFifo);
        size        = CharFifo_getSize(&self->writeFifo);

        free    = capacity - size;
        written = length < free ? length : free;

        for (size_t i = 0; i < written; i++)
        {
            char const* pC = &((char const*) buffer)[i];
            DECL_UNUSED_VAR(const bool isPushed)
                = CharFifo_push(&self->writeFifo, pC);
            Debug_ASSERT(isPushed);
        }
    }
    else { /* do nothing */ }

    return written;
}

void
FifoStream_flush(Stream* stream)
{
    Debug_ASSERT_SELF((FifoStream*) stream);

    /* There is no generic way to flush a FifoStream. What the caller could do
     * is polling stream->available() until this is zero and use some form of
     * sleeping between the calls to avoid burning CPU time. If the caller has
     * more details about how the FIFO is used, it could also wait on some
     * dedicated event that is triggered by the other entity using the FIFO
     * once it has read all pending data.
     *
     * Unfortunately, we have to implement this function can can't just set
     * FifoStream_vtable.flush to NULL. In Stream.c, there is Stream_flush()
     * that will call our flush() without checking if a function is really set
     * there. Unfortuantely, flush() does not have a return code either, that
     * could be used to inform the caller that the flush failed.
     * For debug builds we trigger an assert here. For release builds we can't
     * do anything besides logging the message.
     */
    Debug_LOG_FATAL("flushing a FifoStream is not supported");
    Debug_ASSERT(false);
}

void
FifoStream_dtor(Stream* stream)
{
    FifoStream* self = (FifoStream*) stream;
    Debug_ASSERT_SELF(self);

    CharFifo_dtor(&self->writeFifo);
    InputFifoStream_dtor(stream);
}


/* Private functions ---------------------------------------------------------*/


///@}
