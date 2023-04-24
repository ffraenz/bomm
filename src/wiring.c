//
//  wiring.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "wiring.h"

bomm_wiring_t* bomm_wiring_init(bomm_wiring_t* wiring, const char* string) {
    bomm_message_t* message;
    if ((message = bomm_message_init(string)) == NULL) {
        return NULL;
    }

    bool owning = wiring == NULL;
    if (owning) {
        if ((wiring = malloc(sizeof(bomm_wiring_t))) == NULL) {
            free(message);
            return NULL;
        }
    }

    memset(wiring, BOMM_ALPHABET_SIZE, sizeof(bomm_wiring_t));

    unsigned char char_index;
    bool error = message->length != BOMM_ALPHABET_SIZE;
    unsigned int position = 0;
    while (!error && position < message->length) {
        char_index = message->letters[position];
        if (wiring->rev[char_index] == BOMM_ALPHABET_SIZE) {
            wiring->map[position] = char_index;
            wiring->rev[char_index] = position;
            position++;
        } else {
            error = true;
        }
    }

    free(message);

    if (error) {
        fprintf(
            stderr,
            "Encountered unexpected wiring string '%s' at position %d\n",
            string,
            position
        );
        if (owning) {
            free(wiring);
        }
        return NULL;
    }

    return wiring;
}

void bomm_wiring_stringify(char* str, size_t size, bomm_wiring_t* wiring) {
    unsigned int i = 0;
    while (i < BOMM_ALPHABET_SIZE && i < size - 1) {
        str[i] = wiring->map[i] + 97;
        i++;
    }
    str[i] = '\0';
}
