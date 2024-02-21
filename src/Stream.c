/*
 * Copyright (C) 2020-2024, HENSOLDT Cyber GmbH
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * For commercial licensing, contact: info.cyber@hensoldt.net
 */

/* Includes ------------------------------------------------------------------*/

#include "lib_io/Stream.h"

#include "lib_debug/Debug.h"
#include "lib_mem/Memory.h"

#include <stdarg.h>

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

#if !defined(Memory_Config_STATIC)

int
Stream_vaprintf(Stream* self, const char* fmt, va_list args)
{
    Debug_ASSERT_SELF(self);

    static size_t capacity = 128;
    int n = -1;
    int retval = -1;

    char* msg = Memory_alloc(capacity);

    do
    {
        Debug_ASSERT(capacity <= 512); // i think more is exagerate

        n = vsnprintf(msg, capacity, fmt, args);

        if (n < -1 || n >= capacity)
        {
            size_t newCapacity = capacity * 2;
            char* tmp = Memory_realloc(msg, newCapacity);

            if (tmp == NULL)
            {
                break;
            }
            else
            {
                capacity = newCapacity;
                msg = tmp;
            }
            /* Try to print in the allocated space. */
            n = vsnprintf(msg, capacity, fmt, args);
        }
        else { /* do nothing */ }
    }
    while (n < -1 || n >= capacity);

    if (msg != NULL)
    {
        retval = (int) Stream_write(self, msg, strlen(msg));
    }
    else { /* do nothing */ }

    Memory_free(msg);

    return retval;
}

int
Stream_printf(Stream* self, const char* fmt, ...)
{
    Debug_ASSERT_SELF(self);

    int retval = -1;
    va_list args;

    va_start(args, fmt);

    retval = Stream_vaprintf(self, fmt, args);

    va_end(args);

    return retval;
}
#endif

/* Private Functions -------------------------------------------------------- */
///@}

