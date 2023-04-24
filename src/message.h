//
//  message.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef message_h
#define message_h

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BOMM_ALPHABET "abcdefghijklmnopqrstuvwxyz"
#define BOMM_ALPHABET_SIZE 26

/**
 * Type representing an Enigma message letter. Its values range from `0`,
 * representing the first letter of the alphabet, and `BOMM_ALPHABET_SIZE - 1`,
 * representing the last one.
 */
typedef unsigned char bomm_letter_t;

/**
 * Variable-size struct storing an Enigma message. It consists of an arbitrary
 * number of letter indices.
 */
typedef struct _bomm_message {
    /**
     * Total number of letters in message
     */
    unsigned int length;

    /**
     * Letters
     */
    bomm_letter_t letters[];
} bomm_message_t;

/**
 * Allocate a message for the given length.
 */
bomm_message_t* bomm_message_init_with_length(unsigned int length);

/**
 * Allocate and load a message from the given C-string.
 */
bomm_message_t* bomm_message_init(const char* string);

/**
 * Calculate the serialize string size for the given message.
 */
static inline size_t bomm_message_serialize_size(bomm_message_t* message) {
    return message->length + 1;
}

/**
 * Calculate the number of bytes required to store a message struct of the
 * given length.
 */
static inline size_t bomm_message_size_for_length(unsigned int length) {
    return sizeof(bomm_message_t) + length * sizeof(bomm_letter_t);
}

/**
 * Turn the given ASCII code point to a message letter.
 * If no mapping exists, 255 is returned.
 */
static inline unsigned char bomm_message_letter_from_ascii(char code_point) {
    if (code_point >= 97 && code_point <= 122) {
        // Read lowercase ASCII a-z
        return code_point - 97;
    } else if (code_point >= 65 && code_point <= 90) {
        // Read uppercase ASCII A-Z
        return code_point - 65;
    }
    return 255;
}

/**
 * Turn the given message letter to an ASCII code point.
 * Uses the lowercase alphabet.
 */
static inline char bomm_message_letter_to_ascii(unsigned char letter) {
    return letter + 97;
}

/**
 * Export the given message to a string.
 */
static inline void bomm_message_stringify(
    char* str,
    size_t size,
    bomm_message_t* message
) {
    size_t actual_size = message->length < size ? message->length : size - 1;
    for (unsigned int i = 0; i < actual_size; i++) {
        str[i] = bomm_message_letter_to_ascii(message->letters[i]);
    }
    str[actual_size] = '\0';
}

#endif /* message_h */
