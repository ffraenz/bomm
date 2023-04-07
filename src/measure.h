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
 * Measure the n-gram frequency of the given message.
 * The frequency array is expected to be of size `BOMM_ALPHABET_SIZE^n`.
 * TODO: Optimize speed for monograms.
 */
static inline void bomm_measure_message_frequency(
    unsigned int n,
    unsigned int* frequencies,
    bomm_message_t* message
) {
    unsigned int count = pow(BOMM_ALPHABET_SIZE, n);
    memset(frequencies, 0, count * sizeof(unsigned int));
    
    unsigned int letter;
    unsigned int map_index = 0;
    for (unsigned int index = 0; index < message->length; index++) {
        letter = message->letters[index];
        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % count;
        if (index >= n - 1) {
            frequencies[map_index]++;
        }
    }
}

/**
 * Measure the n-gram frequency of the given scrambler, plugboard, and message.
 * The frequency array is expected to be of size `BOMM_ALPHABET_SIZE^n`.
 */
static inline void bomm_measure_scrambler_frequency(
    unsigned int n,
    unsigned int* frequencies,
    bomm_scrambler_t* scrambler,
    bomm_letter_t* plugboard,
    bomm_message_t* message
) {
    unsigned int count = pow(BOMM_ALPHABET_SIZE, n);
    memset(frequencies, 0, count * sizeof(unsigned int));
    
    unsigned int letter;
    unsigned int map_index = 0;
    for (unsigned int index = 0; index < message->length; index++) {
        letter = message->letters[index];
        letter = plugboard[letter];
        letter = scrambler->map[index][letter];
        letter = plugboard[letter];
        
        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % count;
        if (index >= n - 1) {
            frequencies[map_index]++;
        }
    }
}

/**
 * Calculate the normalized Index of coincidence (IC) for the given message.
 *
 * Definition: `\text{IC} = c\frac{\sum_{i=1}^cf_i(f_i-1)}{n(n-1)}`
 * with `f_i` appearances of letter `i`, `n` the total number of letters, and
 * `c` the number of letters in the alphabet.
 */
static inline float bomm_measure_frequency_ic(
    unsigned int n,
    unsigned int* frequencies
) {
    unsigned int count = pow(BOMM_ALPHABET_SIZE, n);
    unsigned int coincidence = 0;
    unsigned int sum = 0;
    unsigned int frequency;
    for (unsigned int index = 0; index < count; index++) {
        frequency = frequencies[index];
        coincidence += frequency * (frequency - 1);
        sum += frequency;
    }
    return (float) (count * coincidence) / (sum * (sum - 1));
}

/**
 * Calculate the Entropy for the given message in bits.
 */
static inline float bomm_measure_frequency_entropy(
    unsigned int n,
    unsigned int* frequencies
) {
    unsigned int count = pow(BOMM_ALPHABET_SIZE, n);
    
    unsigned int sum = 0;
    unsigned int index;
    for (index = 0; index < count; index++) {
        sum += frequencies[index];
    }
    
    double entropy = 0;
    double p;
    if (sum > 0) {
        for (index = 0; index < count; index++) {
            p = (double) frequencies[index] / sum;
            entropy -= (p > 0) ? (p * log2(p)) : 0;
        }
    }
    return (float) entropy;
}

#endif /* measure_h */
