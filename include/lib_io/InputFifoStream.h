#if !defined(INPUT_FIFO_STREAM_H)
#define INPUT_FIFO_STREAM_H


/* Includes ------------------------------------------------------------------*/

#include "lib_io/Stream.h"
#include "lib_util/CharFifo.h"

#include <stddef.h>


/* Exported macro ------------------------------------------------------------*/

#define InputFifoStream_TO_STREAM(self)  (&(self)->parent)


/* Exported types ------------------------------------------------------------*/

typedef struct InputFifoStream InputFifoStream;

struct InputFifoStream
{
    Stream                  parent;
    CharFifo                readFifo;
};

/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

bool
InputFifoStream_ctor(InputFifoStream* self, void* readBuf, size_t readBufSize);

size_t
InputFifoStream_read(Stream* self, char* buffer, size_t length);

size_t
InputFifoStream_get(Stream* self,
                    char* buff,
                    size_t len,
                    const char* delims,
                    unsigned timeoutTicks);
size_t
InputFifoStream_available(Stream* self);

void
InputFifoStream_skip(Stream* self);

void
InputFifoStream_dtor(Stream* self);


#endif /* FIFO_STREAM_H */
