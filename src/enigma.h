//
//  enigma.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef enigma_h
#define enigma_h

#include <math.h>
#include <stdio.h>
#include "key.h"

#define bomm_map_position_ring(x, p, r) (x + p - r + BOMM_ALPHABET_SIZE) % BOMM_ALPHABET_SIZE
#define bomm_rev_position_ring(x, p, r) (x - p + r + BOMM_ALPHABET_SIZE) % BOMM_ALPHABET_SIZE

/**
 * Exhaust the relevant key space of the given model excl. the plugboard.
 */
void bomm_model_attack_phase_1(bomm_model_t* model, bomm_message_t* ciphertext);

/**
 * Simulate the Enigma on the given original message and key.
 * Original and result message structs are assumed to be of same length.
 */
void bomm_model_encrypt(bomm_message_t* original, bomm_key_t* key, bomm_message_t* result);

#endif /* enigma_h */
