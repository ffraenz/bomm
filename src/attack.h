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
 * Exhaust the relevant key space of the given model excl. the plugboard.
 */
void bomm_attack_phase_1(bomm_model_t* model, bomm_message_t* ciphertext);

#endif /* attack_h */
