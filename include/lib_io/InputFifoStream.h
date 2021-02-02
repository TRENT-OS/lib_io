/**
 * @addtogroup lib_io
 * @{
 *
 * @file InputFifoStream.h
 *
 * @brief a class that implements the Stream.h interface providing a buffered
 *  input with a FIFO.
 *
 *
 * @author Carmelo Pintaudi
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
#if !defined(INPUT_FIFO_STREAM_H)
#define INPUT_FIFO_STREAM_H


/* Includes ------------------------------------------------------------------*/

#include "lib_io/Stream.h"
#include "lib_utils/CharFifo.h"

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
/**
 * @brief constructor. The input fifo stream is a input stream that has a fifo
 *  buffer to contain the bytes in input.
 *
 * @param self pointer to self
 * @param readBuffer a chunk of memory to be used by the fifo
 * @param readBufSize size of the memory that the fifo can use
 *
 * @return true if success
 *
 */
bool
InputFifoStream_ctor(InputFifoStream* self, void* readBuf, size_t readBufSize);
/**
 * @brief static implementation of virtual method Stream_read(). For an input
 * fifo stream the read is always a non blocking function. The bytes in the
 *  fifo are taken
 *
 */
size_t
InputFifoStream_read(Stream* self, char* buffer, size_t length);
/**
 * @brief static implementation of virtual method Stream_get()
 *
 */
size_t
InputFifoStream_get(Stream* self,
                    char* buff,
                    size_t len,
                    const char* delims,
                    unsigned timeoutTicks);
/**
 * @brief static implementation of virtual method Stream_available()
 *
 */
size_t
InputFifoStream_available(Stream* self);
/**
 * @brief static implementation of virtual method Stream_skip()
 *
 */
void
InputFifoStream_skip(Stream* self);
/**
 * @brief static implementation of virtual method Stream_dtor()
 *
 */
void
InputFifoStream_dtor(Stream* self);

#endif /* FIFO_STREAM_H */
///@}
