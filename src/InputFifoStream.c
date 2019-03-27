/* Includes ------------------------------------------------------------------*/

#include "lib_io/InputFifoStream.h"
#include "lib_osal/System.h"

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

    CharFifo*   readBuf = &self->readFifo;
    char const* read = NULL;
    size_t      readBytes = 0;

    for (size_t i = 0;
         i < length && (read = CharFifo_getFirst(readBuf)) != NULL;
         i++, readBytes++)
    {
        buffer[i] = *read;
        CharFifo_pop(readBuf);
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
    DECL_UNUSED_VAR(InputFifoStream * self) = (InputFifoStream*) stream;
    Debug_ASSERT_SELF(self);

    size_t i        = 0;
    bool foundDelim = false;
    unsigned long long timeBase = System_getTickCount();

    while (i < len &&
           !foundDelim &&
           (timeoutTicks == 0 ||
            System_getTickCount() - timeBase < timeoutTicks))
    {
        if (InputFifoStream_read(stream, &buff[i], sizeof(char)) > 0)
        {
            if (delims != NULL)
            {
                if (strchr(delims, buff[i]) != NULL)
                {
                    foundDelim = true;
                    buff[i] = 0;
                }
                else
                {
                    i++;
                }
            }
            else
            {
                i++;
            }
        }
        else
        {
            System_delayTicks(1);
        }
    }
    return (foundDelim || i == len) ? i : EOF;
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
