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

#define BOMM_WHEEL_NAME_MAX_LENGTH 16
#define BOMM_MODEL_MAX_SLOT_COUNT 6
#define BOMM_MODEL_MAX_WHEEL_SET_SIZE 15

/**
 * Struct representing a wheel without its state (e.g. ring setting, position)
 */
typedef struct _bomm_wheel {
    /**
     * Wheel name (NULL-terminated string of max. 15 chars)
     */
    char name[BOMM_WHEEL_NAME_MAX_LENGTH];
    
    /**
     * Wheel wiring
     */
    bomm_wiring_t wiring;
    
    /**
     * Wheel turnovers
     */
    bomm_lettermask_t turnovers;
} bomm_wheel_t;

typedef unsigned char bomm_wheel_index_t;
typedef unsigned char bomm_slot_index_t;

/**
 * Enigma wheel rotation mechanism options
 */
typedef enum {
    BOMM_MECHANISM_STEPPING,
    BOMM_MECHANISM_COG_WHEEL
} bomm_mechanism_t;

/**
 * Variable-size struct representing a model, from which the set of all possible
 * keys (search space) can be derived.
 */
typedef struct _bomm_model {
    /**
     * Number of wheel slots
     *
     * Slot at index 0 points to the left-most wheel.
     *
     * Slots include:
     * - Reflector (Umkehrwalze)
     * - Regular wheel from left to right
     * - Entry wheel (Eintrittswalze)
     */
    unsigned int slot_count;
    
    /**
     * Set of wheels per slot. Indices reference entries in `wheels`.
     * Index `255` signifies the end of the wheel set.
     */
    bomm_wheel_index_t wheel_sets[BOMM_MODEL_MAX_SLOT_COUNT]
        [BOMM_MODEL_MAX_WHEEL_SET_SIZE + 1];
    
    /**
     * Whether wheel is rotating for each slot.
     */
    bool rotating_slots[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Possible ring settings for each slot.
     * Must not be empty (equal to 0).
     */
    bomm_lettermask_t ring_masks[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Possible start positions for each slot.
     * Must not be empty (equal to 0).
     */
    bomm_lettermask_t position_masks[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Rotation mechanism:
     * - Stepping: Assumes the second last slot to be the fast rotating slot.
     * - Cog wheel
     */
    bomm_mechanism_t mechanism;
    
    /**
     * Number of wheels that can be referenced by other fields
     */
    unsigned int wheel_count;
    
    /**
     * Wheels that can be referenced by other fields
     */
    bomm_wheel_t wheels[];
} bomm_model_t;

/**
 * Load a wheel from the given args.
 */
void bomm_load_wheel_args(
    bomm_wheel_t* ptr,
    char* name,
    char* wiring_string,
    char* turnovers_string
);

/**
 * Struct size for a model with the given number of wheels.
 */
static inline size_t bomm_model_size(unsigned char wheel_count) {
    return
        sizeof(bomm_model_t) +
        sizeof(bomm_wheel_t) * wheel_count;
}

/**
 * Allocate space for a model in memory.
 */
bomm_model_t* bomm_model_alloc(unsigned char wheel_count);

/**
 * Allocate and load the Enigma I model in memory.
 */
bomm_model_t* bomm_model_alloc_enigma_i(void);

#endif /* model_h */
