//
//  lettermask.c
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#include "lettermask.h"

void bomm_lettermask_extract(bomm_lettermask_t* ptr, char* string) {
    // TODO: Implement parser for lettermask notation
    bomm_message_t *message = bomm_alloc_message(string);
    if (!message) {
        fprintf(stderr, "Out of memory while loading lettermask\n");
        return;
    }
    
    *ptr = 0;
    for (unsigned int i = 0; i < message->length; i++) {
        // Set bits at letter positions
        *ptr = *ptr | (1 << message->letters[i]);
    }

    free(message);
}

char* bomm_lettermask_serialize(bomm_lettermask_t* lettermask) {
    char* string = (char*) malloc(sizeof(char) * (BOMM_ALPHABET_SIZE + 1));
    if (!string) {
        fprintf(stderr, "Out of memory while describing lettermask\n");
        return "";
    }
    
    int j = 0;
    for (bomm_letter_t letter = 0; letter < BOMM_ALPHABET_SIZE; letter++) {
        if (bomm_lettermask_has(*lettermask, letter)) {
            string[j++] = BOMM_ALPHABET[letter];
        }
    }
    
    string[j++] = '\0';
    string = realloc(string, sizeof(char) * j);
    return string;
}
