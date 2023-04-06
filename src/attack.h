//
//  attack.h
//  Bomm
//
//  Created by Fränz Friederes on 20/03/2023.
//

#ifndef attack_h
#define attack_h

#include <stdio.h>
#include <time.h>
#include <math.h>
#include "enigma.h"
#include "key.h"
#include "measure.h"

/**
 * Exhaust the given key space.
 */
void bomm_attack_key_space(
    bomm_key_space_t* key_space,
    bomm_message_t* ciphertext
);

/**
 * Attack the plugboard using the given scrambler and ciphertext.
 */
float bomm_attack_plugboard(
    bomm_letter_t* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext,
    bomm_ngram_map_t* ngram_map
);

#endif /* attack_h */
