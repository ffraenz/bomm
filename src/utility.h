//
//  utility.h
//  Bomm
//
//  Created by Fränz Friederes on 23/03/2023.
//

#ifndef utility_h
#define utility_h

#include <stdlib.h>

/**
 * Safe alternative to `strncpy`: Up to `n` characters of the `src` string are
 * copied to `dest`. If the original string is longer than `n` it is truncated.
 * The copied string at `dest` will always be terminated with a null byte.
 */
inline static char* bomm_strncpy(char* dest, char* src, size_t n) {
    size_t i;
    for (i = 0; i < n - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}

/**
 * Calculate the modulo operation on the given numbers. Assumes `b > 0`.
 */
inline static int bomm_mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

#endif /* utility_h */
