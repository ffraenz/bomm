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

/**
 * Struct representing wheel, reflector, and plugboard wirings in
 * memory, providing a forward and backward map for efficient mapping.
 */
typedef struct _bomm_wiring {
    /**
     * Forward map
     */
    bomm_letter_t map[BOMM_ALPHABET_SIZE];
    
    /**
     * Reverse map
     */
    bomm_letter_t rev[BOMM_ALPHABET_SIZE];
} bomm_wiring_t;

/**
 * Extract a wiring from the given string
 */
bool bomm_wiring_extract(bomm_wiring_t* ptr, char* string);

/**
 * Export the given enigma wiring to a string
 */
char* bomm_wiring_serialize(bomm_wiring_t* wiring);

#endif /* wiring_h */
