/*  Copyright (C) 2020, HENSOLDT Cyber GmbH */
/* Includes ------------------------------------------------------------------*/

#include "lib_io/InputFifoStream.h"

#include "lib_debug/Debug.h"
#include <stdbool.h>


/* Defines -------------------------------------------------------------------*/

/* Private functions prototypes ----------------------------------------------*/

static size_t
write(Stream* stream, char const* buffer, size_t length)
{
    // an input fifo stream does not know how to write
    return 0;
}

static void
flush(Stream* stream)
{
    // an input fifo stream does not know how to flush
    return;
}


/* Private variables ---------------------------------------------------------*/

static const Stream_Vtable InputFifoStream_vtable =
{
    .read       = InputFifoStream_read,
    .get        = InputFifoStream_get,
    .write      = write,
    .available  = InputFifoStream_available,
    .flush      = flush,
    .skip       = InputFifoStream_skip,
    .close      = flush,
    .dtor       = InputFifoStream_dtor
};


/* Public functions ----------------------------------------------------------*/

bool
InputFifoStream_ctor(InputFifoStream* self,
                     void*  readBuf,
                     size_t readBufSize)
{
    Debug_ASSERT_SELF(self);

    bool retval = true;

    if (!CharFifo_ctor(&self->readFifo, readBuf, readBufSize))
    {
        goto error1;
    }
    self->parent.vtable = &InputFifoStream_vtable;
    goto exit;

error1:
    retval = false;
exit:
    return retval;
}

size_t
InputFifoStream_read(Stream* stream, char* buffer, size_t length)
{
    InputFifoStream* self = (InputFifoStream*) stream;
    Debug_ASSERT_SELF(self);
    Debug_ASSERT(buffer != NULL);

    CharFifo*   readFifo    = &self->readFifo;
    size_t      readBytes   = 0;
    size_t      fifoSize    = CharFifo_getSize(readFifo);

    while (readBytes < length && readBytes < fifoSize)
    {
        buffer[readBytes++] = * CharFifo_getFirst(readFifo);
        CharFifo_pop(readFifo);
    }
    return readBytes;
}

size_t
InputFifoStream_get(Stream* stream,
                    char* buff,
                    size_t len,
                    const char* delims,
                    unsigned timeoutTicks)
{
    InputFifoStream* self = (InputFifoStream*) stream;
    Debug_ASSERT_SELF(self);
    Debug_ASSERT(buff != NULL);

    CharFifo* readFifo = &self->readFifo;
    Debug_ASSERT(readFifo != NULL);

    if (0 != timeoutTicks)
    {
        Debug_LOG_ERROR("timeouts are not supported");
        return 0;
    }

    size_t i = 0;
    while (i < len)
    {
        if (CharFifo_isEmpty(readFifo))
        {
            break;
        }

        char c = CharFifo_getAndPop(readFifo);
        if ((delims != NULL) && (strchr(delims, c) != NULL))
        {
            break;
        }

        buff[i++] = c;
    }

    return i;
}

size_t
InputFifoStream_available(Stream* stream)
{
    InputFifoStream* self = (InputFifoStream*) stream;
    Debug_ASSERT_SELF(self);
    return CharFifo_getSize(&self->readFifo);
}

void
InputFifoStream_skip(Stream* stream)
{
    InputFifoStream* self = (InputFifoStream*) stream;
    Debug_ASSERT_SELF(self);

    CharFifo_clear(&self->readFifo);
}

void
InputFifoStream_dtor(Stream* stream)
{
    DECL_UNUSED_VAR(InputFifoStream * self) = (InputFifoStream*) stream;
    Debug_ASSERT_SELF(self);

    CharFifo_dtor(&self->readFifo);
}


/* Private functions ---------------------------------------------------------*/


///@}
