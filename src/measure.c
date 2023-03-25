//
//  measure.c
//  Bomm
//
//  Created by Fränz Friederes on 22/03/2023.
//

#include "measure.h"

void* bomm_measure_ngram_map_alloc(unsigned char n, char* filename) {
    // Open file in reading mode
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return NULL;
    }
    
    // Allocate map
    unsigned int map_size = pow(BOMM_ALPHABET_SIZE, n);
    unsigned char* ngram_map = calloc(map_size, sizeof(unsigned char));
    if (!ngram_map) {
        fprintf(stderr, "Out of memory while loading ngram map\n");
        return NULL;
    }
    
    // Parse file
    size_t line_restrict = 32;
    ssize_t line_size;
    char line[line_restrict];
    char* line_buffer = (char*) &line;
    unsigned int state, map_index, line_index;
    float frequency, max_frequency;
    char ascii;
    unsigned char letter;
    
    max_frequency = 255;
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
                frequency = frequency * 10 + (ascii - '0');
            } else {
                // Unexpected character
                state = 255;
            }
        }
        
        // Store frequency
        if (state == n) {
            // Here we are assuming trigram files are sorted by frequency
            // in decending order
            if (frequency > max_frequency) {
                max_frequency = frequency;
            }
            
            ngram_map[map_index] = (char) ((frequency / max_frequency) * 255);
            state = 0;
        }
    }
    
    // Close file
    fclose(file);
    
    // Handle parsing error
    if (state == 255) {
        free(ngram_map);
        fprintf(stderr, "Error parsing n-gram file\n");
        return NULL;
    }
    
    return ngram_map;
}
