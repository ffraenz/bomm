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

/**
 * Struct representing a slice of an attack on Enigma ciphertext that can be
 * executed in a single thread.
 */
typedef struct _bomm_attack_slice {
    /**
     * Pointer to the attack
     */
    bomm_attack_t* attack;
    
    /**
     * Number uniquely identifying the slice within an attack
     */
    unsigned int id;
    
    /**
     * Pointer to object identifying the thread that is executing this slice
     */
    pthread_t thread;
    
    /**
     * Target ciphertext
     */
    bomm_message_t* ciphertext;
    
    /**
     * Target key space
     */
    bomm_key_space_t* key_space;
} bomm_attack_slice_t;

/**
 * Struct representing a distributed attack on Enigma ciphertext
 */
typedef struct _bomm_attack {
    /**
     * Shared hold results are reported in
     */
    bomm_hold_t* hold;
    
    /**
     * Number of slices the attack is divided into
     */
    unsigned int slice_count;
    
    /**
     * Attack slices
     */
    bomm_attack_slice_t slices[];
} bomm_attack_t;

/**
 * Initialize an attack.
 */
bomm_attack_t* bomm_attack_init(unsigned int slice_count);

/**
 * Destroy an attack.
 */
void bomm_attack_destroy(bomm_attack_t* attack);

/**
 * Run the given attack slice in a thread.
 */
void* bomm_attack_slice_run(void* arg);

/**
 * Exhaust the given key space.
 */
void bomm_attack_key_space(bomm_attack_slice_t* attack_slice);

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
