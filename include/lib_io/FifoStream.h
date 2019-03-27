#if !defined(FIFO_STREAM_H)
#define FIFO_STREAM_H


/* Includes ------------------------------------------------------------------*/

#include "lib_io/InputFifoStream.h"


/* Exported macro ------------------------------------------------------------*/

#define FifoStream_TO_INPUT_FIFO_STREAM(self) (&(self)->parent)


/* Exported types ------------------------------------------------------------*/

typedef struct FifoStream FifoStream;

struct FifoStream
{
    InputFifoStream         parent;
    CharFifo                writeFifo;
};


/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

bool
FifoStream_ctor(FifoStream* self,
                void* writeBuf, size_t writeBufSize,
                void* readBuf, size_t readBufSize);

size_t
FifoStream_write(Stream* self, char const* buffer, size_t length);

void
FifoStream_flush(Stream* self);

void
FifoStream_dtor(Stream* self);


#endif /* FIFO_STREAM_H */
