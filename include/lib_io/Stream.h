#if !defined(STREAM_H)
#define STREAM_H

/* Includes ------------------------------------------------------------------*/

#include "lib_compiler/compiler.h"

#include "lib_debug/Debug.h"
#include "lib_osal/System.h"

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
/* Exported functions ------------------------------------------------------- */

INLINE size_t
Stream_write(Stream* self, char const* buffer, size_t length)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->write(self, buffer, length);
}

INLINE size_t
Stream_read(Stream* self, char* buffer, size_t length)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->read(self, buffer, length);
}
/// can ret EOF
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

INLINE void
Stream_writeNassert(Stream* self, const char* buff, size_t len)
{
    Debug_ASSERT_SELF(self);
    size_t written = Stream_write(self, buff, len);
    Debug_ASSERT(written == len);
}

INLINE void
Stream_readNassert(Stream* self, char* buff, size_t len)
{
    Debug_ASSERT_SELF(self);
    size_t read = Stream_read(self, buff, len);
    Debug_ASSERT(read == len);
}

INLINE size_t
Stream_available(Stream* self)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->available(self);
}

INLINE void
Stream_flush(Stream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->flush(self);
}

INLINE void
Stream_skip(Stream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->skip(self);
}

INLINE void
Stream_putChar(Stream* self, char byte)
{
    Debug_ASSERT_SELF(self);

    self->vtable->write(self, &byte, sizeof(byte));
    self->vtable->flush(self);
}

INLINE void
Stream_close(Stream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->close(self);
}

INLINE void
Stream_dtor(Stream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->dtor(self);
}

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

INLINE size_t
Stream_writeSync(Stream* self, const char* buff, size_t len)
{
    size_t retval = Stream_write(self, buff, len);
    Stream_flush(self);

    return retval;
}

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

INLINE void
Stream_readAll(Stream* self, char* buff, size_t len)
{
    Debug_ASSERT_SELF(self);
    Debug_ASSERT(buff != NULL);

    size_t todo = len;

    while (todo > 0)
    {
        todo -= Stream_read(self, &buff[len - todo], todo);

        if (todo > 0)
        {
            System_delayTicks(1);
        }
    }
}

/// can ret EOF
INLINE int
Stream_getChar(Stream* self)
{
    Debug_ASSERT_SELF(self);

    char c = 0;
    size_t got = Stream_get(self, &c, sizeof(c), NULL, 0);

    return got == EOF ? EOF : (int) c;
}

int
Stream_vaprintf(Stream* self, const char* fmt, va_list args);

int
Stream_printf(Stream* self, const char* fmt, ...);

#endif /* STREAM_H */
