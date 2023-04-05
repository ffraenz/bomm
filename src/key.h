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
#include "hold.h"

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
    bomm_letter_t plugboard[BOMM_ALPHABET_SIZE];
} bomm_key_t;

/**
 * Identity or empty plugboard having all plugs self-steckered.
 */
extern const bomm_letter_t bomm_key_plugboard_identity[];

/**
 * Serialize the given key to a string.
 */
void bomm_key_serialize(char* str, size_t size, bomm_key_t* key);

void bomm_key_serialize_wheel_order(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_ring_settings(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_start_positions(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_plugboard(char* str, size_t size, bomm_key_t* key);

void bomm_key_hold_print(bomm_hold_t* hold);

#endif /* key_h */
