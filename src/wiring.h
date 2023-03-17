//
//  wiring.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef wiring_h
#define wiring_h

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "message.h"

#define BOMM_WIRING_IDENTITY BOMM_ALPHABET

typedef struct _bomm_wiring {
    bomm_letter_t forward_map[BOMM_ALPHABET_SIZE];
    bomm_letter_t backward_map[BOMM_ALPHABET_SIZE];
} bomm_wiring_t;

/**
 * Extract a wiring from the given string.
 */
bool bomm_wiring_extract(bomm_wiring_t* ptr, char* string);

/**
 * Export the given enigma wiring to a string.
 */
char* bomm_wiring_serialize(bomm_wiring_t* wiring);

#endif /* wiring_h */
