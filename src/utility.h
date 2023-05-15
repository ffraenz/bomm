//
//  utility.h
//  Bomm
//
//  Created by Fränz Friederes on 23/03/2023.
//

#ifndef utility_h
#define utility_h

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/**
 * Safe alternative to `strncpy`: Up to `n` characters of the `src` string are
 * copied to `dest`. If the original string is longer than `n` it is truncated.
 * The copied string at `dest` will always be terminated with a null byte.
 */
inline static char* bomm_strncpy(char* dest, const char* src, size_t n) {
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

/**
 * Swap the values of the given unsigned integers.
 */
inline static void bomm_swap(unsigned int* a, unsigned int* b) {
    unsigned int tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * Swap the values of the given pointers.
 */
inline static void bomm_swap_pointer(void** a, void** b) {
    void* tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * Lookup the given string in an array of arbitrary elements.
 * @param elements Pointer to the first element's string
 * @param element_size Number of bytes until reaching the next element's string
 * @param count Number of elements
 * @param string String to lookup
 */
inline static void* bomm_lookup_string(
    void* elements,
    size_t element_size,
    unsigned int count,
    const char* string
) {
    void* match = NULL;
    char* element_string = (char*) elements;
    unsigned int i = 0;

    while (match == NULL && i < count) {
        if (strcmp(element_string, string) == 0) {
            match = element_string;
        } else {
            i++;
            element_string += element_size;
        }
    }

    return match;
}

/**
 * Shuffle the given array following the Fisher–Yates algorithm.
 */
void bomm_array_shuffle(unsigned int* array, size_t size);

/**
 * Return the wall clock time as a Unix timestamp (seconds).
 */
inline static double bomm_timestamp_sec(void) {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec + time.tv_usec / 1000000.0;
}

/**
 * Export the given duration to a string.
 */
static inline void bomm_duration_stringify(
    char* str,
    size_t size,
    double duration
) {
    snprintf(
        str,
        size,
        "%02d:%02d:%02d",
        (unsigned int) (duration / 3600),
        (unsigned int) (duration / 60) % 60,
        (unsigned int) duration % 60
    );
}

/**
 * Return the number of concurrent threads supported by the current environment.
 * Similar to `std::thread::hardware_concurrency` in C++.
 */
unsigned int bomm_hardware_concurrency(void);

#endif /* utility_h */
