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

/**
 * A single letter index between 0 (for 'A') and 25 (for 'Z')
 */
typedef unsigned char bomm_letter_t;

/**
 * Struct representing a message, consisting of an arbitrary number of
 * letter indices.
 */
typedef struct _bomm_message {
    unsigned int length;
    bomm_letter_t letters[];
} bomm_message_t;

/**
 * Allocate a message for the given length.
 */
bomm_message_t* bomm_alloc_message_with_length(unsigned int length);

/**
 * Allocate and load a message from the given C-string.
 */
bomm_message_t* bomm_alloc_message(char* string);

char* bomm_describe_message(bomm_message_t* message);

#endif /* message_h */
