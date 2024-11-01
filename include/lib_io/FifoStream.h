/*
 * Copyright (C) 2019-2024, HENSOLDT Cyber GmbH
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * For commercial licensing, contact: info.cyber@hensoldt.net
 */

/**
 * @addtogroup lib_io
 * @{
 *
 * @file FifoStream.h
 *
 * @brief a class that implements the Stream.h interface providing a buffered IO
 *  with FIFOs.
 *
 * @author Carmelo Pintaudi
 */
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
/**
 * @brief constructor. The fifo stream is a input/output stream that has 2 fifo
 *  buffers to contain the bytes in input and output.
 *
 * @param self pointer to self
 * @param readBuffer a chunk of memory to be used by the input fifo
 * @param readBufSize size of the memory that the input fifo can use
 * @param writeBuffer a chunk of memory to be used by the output fifo
 * @param writeBufSize size of the memory that the output fifo can use
 *
 * @return true if success
 *
 */
bool
FifoStream_ctor(FifoStream* self,
                void* writeBuf, size_t writeBufSize,
                void* readBuf, size_t readBufSize);
/**
 * @brief static implementation of virtual method Stream_write(). For a fifo
 *  stream the write is always a non blocking function. The bytes are just
 *  stored in the fifo and the actual transfer is deferred. Synchronization
 *  functions of Stream.h can be used
 *
 */
size_t
FifoStream_write(Stream* self, char const* buffer, size_t length);
/**
 * @brief static implementation of virtual method Stream_flush()
 *
 */
void
FifoStream_flush(Stream* self);
/**
 * @brief static implementation of virtual method Stream_dtor()
 *
 */
void
FifoStream_dtor(Stream* self);

#endif /* FIFO_STREAM_H */
///@}
