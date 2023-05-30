//
//  measure.h
//  Bomm
//
//  Created by Fränz Friederes on 22/03/2023.
//

#ifndef measure_h
#define measure_h

#include <jansson.h>
#include "message.h"
#include "wiring.h"
#include "trie.h"

/**
 * Enum identifying a measure
 */
typedef enum {
    BOMM_MEASURE_SINKOV_MONOGRAM   = 0x01,
    BOMM_MEASURE_SINKOV_BIGRAM     = 0x02,
    BOMM_MEASURE_SINKOV_TRIGRAM    = 0x03,
    BOMM_MEASURE_SINKOV_QUADGRAM   = 0x04,
    BOMM_MEASURE_SINKOV_PENTAGRAM  = 0x05,
    BOMM_MEASURE_SINKOV_HEXAGRAM   = 0x06,
    BOMM_MEASURE_IC                = 0x11,
    BOMM_MEASURE_IC_BIGRAM         = 0x12,
    BOMM_MEASURE_IC_TRIGRAM        = 0x13,
    BOMM_MEASURE_IC_QUADGRAM       = 0x14,
    BOMM_MEASURE_IC_PENTAGRAM      = 0x15,
    BOMM_MEASURE_IC_HEXAGRAM       = 0x16,
    BOMM_MEASURE_ENTROPY           = 0x21,
    BOMM_MEASURE_ENTROPY_BIGRAM    = 0x22,
    BOMM_MEASURE_ENTROPY_TRIGRAM   = 0x23,
    BOMM_MEASURE_ENTROPY_QUADGRAM  = 0x24,
    BOMM_MEASURE_ENTROPY_PENTAGRAM = 0x25,
    BOMM_MEASURE_ENTROPY_HEXAGRAM  = 0x26,
    BOMM_MEASURE_TRIE              = 0xf0,
    BOMM_MEASURE_NONE              = 0xff
} bomm_measure_t;

typedef struct _bomm_measure_string_mapping {
    bomm_measure_t value;
    const char* name;
} bomm_measure_string_mapping_t;

/**
 * Lookup table mapping measure values to names
 */
static const bomm_measure_string_mapping_t bomm_measure_string_map[] = {
    { BOMM_MEASURE_ENTROPY,           "entropy" },
    { BOMM_MEASURE_ENTROPY_BIGRAM,    "entropy_bigram" },
    { BOMM_MEASURE_ENTROPY_TRIGRAM,   "entropy_trigram" },
    { BOMM_MEASURE_ENTROPY_QUADGRAM,  "entropy_quadgram" },
    { BOMM_MEASURE_ENTROPY_PENTAGRAM, "entropy_pentagram" },
    { BOMM_MEASURE_ENTROPY_HEXAGRAM,  "entropy_hexagram" },
    { BOMM_MEASURE_IC,                "ic" },
    { BOMM_MEASURE_IC_BIGRAM,         "ic_bigram" },
    { BOMM_MEASURE_IC_TRIGRAM,        "ic_trigram" },
    { BOMM_MEASURE_IC_QUADGRAM,       "ic_quadgram" },
    { BOMM_MEASURE_IC_PENTAGRAM,      "ic_pentagram" },
    { BOMM_MEASURE_IC_HEXAGRAM,       "ic_hexagram" },
    { BOMM_MEASURE_SINKOV_MONOGRAM,   "sinkov_monogram" },
    { BOMM_MEASURE_SINKOV_BIGRAM,     "sinkov_bigram" },
    { BOMM_MEASURE_SINKOV_TRIGRAM,    "sinkov_trigram" },
    { BOMM_MEASURE_SINKOV_QUADGRAM,   "sinkov_quadgram" },
    { BOMM_MEASURE_SINKOV_PENTAGRAM,  "sinkov_pentagram" },
    { BOMM_MEASURE_SINKOV_HEXAGRAM,   "sinkov_hexagram" },
    { BOMM_MEASURE_TRIE,              "trie" },
    { BOMM_MEASURE_NONE,              "none" }
};

typedef float bomm_ngram_map_entry;

typedef struct _bomm_ngram_map {
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
extern bomm_ngram_map_t* bomm_ngram_map[7];

/**
 * Set of configuration options for the trie measure.
 */
typedef struct _bomm_measure_trie_config {
    bomm_trie_t* trie;
    bomm_measure_t base_measure;
} bomm_measure_trie_config_t;

/**
 * Global variable storing the trie measure config.
 */
extern bomm_measure_trie_config_t* bomm_measure_trie_config;

/**
 * Global variable storing pre-calculated `pow(BOMM_ALPHABET_SIZE, index)` for
 * each index to reuse these values.
 */
extern const unsigned int bomm_pow_map[7];

/**
 * Allocate an n-gram frequency map in memory and fill it with the contents
 * parsed from the given file.
 * Stores the pointer to the global variable `bomm_ngram_map[n]`.
 */
bomm_ngram_map_t* bomm_measure_ngram_map_init(
    unsigned char n,
    const char* filename
);

/**
 * Destroy and free global measure config values.
 */
void bomm_measure_config_destroy(void);

/**
 * Measure the n-gram score of a message put through the given
 * scrabler and plugboard.
 * @param n The n in n-gram
 */
static inline double bomm_measure_scrambler_sinkov(
    unsigned int n,
    bomm_scrambler_t* scrambler,
    unsigned int* plugboard,
    bomm_message_t* message
) {
    unsigned int map_size = bomm_pow_map[n];
    const bomm_ngram_map_t* map = bomm_ngram_map[n];
    unsigned int index, letter;

    double score = 0;
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

    return score / (double) (message->length - n + 1);
}

/**
 * Measure the n-gram score of the given message.
 * @param n The n in n-gram
 */
static inline double bomm_measure_message_sinkov(
    unsigned int n,
    bomm_message_t* message
) {
    unsigned int map_size = bomm_pow_map[n];
    const bomm_ngram_map_t* map = bomm_ngram_map[n];
    unsigned int index, letter;

    double score = 0;
    unsigned int map_index = 0;

    for (index = 0; index < message->length; index++) {
        letter = message->letters[index];
        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % map_size;

        if (index >= n - 1) {
            score += map->map[map_index];
        }
    }

    return score / (double) (message->length - n + 1);
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
    unsigned int num_frequencies = bomm_pow_map[n];
    memset(frequencies, 0, num_frequencies * sizeof(unsigned int));

    unsigned int letter;
    unsigned int map_index = 0;
    for (unsigned int index = 0; index < message->length; index++) {
        letter = message->letters[index];
        letter = plugboard[letter];
        letter = scrambler->map[index][letter];
        letter = plugboard[letter];

        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % num_frequencies;
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
    unsigned int num_frequencies = bomm_pow_map[n];
    memset(frequencies, 0, num_frequencies * sizeof(unsigned int));

    unsigned int letter;
    unsigned int map_index = 0;
    for (unsigned int index = 0; index < message->length; index++) {
        letter = message->letters[index];
        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % num_frequencies;
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
static inline double bomm_measure_frequency_ic(
    unsigned int n,
    unsigned int* frequencies
) {
    unsigned int num_frequencies = bomm_pow_map[n];
    unsigned int coincidence = 0;
    unsigned int sum = 0;
    unsigned int frequency;
    for (unsigned int index = 0; index < num_frequencies; index++) {
        frequency = frequencies[index];
        coincidence += frequency * (frequency - 1);
        sum += frequency;
    }
    return (double) (num_frequencies * coincidence) / (double) (sum * (sum - 1));
}

/**
 * Calculate the Entropy for the given message in bits.
 * @param n The n in n-gram
 * @param frequencies Frequencies map of size `pow(BOMM_ALPHABET_SIZE, n)`
 */
static inline double bomm_measure_frequency_entropy(
    unsigned int n,
    unsigned int* frequencies
) {
    unsigned int num_frequencies = bomm_pow_map[n];

    unsigned int sum = 0;
    unsigned int index;
    for (index = 0; index < num_frequencies; index++) {
        sum += frequencies[index];
    }

    double entropy = 0;
    double p;
    if (sum > 0) {
        for (index = 0; index < num_frequencies; index++) {
            p = (double) frequencies[index] / (double) sum;
            entropy -= (p > 0) ? (p * log2(p)) : 0;
        }
    }
    return entropy;
}

/**
 * Return the measure value from the given string.
 */
static inline bomm_measure_t bomm_measure_from_string(
    const char* measure_string
) {
    unsigned int num_mappings =
        sizeof(bomm_measure_string_map) /
        sizeof(bomm_measure_string_mapping_t);
    unsigned int i = 0;
    bomm_measure_t measure = BOMM_MEASURE_NONE;
    while (measure == BOMM_MEASURE_NONE && i < num_mappings) {
        if (strcmp(bomm_measure_string_map[i].name, measure_string) == 0) {
            measure = bomm_measure_string_map[i].value;
        }
        i++;
    }
    return measure;
}

/**
 * Return the measure value from the given JSON value.
 */
static inline bomm_measure_t bomm_measure_from_json(
    const json_t* measure_json
) {
    if (!json_is_string(measure_json)) {
        return BOMM_MEASURE_NONE;
    } else {
        return bomm_measure_from_string(json_string_value(measure_json));
    }
}

/**
 * Return the string value for the given measure.
 */
static inline const char* bomm_measure_to_string(bomm_measure_t measure) {
    unsigned int num_mappings =
        sizeof(bomm_measure_string_map) /
        sizeof(bomm_measure_string_mapping_t);
    unsigned int i = 0;
    const char* string = NULL;
    while (string == NULL && i < num_mappings) {
        if (bomm_measure_string_map[i].value == measure) {
            string = bomm_measure_string_map[i].name;
        }
        i++;
    }
    return string;
}

/**
 * Measure a message
 */
static inline double bomm_measure_message(
    bomm_measure_t measure,
    bomm_message_t* message
) {
    if (measure < 0x10) {
        unsigned int n = measure;
        return bomm_measure_message_sinkov(n, message);
    } else if (measure < 0x20) {
        unsigned int n = measure - 0x10;
        unsigned int frequencies[bomm_pow_map[n]];
        bomm_measure_message_frequency(n, frequencies, message);
        return bomm_measure_frequency_ic(n, frequencies);
    } else if (measure < 0x30) {
        unsigned int n = measure - 0x20;
        unsigned int frequencies[bomm_pow_map[n]];
        bomm_measure_message_frequency(n, frequencies, message);
        return bomm_measure_frequency_entropy(n, frequencies);
    } else if (measure == BOMM_MEASURE_TRIE && bomm_measure_trie_config != NULL) {
        bomm_measure_t base_measure = bomm_measure_trie_config->base_measure;
        double score = 0;
        if (base_measure != BOMM_MEASURE_NONE) {
            score = bomm_measure_message(base_measure, message);
        }
        score += bomm_trie_measure_message(
            bomm_measure_trie_config->trie, message);
        return score;
    }
    return 0;
}

/**
 * Measure a message put through the given scrabler and plugboard
 */
static inline __attribute__((always_inline)) double bomm_measure_scrambler(
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
        unsigned int frequencies[bomm_pow_map[n]];
        bomm_measure_scrambler_frequency(n, frequencies, scrambler, plugboard, message);
        return bomm_measure_frequency_ic(n, frequencies);
    } else if (measure < 0x30) {
        unsigned int n = measure - 0x20;
        unsigned int frequencies[bomm_pow_map[n]];
        bomm_measure_scrambler_frequency(n, frequencies, scrambler, plugboard, message);
        return bomm_measure_frequency_entropy(n, frequencies);
    } else if (measure == BOMM_MEASURE_TRIE && bomm_measure_trie_config != NULL) {
        size_t message_size = bomm_message_size_for_length(message->length);
        bomm_message_t* plaintext = malloc(message_size);
        if (plaintext != NULL) {
            bomm_scrambler_encrypt(scrambler, plugboard, message, plaintext);
            bomm_measure_t base_measure = bomm_measure_trie_config->base_measure;
            double score = 0;
            if (base_measure != BOMM_MEASURE_NONE) {
                score = bomm_measure_message(base_measure, plaintext);
            }
            score += bomm_trie_measure_message(
                bomm_measure_trie_config->trie, plaintext);
            free(plaintext);
            return score;
        }
    }
    return 0;
}

#endif /* measure_h */
