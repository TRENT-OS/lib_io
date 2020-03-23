/* Includes ------------------------------------------------------------------*/

#include "lib_io/FifoStream.h"
#include "lib_osal/System.h"

#include "SeosLogger.h"
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

    bool ok = false;
    size_t capacity = 0, size = 0, free = 0, written = 0;
    char const* pC = NULL;

    if (length > 0)
    {
        capacity    = CharFifo_getCapacity(&self->writeFifo);
        size        = CharFifo_getSize(&self->writeFifo);

        free    = capacity - size;
        written = length < free ? length : free;

        for (size_t i = 0; i < written; i++)
        {
            pC = &((char const*) buffer)[i];
            ok = CharFifo_push(&self->writeFifo, pC);
            Debug_ASSERT(ok);
        }
    }
    else { /* do nothing */ }

    return written;
}

void
FifoStream_flush(Stream* stream)
{
    FifoStream* self = (FifoStream*) stream;
    Debug_ASSERT_SELF(self);

    while (CharFifo_getSize(&self->writeFifo) > 0)
    {
        System_delayTicks(1);
    }
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
