//
//  lettermask.c
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#include "lettermask.h"

bomm_lettermask_t* bomm_lettermask_from_string(
    bomm_lettermask_t* lettermask,
    const char* string
) {
    // Handle lettermask wildcard
    if (strcmp(string, "*") == 0) {
        *lettermask = BOMM_LETTERMASK_ALL;
        return lettermask;
    }

    // TODO: Implement parser for lettermask notation
    bomm_message_t *message = bomm_message_init(string);
    if (!message) {
        fprintf(stderr, "Out of memory while loading lettermask\n");
        return NULL;
    }

    *lettermask = 0;
    for (unsigned int i = 0; i < message->length; i++) {
        // Set bits at letter positions
        *lettermask = *lettermask | (1 << message->letters[i]);
    }

    free(message);
    return lettermask;
}

void bomm_lettermask_stringify(
    char* str,
    size_t size,
    const bomm_lettermask_t* lettermask
) {
    unsigned int i = 0;
    unsigned int letter = 0;
    while (letter < BOMM_ALPHABET_SIZE && i < size - 1) {
        if (bomm_lettermask_has(lettermask, letter)) {
            str[i++] = BOMM_ALPHABET[letter];
        }
        letter++;
    }
    str[i] = '\0';
}
