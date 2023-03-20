//
//  key.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef key_h
#define key_h

#include <math.h>
#include <stdio.h>
#include "model.h"

/**
 * Location in the Enigma model key space.
 * Struct is optimized to being used in-place.
 */
typedef struct _bomm_key {
    /**
     * Pointer to the model
     */
    bomm_model_t* model;
    
    /**
     * Wheel order (Walzenlage): Chosen wheel index for each slot
     */
    unsigned int wheels[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Ring setting (Ringstellung): Ring position of the wheel in each slot
     */
    unsigned int rings[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Start position (Walzenstellung): Start position of the wheel in each slot
     */
    unsigned int positions[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Plugboard setting (Steckerverbindungen) to be used
     */
    bomm_wiring_t plugboard;
} bomm_key_t;

/**
 * Location in the Enigma model key space scored by a measure.
 */
typedef struct _bomm_key_scored {
    /**
     * Key
     */
    bomm_key_t key;
    
    /**
     * Key score (e.g. Index of coincidence)
     */
    float score;
} bomm_key_scored_t;

/**
 * Variable-size struct storing a list of keys sorted by their score measure in
 * descending order.
 */
typedef struct _bomm_key_leaderboard {
    /**
     * Current number of entries
     */
    unsigned int count;
    
    /**
     * Maximum number of entries to be maintained in the leaderboard
     */
    unsigned int size;
    
    /**
     * Leaderboard entries
     */
    bomm_key_scored_t entries[];
} bomm_key_leaderboard_t;

/**
 * Serialize the given key to a string.
 */
void bomm_key_serialize(char* str, size_t size, bomm_key_t* key);

void bomm_key_serialize_wheel_order(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_ring_settings(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_start_positions(char* str, size_t size, bomm_key_t* key);

/**
 * Allocates a key leaderboard in memory for the given size.
 */
void* bomm_key_leaderboard_alloc(unsigned int size);

/**
 * Add the given scored key to the leaderboard.
 * @return New minimum score necessary to enter the leaderboard
 */
float bomm_key_leaderboard_add(bomm_key_leaderboard_t* leaderboard, bomm_key_t* key, float score);

/**
 * Print the given leaderboard to the console.
 */
void bomm_key_leaderboard_print(bomm_key_leaderboard_t* leaderboard);

#endif /* key_h */
