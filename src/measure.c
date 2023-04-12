//
//  measure.c
//  Bomm
//
//  Created by Fränz Friederes on 22/03/2023.
//

#include "measure.h"

const bomm_ngram_map_t* bomm_ngram_map[9] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

bomm_ngram_map_t* bomm_measure_ngram_map_init(unsigned char n, const char* filename) {
    // Open file in reading mode
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return NULL;
    }
    
    // Allocate map
    unsigned int map_size = pow(BOMM_ALPHABET_SIZE, n);
    size_t ngram_map_size = sizeof(bomm_ngram_map_t) + map_size * sizeof(bomm_ngram_map_entry);
    bomm_ngram_map_t* ngram_map = malloc(ngram_map_size);
    if (!ngram_map) {
        fprintf(stderr, "Out of memory while loading %d-gram map\n", n);
        return NULL;
    }
    
    size_t line_restrict = 32;
    ssize_t line_size;
    char line[line_restrict];
    char* line_buffer = (char*) &line;
    unsigned int state, map_index, line_index;
    double frequency, frequency_sum, frequency_min;
    char ascii;
    bomm_letter_t letter;
    
    // Reset
    ngram_map->n = n;
    memset(ngram_map->map, 0, map_size);
    
    // Parse file
    frequency_sum = 0;
    frequency_min = INFINITY;
    state = 0;
    while (state == 0 && (line_size = getline(&line_buffer, &line_restrict, file)) != -1) {
        // Reset state
        state = 0;
        map_index = 0;
        frequency = 0;
        line_index = 0;
        
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
    for (map_index = 0; map_index < map_size; map_index++) {
        probability = ngram_map->map[map_index] / frequency_sum;
        ngram_map->map[map_index] =
            (float) log(probability > 0 ? probability : fallback_probability);
    }
    
    bomm_ngram_map[n] = ngram_map;
    return ngram_map;
}
