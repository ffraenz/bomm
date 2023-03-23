//
//  enigma.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef enigma_h
#define enigma_h

#include <stdio.h>
#include <stdlib.h>
#include "key.h"
#include "utility.h"

/**
 * Variable-size struct representing the Enigma scrambler mapping of letters
 * for the given number of letters in a message.
 */
typedef struct _bomm_scrambler {
    /**
     * Number of letters mapped in the scrambler
     */
    unsigned int length;
    
    /**
     * Map
     */
    bomm_letter_t map[][BOMM_ALPHABET_SIZE];
} bomm_scrambler_t;

/**
 * Calculate the scrambler struct size for the given message length.
 */
static inline size_t bomm_scrambler_size(unsigned int length) {
    return
        sizeof(bomm_scrambler_t) +
        length * BOMM_ALPHABET_SIZE * sizeof(bomm_letter_t);
}

/**
 * Simulate the Enigma on the given original message and key.
 * Original and result message structs are assumed to be of same length.
 */
void bomm_encrypt(bomm_message_t* original, bomm_key_t* key, bomm_message_t* result);

/**
 * Simulate the Enigma on the given key and generate a scrambler map for each
 * letter position in a message and letter.
 */
void bomm_scrambler_generate(bomm_scrambler_t* scrambler, bomm_key_t* key);

#endif /* enigma_h */
