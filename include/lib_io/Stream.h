/**
 * @addtogroup lib_io
 * @{
 *
 * @file Stream.h
 *
 * @brief interface that abstracts a Stream.
 *
 *
 * @author Carmelo Pintaudi
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
#if !defined(STREAM_H)
#define STREAM_H

/* Includes ------------------------------------------------------------------*/

#include "lib_compiler/compiler.h"

#include "lib_debug/Debug.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>


/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct Stream Stream;

typedef size_t
(*Stream_WriteT)(Stream* self, char const* buffer, size_t length);

typedef size_t
(*Stream_ReadT)(Stream* self, char* buffer, size_t length);

typedef size_t
(*Stream_GetT)(Stream* self,
               char* buff,
               size_t len,
               const char* delims,
               unsigned timeoutTicks);
typedef size_t
(*Stream_AvailableT)(Stream* self);

typedef void
(*Stream_FlushT)(Stream* self);

typedef void
(*Stream_CloseT)(Stream* self);

typedef void
(*Stream_DtorT)(Stream* self);

typedef struct
{
    Stream_WriteT       write;
    Stream_ReadT        read;
    Stream_GetT         get;
    Stream_FlushT       flush;
    Stream_FlushT       skip;
    Stream_AvailableT   available;
    Stream_CloseT       close;
    Stream_DtorT        dtor;
}
Stream_Vtable;

struct Stream
{
    const Stream_Vtable* vtable;
};


/* Exported constants --------------------------------------------------------*/
/* Exported dynamic functions ----------------------------------------------- */

/**
 * @brief writes from 'buffer' at the most 'length' bytes into the stream. The
 * stream could be buffered or not and therefore at this level no assumption can
 *  be done on when exactly the data taken in charge will be effectively
 *  delivered.
 *
 * @param self pointer to self
 * @param buffer input buffer where the data to be written are
 * @param length maximum amount of bytes that can be taken from the buffer
 *
 * @return number of bytes taken in charge, can be 0 to 'length'
 *
 */
INLINE size_t
Stream_write(Stream* self, char const* buffer, size_t length)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->write(self, buffer, length);
}
/**
 * @brief reads to 'buffer' at the most 'length' bytes from the stream. The
 * stream could be buffered or not and therefore at this level no assumption can
 *  be done on when exactly the data taken in charge will be effectively
 *  delivered.
 *
 * @param self pointer to self
 * @param buffer output buffer where the read bytes will be stored
 * @param length maximum amount of bytes that can be put into the buffer
 *
 * @return number of bytes read, can be 0 to 'length'
 *
 */
INLINE size_t
Stream_read(Stream* self, char* buffer, size_t length)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->read(self, buffer, length);
}
/**
 * @brief gets from the stream exactly 'len' bytes or less in case of timeout or
 *  delimiter is ecountered. The difference with read is that here we can
 *  assume that the function will have a bloking behavior until its exit
 *  conditions are reached (read amount equal 'len' or timeout or 'delim'
 *  encountered or a generic condition for which the stream cannot be read
 *  anymore, in this last case it returns EOF)
 *
 * @param self pointer to self
 * @param buffer output buffer where to store the read bytes
 * @param len amount of bytes to get
 * @param delims an array of delimiter characters, can be NULL if none
 * @param timeOutTicks timeout in system ticks, can be 0 if it can stay blocked
 *  for ever in the attempt to reach an exit condition
 *
 * @return number of bytes read. Can be EOF if the stream cannot read further
 *
 */
INLINE int
Stream_get(Stream* self,
           char* buff,
           size_t len,
           const char* delims,
           unsigned timeOutTicks)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->get(self, buff, len, delims, timeOutTicks);
}
/**
 * @brief returns the amount of available bytes in the stream for read
 *
 * @param self pointer to self
 * @return number of bytes available in the stream
 *
 */
INLINE size_t
Stream_available(Stream* self)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->available(self);
}
/**
 * @brief it grants that all the bytes written so far will be delivered. This
 * synchronize the write funtion.
 *
 * @param self pointer to self
 *
 */
INLINE void
Stream_flush(Stream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->flush(self);
}
/**
 * @brief simmetric of flush. It grants that all the bytes available for read
 *  so far will be skipped.
 *
 * @param self pointer to self
 *
 */
INLINE void
Stream_skip(Stream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->skip(self);
}
/**
 * @brief closes the stream and releases any resources
 *
 * @param self pointer to self
 *
 */
INLINE void
Stream_close(Stream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->close(self);
}
/**
 * @brief destructor
 *
 * @param self pointer to self
 *
 */
INLINE void
Stream_dtor(Stream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->dtor(self);
}

/* Exported static functions ------------------------------------------------ */

/**
 * @brief puts a char into the stream and grants the synchronization with the
 *  output channel. It is a blocking call
 *
 */

INLINE void
Stream_putChar(Stream* self, char byte)
{
    Debug_ASSERT_SELF(self);

    self->vtable->write(self, &byte, sizeof(byte));
    self->vtable->flush(self);
}
/**
 * @brief gets a char from the stream or returns EOF if the stream cannot be
 *  read anymore. It is a blocking call
 *
 */
INLINE int
Stream_getChar(Stream* self)
{
    Debug_ASSERT_SELF(self);

    char c = 0;
    size_t got = Stream_get(self, &c, sizeof(c), NULL, 0);

    return got == EOF ? EOF : (int) c;
}

INLINE void
Stream_writeNassert(Stream* self, const char* buff, size_t len)
{
    Debug_ASSERT_SELF(self);
    DECL_UNUSED_VAR(size_t written) = Stream_write(self, buff, len);
    Debug_ASSERT(written == len);
}

INLINE void
Stream_readNassert(Stream* self, char* buff, size_t len)
{
    Debug_ASSERT_SELF(self);
    DECL_UNUSED_VAR(size_t read) = Stream_read(self, buff, len);
    Debug_ASSERT(read == len);
}
/**
 * @brief it grants to write all 'len' bytes but does not grant the
 *  synchronization with the output channel. It is a blocking function but not a
 *  synchronous one.
 *
 */
INLINE void
Stream_writeAll(Stream* self, const char* buff, size_t len)
{
    Debug_ASSERT_SELF(self);
    Debug_ASSERT(buff != NULL);

    size_t todo = len;

    while (todo > 0)
    {
        todo -= Stream_write(self, &buff[len - todo], todo);

        if (todo > 0)
        {
            Stream_flush(self);
        }
    }
}
/**
 * @brief it writes at the most 'len' bytes and grants that the returned amount
 *  of written bytes is delivered by the output channel
 *
 */
INLINE size_t
Stream_writeSync(Stream* self, const char* buff, size_t len)
{
    size_t retval = Stream_write(self, buff, len);
    Stream_flush(self);

    return retval;
}
/**
 * @brief it grants to write all 'len' bytes and the synchronization with the
 *  output channel. It is a blocking synchronous function
 *
 */
INLINE void
Stream_writeAllSync(Stream* self, const char* buff, size_t len)
{
    Debug_ASSERT_SELF(self);
    Debug_ASSERT(buff != NULL);

    size_t todo = len;

    while (todo > 0)
    {
        todo -= Stream_writeSync(self, &buff[len - todo], todo);
    }
}

INLINE void
Stream_putString(Stream* self, char const* string)
{
    Stream_writeAllSync(self, string, strlen(string));
}

int
Stream_vaprintf(Stream* self, const char* fmt, va_list args);
/**
 * @brief writes into the stream a formatted string. Memory is allocated to
 * build the formatted string
 *
 */
int
Stream_printf(Stream* self, const char* fmt, ...);

#endif /* STREAM_H */
///@}
