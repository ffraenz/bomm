//
//  enigma.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef enigma_h
#define enigma_h

#include <stdio.h>
#include "key.h"

#define bomm_map_position_ring(x, p, r) (x + p - r + BOMM_ALPHABET_SIZE) % BOMM_ALPHABET_SIZE
#define bomm_rev_position_ring(x, p, r) (x - p + r + BOMM_ALPHABET_SIZE) % BOMM_ALPHABET_SIZE

/**
 * Simulate the Enigma for the given message.
 */
bomm_message_t* bomm_model_encrypt(bomm_message_t* message, bomm_key_t* key);

#endif /* enigma_h */
