//
//  wiring.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "wiring.h"

bool bomm_wiring_extract(bomm_wiring_t* ptr, char* string) {
    bomm_message_t* message = bomm_alloc_message(string);
    if (!message) {
        return false;
    }

    memset(ptr, BOMM_ALPHABET_SIZE, sizeof(bomm_wiring_t));

    unsigned char char_index;
    bool error = message->length != BOMM_ALPHABET_SIZE;
    unsigned int position = 0;
    while (!error && position < message->length) {
        char_index = message->letters[position];
        if (ptr->backward_map[char_index] == BOMM_ALPHABET_SIZE) {
            ptr->forward_map[position] = char_index;
            ptr->backward_map[char_index] = position;
            position++;
        } else {
            error = true;
        }
    }

    free(message);

    if (error) {
        fprintf(stderr, "Encountered unexpected wiring string '%s' at position %d\n", string, position);
        return false;
    }
    
    return true;
}

char* bomm_wiring_serialize(bomm_wiring_t* wiring) {
    char* string = malloc(sizeof(char) * BOMM_ALPHABET_SIZE);
    for (int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        string[i] = wiring->forward_map[i] + 97;
    }
    return string;
}
