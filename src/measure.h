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

#define BOMM_MAX_INDEXED_MEASURE 9

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
 * Global variable storing pointers to n-gram maps that have been initialized
 * previously. The array index specifies the n in n-gram.
 */
extern const bomm_ngram_map_t* bomm_ngram_map[9];

/**
 * Allocate an n-gram frequency map in memory and fill it with the contents
 * parsed from the given file.
 * Stores the pointer to the global variable `bomm_ngram_map[n]`.
 */
bomm_ngram_map_t* bomm_measure_ngram_map_init(unsigned char n, const char* filename);

/**
 * Measure the n-gram score of a message put through the given
 * scrabler and plugboard.
 * @param n The n in n-gram
 */
static inline float bomm_measure_scrambler_sinkov(
    unsigned int n,
    bomm_scrambler_t* scrambler,
    unsigned int* plugboard,
    bomm_message_t* message
) {
    unsigned int map_size = pow(BOMM_ALPHABET_SIZE, n);
    const bomm_ngram_map_t* map = bomm_ngram_map[n];
    unsigned int index, letter;

    float score = 0;
    unsigned int map_index = 0;

    for (index = 0; index < message->length; index++) {
        letter = message->letters[index];
        letter = plugboard[letter];
        letter = scrambler->map[index][letter];
        letter = plugboard[letter];

        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % map_size;

        if (index >= n - 1) {
            score += map->map[map_index];
        }
    }

    return score / (message->length - n + 1);
}

/**
 * Measure the n-gram score of the given message.
 * @param n The n in n-gram
 */
static inline float bomm_measure_message_sinkov(
    unsigned int n,
    bomm_message_t* message
) {
    unsigned int map_size = pow(BOMM_ALPHABET_SIZE, n);
    const bomm_ngram_map_t* map = bomm_ngram_map[n];
    unsigned int index, letter;

    float score = 0;
    unsigned int map_index = 0;

    for (index = 0; index < message->length; index++) {
        letter = message->letters[index];
        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % map_size;

        if (index >= n - 1) {
            score += map->map[map_index];
        }
    }

    return score / (message->length - n + 1);
}

/**
 * Measure the n-gram frequency of the given scrambler, plugboard, and message.
 * @param n The n in n-gram
 * @param frequencies Frequencies map of size `pow(BOMM_ALPHABET_SIZE, n)`
 */
static inline void bomm_measure_scrambler_frequency(
    unsigned int n,
    unsigned int* frequencies,
    bomm_scrambler_t* scrambler,
    unsigned int* plugboard,
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
 * Measure the n-gram frequency of the given message.
 * TODO: Optimize speed for monograms.
 * @param n The n in n-gram
 * @param frequencies Frequencies map of size `pow(BOMM_ALPHABET_SIZE, n)`
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
 * Calculate the normalized Index of coincidence (IC) for the given message.
 *
 * Definition: `\text{IC} = c\frac{\sum_{i=1}^cf_i(f_i-1)}{n(n-1)}`
 * with `f_i` appearances of letter `i`, `n` the total number of letters, and
 * `c` the number of letters in the alphabet.
 *
 * @param n The n in n-gram
 * @param frequencies Frequencies map of size `pow(BOMM_ALPHABET_SIZE, n)`
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
 * @param n The n in n-gram
 * @param frequencies Frequencies map of size `pow(BOMM_ALPHABET_SIZE, n)`
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

/**
 * Enum identifying a measure for scoring plaintext candidates.
 */
typedef enum {
    BOMM_MEASURE_SINKOV           = 0x01,
    BOMM_MEASURE_SINKOV_BIGRAM    = 0x02,
    BOMM_MEASURE_SINKOV_TRIGRAM   = 0x03,
    BOMM_MEASURE_SINKOV_QUADGRAM  = 0x04,
    BOMM_MEASURE_SINKOV_PENTAGRAM = 0x05,
    BOMM_MEASURE_SINKOV_HEXAGRAM  = 0x06,
    BOMM_MEASURE_SINKOV_HEPTAGRAM = 0x07,
    BOMM_MEASURE_SINKOV_OCTAGRAM  = 0x08,

    BOMM_MEASURE_IC           = 0x11,
    BOMM_MEASURE_IC_BIGRAM    = 0x12,
    BOMM_MEASURE_IC_TRIGRAM   = 0x13,
    BOMM_MEASURE_IC_QUADGRAM  = 0x14,
    BOMM_MEASURE_IC_PENTAGRAM = 0x15,
    BOMM_MEASURE_IC_HEXAGRAM  = 0x16,
    BOMM_MEASURE_IC_HEPTAGRAM = 0x17,
    BOMM_MEASURE_IC_OCTAGRAM  = 0x18
} bomm_measure_t;

/**
 * Measure a message
 */
static inline float bomm_measure_message(
    bomm_measure_t measure,
    bomm_message_t* message
) {
    if (measure < 0x10) {
        unsigned int n = measure;
        return bomm_measure_message_sinkov(n, message);
    } else if (measure < 0x20) {
        unsigned int n = measure - 0x10;
        unsigned int frequencies[(unsigned int) pow(BOMM_ALPHABET_SIZE, n)];
        bomm_measure_message_frequency(n, frequencies, message);
        return bomm_measure_frequency_ic(n, frequencies);
    } else {
        return 0;
    }
}

/**
 * Measure a message put through the given scrabler and plugboard
 */
static inline float bomm_measure_scrambler(
    bomm_measure_t measure,
    bomm_scrambler_t* scrambler,
    unsigned int* plugboard,
    bomm_message_t* message
) {
    if (measure < 0x10) {
        unsigned int n = measure;
        return bomm_measure_scrambler_sinkov(n, scrambler, plugboard, message);
    } else if (measure < 0x20) {
        unsigned int n = measure - 0x10;
        unsigned int frequencies[(unsigned int) pow(BOMM_ALPHABET_SIZE, n)];
        bomm_measure_scrambler_frequency(n, frequencies, scrambler, plugboard, message);
        return bomm_measure_frequency_ic(n, frequencies);
    } else {
        return 0;
    }
}

#endif /* measure_h */
