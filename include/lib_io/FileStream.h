/**
 * @addtogroup LibIO
 * @{
 *
 * @file FileStream.h
 *
 * @brief a file stream interface
 *
 * @author Carmelo Pintaudi
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#if !defined(FILE_STREAM_H)
#define FILE_STREAM_H


/* Includes ------------------------------------------------------------------*/

#include "lib_io/Stream.h"


/* Exported macro ------------------------------------------------------------*/

#define FileStream_TO_STREAM(self) ((Stream*) self)


/* Exported types ------------------------------------------------------------*/

typedef enum
{
    FileStream_OpenMode_Default,
    FileStream_OpenMode_r,   ///< Open a file for reading. The file must exist.
    FileStream_OpenMode_w,   ///< Create an empty file for writing. If a file with the same name already exists its content is erased and the file is treated as a new empty file.
    FileStream_OpenMode_a,   ///< Append to a file. Writing operations append data at the end of the file. The file is created if it does not exist.
    FileStream_OpenMode_R,   ///< Open a file for update both reading and writing. The file must exist.
    FileStream_OpenMode_W,   ///< Create an empty file for both reading and writing. If a file with the same name already exists its content is erased and the file is treated as a new empty file.
    FileStream_OpenMode_A    ///< Open a file for reading and appending. All writing operations are performed at the end of the file, protecting the previous content to be overwritten. You can reposition (fseek, rewind) the internal pointer to anywhere in the file for reading, but writing operations will move it back to the end of file. The file is created if it does not exist.
}
FileStream_OpenMode;

typedef struct FileStream FileStream;

typedef enum
{
    FileStream_SeekMode_Begin,
    FileStream_SeekMode_End,
    FileStream_SeekMode_Curr
}
FileStream_SeekMode;

typedef long int
(*FileStream_SeekT)(FileStream* self,
                    long long int offset,
                    FileStream_SeekMode mode);
typedef FileStream*
(*FileStream_ReOpenT)(FileStream* self, FileStream_OpenMode mode);

typedef int
(*FileStream_ErrorT)(FileStream* self);

typedef void
(*FileStream_ClearErrorT)(FileStream* self);

typedef struct
{
    Stream_Vtable parent;

    FileStream_SeekT        seek;
    FileStream_ReOpenT      reopen;
    FileStream_ErrorT       error;
    FileStream_ClearErrorT  clearError;
}
FileStream_Vtable;

struct FileStream
{
    const FileStream_Vtable* vtable;
};


/* Exported constants --------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

INLINE long int
FileStream_seek(FileStream* self,
                long long int offset,
                FileStream_SeekMode mode)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->seek(self, offset, mode);
}

INLINE FileStream*
FileStream_reOpen(FileStream* self, FileStream_OpenMode mode)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->reopen(self, mode);
}

INLINE int
FileStream_error(FileStream* self)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->error(self);
}

INLINE void
FileStream_clearError(FileStream* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->clearError(self);
}

#endif /* FILE_STREAM_H */

