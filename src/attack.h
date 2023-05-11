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
#include "wiring.h"

typedef struct _bomm_attack bomm_attack_t;

#include "query.h"

/**
 * Execute the given attack.
 * Function that can be used from `pthread_create`.
 */
void* bomm_attack_thread(void* arg);

/**
 * Exhaust the given key space.
 */
void bomm_attack_key_space(bomm_attack_t* attack);

/**
 * Attack the plugboard using the given scrambler and ciphertext.
 */
float bomm_attack_plugboard(
    unsigned int* plugboard,
    const unsigned int* plug_order,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
);

typedef enum {
    BOMM_ATTACK_PLUGBOARD_FIRST_IMPROVEMENT,
    BOMM_ATTACK_PLUGBOARD_ENIGMA_SUITE,
    BOMM_ATTACK_PLUGBOARD_BEST_IMPROVEMENT
} bomm_attack_plugboard_strategy_t;

/**
 * Attack the plugboard using the hill climb strategy.
 */
float bomm_attack_plugboard_hill_climb(
    bomm_attack_plugboard_strategy_t strategy,
    const bomm_measure_t* measures,
    unsigned int* plugboard,
    const unsigned int* plug_order,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
);

/**
 * Try reswapping each self-steckered pair. Steepest ascent.
 */
float bomm_attack_plugboard_enigma_suite_reswapping(
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
);

#endif /* attack_h */
