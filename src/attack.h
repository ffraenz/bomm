//
//  attack.h
//  Bomm
//
//  Created by Fränz Friederes on 20/03/2023.
//

#ifndef attack_h
#define attack_h

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

#endif /* attack_h */
