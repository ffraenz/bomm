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
#include "wiring.h"

/**
 * Return true, if the given letter is set to 1 in a lettermask.
 */
#define bomm_lettermask_has(mask, letter) ((mask >> letter) & 0x1)

/**
 * A value encoding a bit per letter in the alphabet. Useful to encode turnovers
 * or a set of start positions. The number of bits in this value must be larger
 * or equal to the number of letters in the alphabet.
 */
typedef unsigned long bomm_lettermask_t;

/**
 * Load the given lettermask string into memory at the specified pointer.
 */
void bomm_load_lettermask(bomm_lettermask_t* ptr, char* string);

/**
 * Export the given lettermask to a string.
 */
char* bomm_describe_lettermask(bomm_lettermask_t* lettermask);

#endif /* lettermask_h */
