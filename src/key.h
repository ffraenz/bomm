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

typedef struct _bomm_key_iterator {
    /**
     * Key space iterator was created for
     */
    bomm_key_space_t* key_space;

    /**
     * Current key
     */
    bomm_key_t key;

    /**
     * Wheel indices
     */
    unsigned int wheel_indices[BOMM_MAX_SLOT_COUNT];

    /**
     * Shifting ring mask per slot
     */
    bomm_lettermask_t ring_masks[BOMM_MAX_SLOT_COUNT];

    /**
     * Shifting position mask per slot
     */
    bomm_lettermask_t position_masks[BOMM_MAX_SLOT_COUNT];
} bomm_key_iterator_t;

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

static inline void bomm_key_iterator_lettermask_init(
    unsigned char slot_count,
    unsigned int* positions,
    bomm_lettermask_t* shifting_masks
) {
    memset(positions, 0, BOMM_MAX_SLOT_COUNT * sizeof(bomm_letter_t));
    for (int slot = 0; slot < slot_count; slot++) {
        while ((shifting_masks[slot] & 0x1) == 0) {
            shifting_masks[slot] = shifting_masks[slot] >> 1;
            positions[slot]++;
        }
    }
}

static inline bool bomm_key_iterator_lettermask_next(
    unsigned char slot_count,
    unsigned int* positions,
    bomm_lettermask_t* shifting_masks
) {
    bool carry = true;
    int slot = slot_count;
    while (carry && --slot >= 0) {
        // Optimization: If there's only one bit set on the mask we can skip the
        // loops and literally carry on to the next slot
        if (shifting_masks[slot] != BOMM_LETTERMASK_FIRST) {
            // Increment position as well as shift and wrap shifting mask
            positions[slot]++;
            shifting_masks[slot] = BOMM_LETTERMASK_LAST | (shifting_masks[slot] >> 1);

            // Increment position and shift mask until the next valid position
            // is found
            while ((shifting_masks[slot] & 0x1) == 0) {
                positions[slot]++;
                shifting_masks[slot] = shifting_masks[slot] >> 1;
            }

            // If position exceeded the alphabet size, we have a carry
            if ((carry = positions[slot] >= BOMM_ALPHABET_SIZE)) {
                positions[slot] -= BOMM_ALPHABET_SIZE;
            }
        }
    }
    return carry;
}

/**
 * Validate the iterator wheel order
 */
static inline bool bomm_key_iterator_validate_wheel_order(
    bomm_key_iterator_t* iterator
) {
    int slot_count = iterator->key_space->slot_count;
    for (int i = 0; i < slot_count; i++) {
        for (int j = i + 1; j < slot_count; j++) {
            if (
                iterator->key_space->wheel_sets[i][iterator->wheel_indices[i]] ==
                iterator->key_space->wheel_sets[j][iterator->wheel_indices[j]]
            ) {
                return false;
            }
        }
    }
    return true;
}

/**
 * Find the next relevant wheel order
 * @param increment Leaves a valid wheel order intact if set to false
 */
static inline bool bomm_key_iterator_wheel_order_next(
    bomm_key_iterator_t* iterator,
    bool increment
) {
    bool carry_out = false;
    while (!bomm_key_iterator_validate_wheel_order(iterator) || increment) {
        increment = false;
        bool carry = true;
        int slot = iterator->key_space->slot_count;
        while (carry && --slot >= 0) {
            iterator->wheel_indices[slot]++;
            if ((carry = (iterator->key_space->wheel_sets[slot][iterator->wheel_indices[slot]] == NULL))) {
                iterator->wheel_indices[slot] = 0;
            }
            memcpy(
                &iterator->key.wheels[slot],
                iterator->key_space->wheel_sets[slot][iterator->wheel_indices[slot]],
                sizeof(bomm_wheel_t)
            );
        }
        if (carry) {
            carry_out = true;
        }
    }
    return carry_out;
}

/**
 * Initialize an iterator for the given key space.
 */
static inline bomm_key_iterator_t* bomm_key_iterator_init(
    bomm_key_iterator_t* iterator,
    bomm_key_space_t* key_space
) {
    if (iterator == NULL) {
        if ((iterator = malloc(sizeof(bomm_key_iterator_t))) == NULL) {
            return NULL;
        }
    }

    // Reference key space
    iterator->key_space = key_space;
    unsigned int slot_count = key_space->slot_count;

    // Init key
    bomm_key_init(&iterator->key, key_space);

    // Find initial wheel indices
    memset(&iterator->wheel_indices, 0, sizeof(unsigned int) * slot_count);
    bomm_key_iterator_wheel_order_next(iterator, false);

    // Find initial rings
    memcpy(&iterator->ring_masks, key_space->ring_masks, sizeof(bomm_lettermask_t) * slot_count);
    bomm_key_iterator_lettermask_init(slot_count, iterator->key.rings, iterator->ring_masks);

    // Find initial positions
    memcpy(&iterator->position_masks, key_space->position_masks, sizeof(bomm_lettermask_t) * slot_count);
    bomm_key_iterator_lettermask_init(slot_count, iterator->key.positions, iterator->position_masks);

    return iterator;
}

/**
 * Increment the iterator. Return false, if the end has been reached.
 */
static inline bool bomm_key_iterator_next(bomm_key_iterator_t* iterator) {
    bomm_key_t* key = &iterator->key;
    unsigned int slot_count = key->slot_count;
    bool relevant = false;
    bool carry = false;

    while (!carry && !relevant) {
        carry = false;
        if (bomm_key_iterator_lettermask_next(slot_count, key->positions, iterator->position_masks)) {
            if (bomm_key_iterator_lettermask_next(slot_count, key->rings, iterator->ring_masks)) {
                carry = bomm_key_iterator_wheel_order_next(iterator, true);
            }
        }

        // Skip redundant positions caused by the double stepping anomaly
        relevant = !carry && (
            key->mechanism != BOMM_MECHANISM_STEPPING ||
            !bomm_lettermask_has(&key->wheels[2].turnovers, key->positions[2])
        );
    }

    return !carry;
}

/**
 * Count the number of elements for the given iterator (needs to be reset).
 */
static inline unsigned int bomm_key_iterator_count(bomm_key_iterator_t* iterator) {
    unsigned int count = 1;
    while (bomm_key_iterator_next(iterator)) {
        count++;
    }
    return count;
}

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
