//
//  message.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "message.h"

bomm_message_t* bomm_alloc_message_with_length(unsigned int length) {
    size_t message_size = bomm_message_size_for_length(length);
    bomm_message_t* message = (bomm_message_t*) malloc(message_size);
    if (!message) {
        return NULL;
    }
    message->length = length;
    // Zero frequency array
    memset(&message->frequency, 0, sizeof(unsigned int) * BOMM_ALPHABET_SIZE);
    return message;
}

bomm_message_t* bomm_alloc_message(char* string) {
    size_t length = strlen(string);
    size_t message_size = bomm_message_size_for_length((unsigned int) length);
    bomm_message_t* message = (bomm_message_t*) malloc(message_size);

    if (!message) {
        return NULL;
    }

    // Reset message length and frequency
    message->length = 0;
    memset(&message->frequency, 0, sizeof(unsigned int) * BOMM_ALPHABET_SIZE);

    unsigned char letter;
    for (unsigned int i = 0; i < length; i++) {
        if ((letter = bomm_message_letter_from_ascii(string[i])) != 255) {
            message->letters[message->length++] = letter;
            message->frequency[letter]++;
        }
    }

    size_t actual_message_size = bomm_message_size_for_length(message->length);
    if (actual_message_size < message_size) {
        message = realloc(message, actual_message_size);
    }
    return message;
}

void bomm_message_serialize(char* str, size_t size, bomm_message_t* message) {
    size_t actual_size = message->length < size ? message->length : size - 1;
    for (unsigned int i = 0; i < actual_size; i++) {
        str[i] = bomm_message_letter_to_ascii(message->letters[i]);
    }
    str[actual_size] = '\0';
}
