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
#include "wiring.h"

typedef float bomm_ngram_map_entry;

typedef struct _bomm_ngram_map_t {
    /**
     * The n in n-gram
     */
    unsigned char n;
    
    /**
     * Maps n-grams to their respective log probabilities
     */
    bomm_ngram_map_entry map[];
} bomm_ngram_map_t;

/**
 * Allocate an n-gram map and load the frequency from the given file.
 */
bomm_ngram_map_t* bomm_measure_ngram_map_init(unsigned char n, char* filename);

/**
 * n-gram score a ciphertext put through the given scrabler and plugboard.
 * @param n 1 for unigram, 2 for bigram, 3 for bigram, etc.
 */
static inline float bomm_measure_scrambler_ngram(
    unsigned int n,
    bomm_scrambler_t* scrambler,
    bomm_letter_t* plugboard,
    bomm_message_t* ciphertext,
    bomm_ngram_map_t* ngram_map
) {
    unsigned int map_size = pow(BOMM_ALPHABET_SIZE, n);
    unsigned int index, letter;

    float score = 0;
    unsigned int map_index = 0;

    for (index = 0; index < ciphertext->length; index++) {
        letter = ciphertext->letters[index];
        letter = plugboard[letter];
        letter = scrambler->map[index][letter];
        letter = plugboard[letter];

        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % map_size;
        
        if (index >= n - 1) {
            score += ngram_map->map[map_index];
        }
    }

    return score / (ciphertext->length - n + 1);
}

/**
 * Calculate the normalized Index of coincidence (IC) for the given message.
 *
 * Definition: `\text{IC} = c\frac{\sum_{i=1}^cf_i(f_i-1)}{n(n-1)}`
 * with `f_i` appearances of letter `i`, `n` the total number of letters, and
 * `c` the number of letters in the alphabet.
 *
 * @param frequencies Array of frequencies
 * @param n Length of the message the frequencies have been collected from
 * @param c Number of frequencies (alphabet size or power of it)
 */
static inline float bomm_measure_ic(
    unsigned int* frequencies,
    unsigned int n,
    unsigned int c
) {
    unsigned int coincidence = 0;
    for (unsigned int index = 0; index < c; index++) {
        coincidence += frequencies[index] * (frequencies[index] - 1);
    }
    return (float) (c * coincidence) / (n * (n - 1));
}

/**
 * IC score a ciphertext put through the given scrabler and plugboard.
 */
static inline float bomm_measure_scrambler_ic(
    bomm_scrambler_t* scrambler,
    bomm_letter_t* plugboard,
    bomm_message_t* ciphertext
) {
    unsigned int index, letter;
    unsigned int letter_frequencies[BOMM_ALPHABET_SIZE];
    memset(letter_frequencies, 0, BOMM_ALPHABET_SIZE * sizeof(unsigned int));
    
    for (index = 0; index < ciphertext->length; index++) {
        letter = ciphertext->letters[index];
        letter = plugboard[letter];
        letter = scrambler->map[index][letter];
        letter = plugboard[letter];
        letter_frequencies[letter]++;
    }
    
    return bomm_measure_ic(letter_frequencies, ciphertext->length, BOMM_ALPHABET_SIZE);
}

/**
 * Calculate the Entropy for the given message.
 *
 * @param frequencies Array of frequencies
 * @param n Length of the message the frequencies have been collected from
 * @param c Number of frequencies (alphabet size or power of it)
 */
static inline float bomm_measure_entropy(
    unsigned int* frequencies,
    unsigned int n,
    unsigned int c
) {
    double entropy = 0;
    double p;
    for (unsigned int index = 0; index < c; index++) {
        p = frequencies[index] / (float) n;
        entropy -= p * log2(p);
    }
    return (float) entropy;
}

#endif /* measure_h */
