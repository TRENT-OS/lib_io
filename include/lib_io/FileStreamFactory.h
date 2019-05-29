/**
 * @addtogroup LibIO
 * @{
 *
 * @file FileStream.h
 *
 * @brief a file stream abstract factory interface
 *
 * @author Carmelo Pintaudi
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#if !defined(FILE_STREAM_FACTORY_H)
#define FILE_STREAM_FACTORY_H

/* Includes ------------------------------------------------------------------*/

#include "lib_io/FileStream.h"


/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct FileStreamFactory FileStreamFactory;

typedef FileStream*
(*FileStreamFactory_CreateT)(FileStreamFactory* self,
                             const char* path,
                             FileStream_OpenMode mode);
typedef void
(*FileStreamFactory_DestroyT)(FileStreamFactory* self,
                              FileStream* fileStream);

typedef struct
{
    FileStreamFactory_CreateT   create;
    FileStreamFactory_DestroyT  destroy;
}
FileStreamFactory_Vtable;

struct FileStreamFactory
{
    const FileStreamFactory_Vtable* vtable;
};


/* Exported constants --------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

INLINE FileStream*
FileStreamFactory_create(FileStreamFactory* self,
                         const char* path,
                         FileStream_OpenMode mode)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->create(self, path, mode);
}

INLINE void
FileStreamFactory_destroy(FileStreamFactory* self,
                          FileStream* fileStream)
{
    Debug_ASSERT_SELF(self);
    self->vtable->destroy(self, fileStream);
}

#endif /* FILE_STREAM_H */

