//
//  measure.h
//  Bomm
//
//  Created by Fränz Friederes on 22/03/2023.
//

#ifndef measure_h
#define measure_h

#include <stdio.h>
#include <math.h>
#include "message.h"

/**
 * Allocate an n-gram map and load the frequency from the given file.
 */
void* bomm_measure_ngram_map_alloc(unsigned char n, char* path);

/**
 * Calculate the normalized Index of coincidence (IC) for the given message.
 *
 * Definition: \text{IC} = c\frac{\sum_{i=1}^cf_i(f_i-1)}{n(n-1)}
 * with f_i appearances of letter i, n the total number of letters, and c the
 * number of letters in the alphabet.
 */
static inline float bomm_measure_ic(bomm_message_t* message) {
    unsigned int coincidence = 0;
    for (unsigned int letter = 0; letter < BOMM_ALPHABET_SIZE; letter++) {
        coincidence += message->frequency[letter] * (message->frequency[letter] - 1);
    }
    return (float) (BOMM_ALPHABET_SIZE * coincidence) / (message->length * (message->length - 1));
}

/**
 * Calculate the Entropy for the given message.
 */
static inline float bomm_measure_entropy(bomm_message_t* message) {
    float entropy = 0;
    float p;
    for (unsigned int letter = 0; letter < BOMM_ALPHABET_SIZE; letter++) {
        p = message->letters[letter] / (float) message->length;
        entropy -= p * log2(p);
    }
    return entropy;
}

#endif /* measure_h */
