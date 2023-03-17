//
//  model.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef model_h
#define model_h

#include <stdio.h>
#include "wiring.h"
#include "lettermask.h"

#define BOMM_ROTOR_NAME_MAX_LENGTH 16
#define BOMM_MODEL_MAX_SLOT_COUNT 6
#define BOMM_MODEL_MAX_ROTOR_COUNT_PER_SLOT 15

/**
 * Struct representing a static rotor specification
 */
typedef struct _bomm_rotor_spec {
    /**
     * Rotor name (NULL-terminated string of max. 15 chars)
     */
    char name[BOMM_ROTOR_NAME_MAX_LENGTH];
    
    /**
     * Rotor wiring
     */
    bomm_wiring_t wiring;
    
    /**
     * Rotor turnovers
     */
    bomm_lettermask_t turnovers;
    
    /**
     * Whether the rotor is rotating
     */
    bool rotating;
} bomm_rotor_spec_t;

typedef unsigned char bomm_rotor_index_t;
typedef unsigned char bomm_slot_index_t;

typedef enum {
    BOMM_ENIGMA_STEPPING,
    BOMM_COG_WHEEL
} bomm_rotation_mechanism_t;

/**
 * Struct representing a model, from which the set of all possible keys
 * (search space) can be derived.
 */
typedef struct _bomm_model {
    /**
     * Rotation mechanism
     */
    bomm_rotation_mechanism_t rotation_mechanism;
    
    /**
     * Number of slots
     *
     * Slots include (from left to right):
     * - Reflector (Umkehrwalze)
     * - Regular rotors from left to right
     * - Entry rotor (Eintrittswalze)
     */
    unsigned char slot_count;
    
    /**
     * The slot index of the fast rotating rotor
     */
    bomm_slot_index_t fast_rotating_slot;
    
    /**
     * Possible rotor indices per slot.
     * Indices reference entries in `rotors`.
     * Index `255` signifies the end of the rotors for this slot is reached.
     */
    bomm_rotor_index_t slot_rotor_indices
        [BOMM_MODEL_MAX_SLOT_COUNT]
        [BOMM_MODEL_MAX_ROTOR_COUNT_PER_SLOT + 1];
    
    /**
     * Possible ring settings for each slot.
     * Must not be empty (equal to 0).
     */
    bomm_lettermask_t slot_ring_mask[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Possible start positions for each slot.
     * Must not be empty (equal to 0).
     */
    bomm_lettermask_t slot_position_mask[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Number of available rotors
     */
    unsigned char rotor_count;
    
    /**
     * Available rotors
     */
    bomm_rotor_spec_t rotors[];
} bomm_model_t;

/**
 * Load a rotor spec from the given args.
 */
void bomm_load_rotor_spec_args(
    bomm_rotor_spec_t* ptr,
    char* name,
    char* wiring_string,
    char* turnovers_string,
    bool rotating
);

/**
 * Allocate space for a model in memory.
 */
bomm_model_t* bomm_alloc_model(unsigned char rotor_count);

/**
 * Allocate and load the Enigma I model in memory.
 */
bomm_model_t* bomm_alloc_model_enigma_i(void);

#endif /* model_h */