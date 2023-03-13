//
//  turnovers.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "turnovers.h"

void bomm_load_turnovers(bomm_turnovers_t* ptr, char* turnovers_string) {
    bomm_message_t *message = bomm_alloc_message(turnovers_string);
    if (!message) {
        fprintf(stderr, "Out of memory while loading turnovers\n");
        return;
    }
    
    bomm_turnovers_t turnovers = 0;
    for (unsigned int i = 0; i < message->length; i++) {
        // Set bits at letter positions
        turnovers = turnovers | (1 << message->letters[i]);
    }

    free(message);
    *ptr = turnovers;
}

char* bomm_describe_turnovers(bomm_turnovers_t* turnovers) {
    char* string = (char*) malloc(sizeof(char) * (BOMM_ALPHABET_SIZE + 1));
    if (!string) {
        fprintf(stderr, "Out of memory while describing turnovers\n");
        return "";
    }
    
    int j = 0;
    for (bomm_letter_t letter = 0; letter < BOMM_ALPHABET_SIZE; letter++) {
        if ((*turnovers >> letter) & 0x1) {
            string[j++] = BOMM_ALPHABET[letter];
        }
    }
    
    string[j++] = '\0';
    string = realloc(string, sizeof(char) * j);
    return string;
}
