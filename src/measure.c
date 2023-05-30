//
//  measure.c
//  Bomm
//
//  Created by Fränz Friederes on 22/03/2023.
//

#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE

#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include "measure.h"

bomm_ngram_map_t* bomm_ngram_map[7] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

bomm_measure_trie_config_t* bomm_measure_trie_config = NULL;

const unsigned int bomm_pow_map[7] = {
    1,
    BOMM_ALPHABET_SIZE,
    BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE,
    BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE,
    BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE *
    BOMM_ALPHABET_SIZE,
    BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE *
    BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE,
    BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE *
    BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE * BOMM_ALPHABET_SIZE
};

bomm_ngram_map_t* bomm_measure_ngram_map_init(
    unsigned char n,
    const char* filename
) {
    // Open file in reading mode
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return NULL;
    }

    // Initialize empty map
    unsigned int map_size = bomm_pow_map[n];
    size_t ngram_map_size =
        sizeof(bomm_ngram_map_t) + map_size * sizeof(bomm_ngram_map_entry);
    bomm_ngram_map_t* ngram_map = calloc(1, ngram_map_size);
    if (!ngram_map) {
        fprintf(stderr, "Out of memory while loading %d-gram map\n", n);
        return NULL;
    }
    ngram_map->n = n;

    // Parse file
    size_t line_restrict = 32;
    ssize_t line_size;
    char line[line_restrict];
    char* line_buffer = (char*) &line;
    char ascii;
    bomm_letter_t letter;
    double frequency_sum = 0;
    double frequency_min = INFINITY;
    unsigned int state = 0;
    while (state == 0 && (line_size = getline(&line_buffer, &line_restrict, file)) != -1) {
        // Reset state
        state = 0;
        unsigned int map_index = 0;
        double frequency = 0;
        unsigned int line_index = 0;

        // Parse line
        while (state != 255 && line_index < line_size) {
            ascii = line[line_index++];

            if (ascii == ' ' || ascii == '\t' || ascii == '\r' || ascii == '\n') {
                // Ignore white spaces
            } else if (state < n && (letter = bomm_message_letter_from_ascii(ascii)) != 255) {
                // Read letter and transition to next letter or frequency
                map_index = map_index * BOMM_ALPHABET_SIZE + letter;
                state++;
            } else if (state == n && (ascii >= '0' && ascii <= '9')) {
                // Read frequency digit
                // TODO: Add support for floating point numbers
                frequency = frequency * 10 + (ascii - '0');
            } else {
                // Unexpected character
                state = 255;
            }
        }

        // Store frequency
        if (state == n) {
            ngram_map->map[map_index] = frequency;
            frequency_sum += frequency;
            frequency_min = frequency < frequency_min ? frequency : frequency_min;
            state = 0;
        }
    }

    // Close file
    fclose(file);

    // Handle parsing error
    if (state == 255 || frequency_sum == 0) {
        free(ngram_map);
        fprintf(stderr, "Error parsing %d-gram file %s\n", n, filename);
        return NULL;
    }

    // When an n-gram is not listed in the dictionary, we would get
    // a probability of 0, leading to the worst possible penalty of -inf in our
    // fitness function. However, the actual probability of such an n-gram
    // appearing is not 0. That's why we set a fallback probability for these
    // cases to a value smaller than the minimum probability
    double min_probability = frequency_min / frequency_sum;
    double fallback_probability = min_probability * 0.5;

    // Turn frequencies into log probabilities
    double probability;
    for (unsigned int map_index = 0; map_index < map_size; map_index++) {
        probability = ngram_map->map[map_index] / frequency_sum;
        ngram_map->map[map_index] =
            (bomm_ngram_map_entry)
            log(probability > 0 ? probability : fallback_probability);
    }

    bomm_ngram_map[n] = ngram_map;
    return ngram_map;
}

void bomm_measure_config_destroy(void) {
    // Frequency n-gram maps
    unsigned int num_ngram_maps =
        sizeof(bomm_ngram_map) / sizeof(bomm_ngram_map[0]);
    for (unsigned int i = 0; i < num_ngram_maps; i++) {
        if (bomm_ngram_map[i] != NULL) {
            free(bomm_ngram_map[i]);
            bomm_ngram_map[i] = NULL;
        }
    }

    // Trie measure config
    if (bomm_measure_trie_config != NULL) {
        bomm_trie_destroy(bomm_measure_trie_config->trie);
        free(bomm_measure_trie_config->trie);
        free(bomm_measure_trie_config);
    }
}
