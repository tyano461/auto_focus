#ifndef __FFT_DEBUG_H__
#define __FFT_DEBUG_H__

#include <stdio.h>

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define d(s, ...)                                                                     \
    do                                                                                \
    {                                                                                 \
        printf("%s(%d) %s " s "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#define ERRRET(c, s, ...)                                                                 \
    do                                                                                    \
    {                                                                                     \
        if (c)                                                                            \
        {                                                                                 \
            printf("%s(%d) %s " s "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); \
            goto error_return;                                                            \
        }                                                                                 \
    } while (0)

#define ERRRETf(c, formula)    \
    do                         \
    {                          \
        if (c)                 \
        {                      \
            formula;           \
            goto error_return; \
        }                      \
    } while (0)


#define ERRRETnp(c)            \
    do                         \
    {                          \
        if (c)                 \
        {                      \
            goto error_return; \
        }                      \
    } while (0)

#endif