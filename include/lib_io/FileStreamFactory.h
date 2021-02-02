/**
 * @addtogroup lib_io
 * @{
 *
 * @file FileStreamFactory.h
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
#include "lib_utils/Bitmap.h"


/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum
{
    FileStream_DeleteFlags_CLOSE,
    FileStream_DeleteFlags_DELETE,

    FileStream_DeleteFlags_MAX
}
FileStream_DeleteFlags;

typedef struct FileStreamFactory FileStreamFactory;

typedef FileStream*
(*FileStreamFactory_CreateT)(FileStreamFactory* self,
                             const char* path,
                             FileStream_OpenMode mode);
typedef void
(*FileStreamFactory_DestroyT)(FileStreamFactory* self,
                              FileStream* fileStream,
                              Bitmap16 flags);

typedef void
(*FileStreamFactory__DtorT)(FileStreamFactory* self);

typedef struct
{
    FileStreamFactory_CreateT   create;
    FileStreamFactory_DestroyT  destroy;
    FileStreamFactory__DtorT    dtor;
}
FileStreamFactory_Vtable;

struct FileStreamFactory
{
    const FileStreamFactory_Vtable* vtable;
};


/* Exported constants --------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
/**
 * @brief creates and initializes a new instance of a FileStream
 * and returns it
 *
 * @param self pointer to self
 * @param path path of the filestream which is to be created
 * @param mode in which the filestream will be opened (read, write, read/write...)
 * @return pointer to the created filestream
 */
INLINE FileStream*
FileStreamFactory_create(FileStreamFactory* self,
                         const char* path,
                         FileStream_OpenMode mode)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->create(self, path, mode);
}
/**
 * @brief frees the memory allocated for the FileStream
 *
 * @param self pointer to self
 * @param fileStream pointer to the filestream which is to be destroyed
 *
 */
INLINE void
FileStreamFactory_destroy(FileStreamFactory* self, FileStream* fileStream,
                          Bitmap16 flags)
{
    Debug_ASSERT_SELF(self);
    self->vtable->destroy(self, fileStream, flags);
}
/**
 * @brief destructor
 *
 */
INLINE void
FileStreamFactory_dtor(FileStreamFactory* self)
{
    Debug_ASSERT_SELF(self);
    self->vtable->dtor(self);
}

#endif /* FILE_STREAM_FACTORY_H */

///@}
