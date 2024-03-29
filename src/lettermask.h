//
//  lettermask.h
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#ifndef lettermask_h
#define lettermask_h

#include <stdio.h>
#include <stdbool.h>
#include "message.h"

/**
 * Mask with all letters selected
 */
#define BOMM_LETTERMASK_ALL ((1 << BOMM_ALPHABET_SIZE) - 1)

/**
 * Mask with no letters selected
 */
#define BOMM_LETTERMASK_NONE 0

/**
 * Mask with only the first letter selected
 */
#define BOMM_LETTERMASK_FIRST 1

/**
 * Mask with only the last letter selected
 */
#define BOMM_LETTERMASK_LAST (1 << (BOMM_ALPHABET_SIZE - 1))

/**
 * A value encoding a bit per letter in the alphabet. Useful to encode turnovers
 * or a set of start positions. The number of bits in this value must be larger
 * or equal to the number of letters in the alphabet.
 */
typedef unsigned long bomm_lettermask_t;

/**
 * Return true, if the given letter is set to 1 in a lettermask.
 */
inline static bool bomm_lettermask_has(
    const bomm_lettermask_t* mask,
    bomm_letter_t letter
) {
    return (*mask >> letter) & 0x1;
}

/**
 * Add a letter to the given mask.
 */
inline static void bomm_lettermask_set(
    bomm_lettermask_t* mask,
    bomm_letter_t letter
) {
    *mask |= (1 << letter);
}

/**
 * Remove a letter from the given mask.
 */
inline static void bomm_lettermask_clear(
    bomm_lettermask_t* mask,
    bomm_letter_t letter
) {
    *mask &= ~(1 << letter);
}

/**
 * Count the number of letters contained in the given mask.
 */
inline static unsigned int bomm_lettermask_count(
    const bomm_lettermask_t* mask
) {
    bomm_lettermask_t shifting_mask = *mask;
    unsigned int num_letters = 0;
    while (shifting_mask != BOMM_LETTERMASK_NONE) {
        num_letters += shifting_mask & 0x1;
        shifting_mask = shifting_mask >> 1;
    }
    return num_letters;
}

/**
 * Load the given lettermask string into memory at the specified pointer
 */
bomm_lettermask_t* bomm_lettermask_from_string(
    bomm_lettermask_t* lettermask,
    const char* string
);

/**
 * Export the given lettermask to a string
 */
void bomm_lettermask_stringify(
    char* str,
    size_t size,
    const bomm_lettermask_t* lettermask
);

#endif /* lettermask_h */
