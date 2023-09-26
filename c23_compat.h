//
// Created by ivan on 08.09.23.
//

#ifndef EMERGENCY_DELAY_C23_COMPAT_H
#define EMERGENCY_DELAY_C23_COMPAT_H

// https://stackoverflow.com/a/76900130

/* C++11 or later? */
#if (defined(__cplusplus) && __cplusplus >= 201103L)
#include <cstddef>

/* C2x/C23 or later? If GCC, is it version 13 or newer? */
#elif (defined(__STDC__) && \
        defined(__STDC_VERSION__) && \
        (__STDC_VERSION__ >= 202000L) && \
        (defined(__GNUC__) && __GNUC__ >= 13 || \
        !defined(__GNUC__)))

#include <stddef.h> /* nullptr_t */

/* pre C23, pre C++11 or non-standard */
#else
#define nullptr (void*)0
typedef void* nullptr_t;

#endif

#endif //EMERGENCY_DELAY_C23_COMPAT_H
