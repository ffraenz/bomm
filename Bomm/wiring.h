//
//  wiring.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef wiring_h
#define wiring_h

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "message.h"

#define BOMM_ALPHABET "abcdefghijklmnopqrstuvwxyz"
#define BOMM_ALPHABET_SIZE 26

typedef struct _bomm_wiring {
    unsigned char forward_map[BOMM_ALPHABET_SIZE];
    unsigned char backward_map[BOMM_ALPHABET_SIZE];
} bomm_wiring_t;

/**
 * Load a wiring from the given string.
 */
bool bomm_load_wiring(bomm_wiring_t* ptr, char* string);

/**
 * Export the given enigma wiring to a string.
 */
char* bomm_describe_wiring(bomm_wiring_t* wiring);

#endif /* wiring_h */
