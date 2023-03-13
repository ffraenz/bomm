//
//  message.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "message.h"

bomm_message_t* bomm_alloc_message_with_length(unsigned int length) {
    size_t message_size =
        sizeof(bomm_message_t) + length * sizeof(bomm_letter_t);
    bomm_message_t* message = (bomm_message_t*) malloc(message_size);
    if (!message) {
        return NULL;
    }
    message->length = length;
    return message;
}

bomm_message_t* bomm_alloc_message(char* string) {
    size_t length = strlen(string);
    size_t letter_size = sizeof(bomm_letter_t);
    size_t message_size = sizeof(bomm_message_t) + length * letter_size;
    bomm_message_t* message = (bomm_message_t*) malloc(message_size);

    if (!message) {
        return NULL;
    }

    message->length = 0;

    char letter = 0;
    for (int i = 0; i < length; i++) {
        letter = string[i];
        if (letter >= 97 && letter <= 122) {
            // Read ASCII a-z
            message->letters[message->length++] = letter - 97;
        } else if (letter >= 65 && letter <= 90) {
            // Read ASCII A-Z
            message->letters[message->length++] = letter - 65;
        } else {
            // Ignore other letters
        }
    }

    size_t actual_message_size =
        sizeof(bomm_message_t) + message->length * letter_size;
    if (actual_message_size < message_size) {
        message = realloc(message, actual_message_size);
    }
    return message;
}

char* bomm_describe_message(bomm_message_t* message) {
    char* string = malloc(sizeof(char) * message->length);
    for (int i = 0; i < message->length; i++) {
        string[i] = message->letters[i] + 97;
    }
    return string;
}
