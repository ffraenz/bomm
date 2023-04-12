//
//  wiring.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "wiring.h"

bool bomm_wiring_extract(bomm_wiring_t* ptr, const char* string) {
    bomm_message_t* message = bomm_message_init(string);
    if (!message) {
        return false;
    }

    memset(ptr, BOMM_ALPHABET_SIZE, sizeof(bomm_wiring_t));

    unsigned char char_index;
    bool error = message->length != BOMM_ALPHABET_SIZE;
    unsigned int position = 0;
    while (!error && position < message->length) {
        char_index = message->letters[position];
        if (ptr->rev[char_index] == BOMM_ALPHABET_SIZE) {
            ptr->map[position] = char_index;
            ptr->rev[char_index] = position;
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

void bomm_wiring_serialize(char* str, size_t size, bomm_wiring_t* wiring) {
    unsigned int i = 0;
    while (i < BOMM_ALPHABET_SIZE && i < size - 1) {
        str[i] = wiring->map[i] + 97;
        i++;
    }
    str[i] = '\0';
}
