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
 * Variable-size struct representing multiple letter mappings.
 */
typedef struct _bomm_scrambler {
    /**
     * Number of letter maps contained
     */
    unsigned int length;
    
    /**
     * Letter maps
     */
    bomm_letter_t map[][BOMM_ALPHABET_SIZE];
} bomm_scrambler_t;

/**
 * Extract a wiring from the given string
 */
bool bomm_wiring_extract(bomm_wiring_t* ptr, const char* string);

/**
 * Export the given enigma wiring to a string
 */
void bomm_wiring_serialize(char* str, size_t size, bomm_wiring_t* wiring);

/**
 * Calculate the scrambler struct size for the given message length.
 */
static inline size_t bomm_scrambler_size(unsigned int length) {
    return
        sizeof(bomm_scrambler_t) +
        length * BOMM_ALPHABET_SIZE * sizeof(bomm_letter_t);
}

/**
 * Encrypt a message using the given scrambler mapping and plugboard.
 */
static inline void bomm_scrambler_encrypt(
    bomm_scrambler_t* scrambler,
    unsigned int* plugboard,
    bomm_message_t* message,
    bomm_message_t* result
) {
    result->length = message->length;
    for (unsigned int i = 0; i < message->length; i++) {
        result->letters[i] =
            plugboard[scrambler->map[i][plugboard[message->letters[i]]]];
    }
}

#endif /* wiring_h */
