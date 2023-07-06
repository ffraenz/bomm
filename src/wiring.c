//
//  wiring.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "wiring.h"
#include "utility.h"

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

bomm_plugboard_t* bomm_plugboard_init_identity(bomm_plugboard_t* plugboard) {
    if (!plugboard && !(plugboard = malloc(sizeof(bomm_plugboard_t)))) {
        return NULL;
    }
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        plugboard->map[i] = i;
    }
    return plugboard;
}

bomm_plugboard_t* bomm_plugboard_init(
    bomm_plugboard_t* plugboard,
    const char* string
) {
    bomm_message_t* message;
    if (!(message = bomm_message_init(string))) {
        return NULL;
    }

    if (message->length % 2 != 0) {
        // A plugboard string having an odd number of letters (whitespaces
        // removed) is invalid
        free(message);
        return NULL;
    }

    bool owning = plugboard == NULL;
    plugboard = bomm_plugboard_init_identity(plugboard);
    if (!plugboard) {
        free(message);
        return NULL;
    }

    bomm_letter_t a, b;
    bool valid = true;
    for (unsigned int i = 0; i < message->length; i += 2) {
        a = message->letters[i];
        b = message->letters[i + 1];
        // Only self-steckered letters may be swapped to form steckered pairs
        valid = valid && plugboard->map[a] == a && plugboard->map[b] == b;
        bomm_swap(&plugboard->map[a], &plugboard->map[b]);
    }
    free(message);

    if (!valid) {
        if (owning) {
            free(plugboard);
        }
        return NULL;
    }

    return plugboard;
}

void bomm_wiring_stringify(
    char* str,
    size_t size,
    const bomm_wiring_t* wiring
) {
    unsigned int i = 0;
    while (i < BOMM_ALPHABET_SIZE && i < size - 1) {
        str[i] = wiring->map[i] + 97;
        i++;
    }
    str[i] = '\0';
}

void bomm_plugboard_stringify(
    char* str,
    size_t size,
    const bomm_plugboard_t* plugboard
) {
    unsigned long j = 0;

    if (!bomm_plugboard_validate(plugboard)) {
        bomm_strncpy(str, "(invalid)", size);
        j = strlen(str);
    }

    bomm_lettermask_t used_letters = BOMM_LETTERMASK_NONE;

    unsigned int i = 0;
    while (i < BOMM_ALPHABET_SIZE && j + 3 < size - 1) {
        if (plugboard->map[i] != i && !bomm_lettermask_has(&used_letters, i)) {
            if (j > 0) {
                str[j++] = ' ';
            }
            bomm_lettermask_set(&used_letters, plugboard->map[i]);
            str[j++] = bomm_message_letter_to_ascii(i);
            str[j++] = bomm_message_letter_to_ascii(plugboard->map[i]);
        }
        i++;
    }

    if (size > 0) {
        str[j] = '\0';
    }
}
