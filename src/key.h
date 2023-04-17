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
#include "hold.h"
#include "wheel.h"
#include "wiring.h"
#include "lettermask.h"
#include "utility.h"

#define BOMM_MAX_SLOT_COUNT 6
#define BOMM_MAX_WHEEL_SET_SIZE 15

/**
 * Rotation mechanism options
 */
typedef enum {
    /**
     * No mechanism; The position does not change throughout the message.
     */
    BOMM_MECHANISM_NONE,
    
    /**
     * Assumes exactly 5 slots with one entry wheel, 3 rotating wheels, and
     * one reflector. Implements the double stepping anomaly.
     */
    BOMM_MECHANISM_STEPPING,
    
    /**
     * Odometer mechanism. Works with any number of slots.
     */
    BOMM_MECHANISM_ODOMETER
} bomm_mechanism_t;

/**
 * Struct representing a key space, from which a set of keys can be derived.
 */
typedef struct _bomm_key_space {
    /**
     * Rotation mechanism
     */
    bomm_mechanism_t mechanism;
    
    /**
     * Number of wheel slots; The first slot represents the reflector, the last
     * slot represents the entry wheel.
     */
    unsigned int slot_count;
    
    /**
     * Set of wheels per slot
     */
    bomm_wheel_t* wheel_sets[BOMM_MAX_SLOT_COUNT]
        [BOMM_MAX_WHEEL_SET_SIZE + 1];
    
    /**
     * Whether wheel is rotating for each slot
     */
    bool rotating_slots[BOMM_MAX_SLOT_COUNT];
    
    /**
     * Possible ring settings for each slot.
     * Must not be empty (equal to 0).
     */
    bomm_lettermask_t ring_masks[BOMM_MAX_SLOT_COUNT];
    
    /**
     * Possible start positions for each slot.
     * Must not be empty (equal to 0).
     */
    bomm_lettermask_t position_masks[BOMM_MAX_SLOT_COUNT];
} bomm_key_space_t;

/**
 * Struct describing a single location in a search space.
 * Struct is optimized to being used in-place.
 * TODO: Optimize key storage by separating key and state structs.
 * TODO: Make struct variable size depending on the slot count.
 */
typedef struct _bomm_key {
    /**
     * Rotation mechanism
     */
    bomm_mechanism_t mechanism;
    
    /**
     * Number of wheel slots; The first slot represents the reflector, the last
     * slot represents the entry wheel.
     */
    unsigned int slot_count;
    
    /**
     * Wheel instances matching the wheels of the key
     */
    bomm_wheel_t wheels[BOMM_MAX_SLOT_COUNT];
    
    /**
     * Whether wheel is rotating for each slot
     */
    bool rotating_slots[BOMM_MAX_SLOT_COUNT];
    
    /**
     * Ring setting (Ringstellung): Ring position of the wheel in each slot
     */
    unsigned int rings[BOMM_MAX_SLOT_COUNT];
    
    /**
     * Start position (Walzenstellung): Start position of the wheel in each slot
     */
    unsigned int positions[BOMM_MAX_SLOT_COUNT];
    
    /**
     * Plugboard setting (Steckerverbindungen) to be used
     */
    unsigned int plugboard[BOMM_ALPHABET_SIZE];
} bomm_key_t;

/**
 * Identity or empty plugboard having all plugs self-steckered.
 */
extern const unsigned int bomm_key_plugboard_identity[BOMM_ALPHABET_SIZE];

/**
 * Extract a mechanism value from the given string.
 */
bomm_mechanism_t bomm_key_mechanism_extract(const char* mechanism_string);

/**
 * Initialize a key space with the given mechanism and slot count.
 */
bomm_key_space_t* bomm_key_space_init(
    bomm_key_space_t* key_space,
    bomm_mechanism_t mechanism,
    unsigned int slot_count
);

/**
 * Extract a key space from the given JSON object.
 * @param key_space Existing key space or NULL to allocate a new one
 * @param key_space_json JSON key space object
 * @param wheels Pointer to an array of wheels for lookup
 * @param wheel_count Number of elements in `wheels`
 */
bomm_key_space_t* bomm_key_space_extract_json(
    bomm_key_space_t* key_space,
    json_t* key_space_json,
    bomm_wheel_t wheels[],
    unsigned int wheel_count
);

/**
 * Initialize a key space for the Enigma I model.
 */
bomm_key_space_t* bomm_key_space_enigma_i_init(void);

/**
 * Destroy the given key space.
 */
void bomm_key_space_destroy(bomm_key_space_t* key_space);

/**
 * Initialize a key from the given key space. Wheels are not initialized.
 * @param key Pointer to an existing key in memory or null, if a new location
 * should be allocated and returned.
 */
bomm_key_t* bomm_key_init(bomm_key_t* key, bomm_key_space_t* key_space);

/**
 * Serialize the given key to a string.
 */
void bomm_key_serialize(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_wheel_order(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_ring_settings(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_start_positions(char* str, size_t size, bomm_key_t* key);
void bomm_key_serialize_plugboard(char* str, size_t size, bomm_key_t* key);

/**
 * Print the given key hold to the console.
 */
void bomm_key_hold_print(bomm_hold_t* hold);

#endif /* key_h */
