//
//  key.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef key_h
#define key_h

#include <stdio.h>
#include "hold.h"
#include "wheel.h"
#include "wiring.h"
#include "lettermask.h"

#define BOMM_MAX_NUM_SLOTS 6
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
 * Extract a mechanism value from the given string.
 */
static inline bomm_mechanism_t bomm_key_mechanism_from_string(
    const char* mechanism_string
) {
    if (strcmp(mechanism_string, "stepping") == 0) {
        return BOMM_MECHANISM_STEPPING;
    } else if (strcmp(mechanism_string, "odometer") == 0) {
        return BOMM_MECHANISM_ODOMETER;
    } else {
        return BOMM_MECHANISM_NONE;
    }
}

/**
 * Return the string value for the given mechanism.
 */
static inline char* bomm_key_mechanism_string(bomm_mechanism_t mechanism) {
    switch (mechanism) {
        case BOMM_MECHANISM_NONE:
            return "none";
        case BOMM_MECHANISM_STEPPING:
            return "stepping";
        case BOMM_MECHANISM_ODOMETER:
            return "odometer";
        default:
            return NULL;
    }
}

/**
 * Struct representing a key space, from which a set of keys can be derived.
 * It can be traversed using an `bomm_key_iterator_t` and related functions.
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
    unsigned int num_slots;

    /**
     * Set of wheels per slot.
     * Each set is terminated with a null pointer.
     */
    bomm_wheel_t wheel_sets[BOMM_MAX_NUM_SLOTS]
        [BOMM_MAX_WHEEL_SET_SIZE + 1];

    /**
     * Whether a wheel is rotating for each slot
     */
    bool rotating_slots[BOMM_MAX_NUM_SLOTS];

    /**
     * Possible ring settings for each slot.
     * Must not be empty (equal to 0).
     */
    bomm_lettermask_t ring_masks[BOMM_MAX_NUM_SLOTS];

    /**
     * Possible start positions for each slot.
     * Must not be empty (equal to 0).
     */
    bomm_lettermask_t position_masks[BOMM_MAX_NUM_SLOTS];

    /**
     * Mask containing plugs to be exhausted.
     * Example: The I-Stecker with letters E, N, R, X, S, or I: `0x862110`
     */
    bomm_lettermask_t plug_mask;

    /**
     * Number of elements contained. Set to 0 if unknown.
     */
    unsigned long num_keys;

    /**
     * The number of keys to be skipped at the beginning of the key space
     */
    unsigned long offset;

    /**
     * The maximum number of keys to be enumerated in this key space (not
     * including the offset)
     */
    unsigned long limit;
} bomm_key_space_t;

/**
 * Struct describing a single location in a search space.
 * Struct is optimized to being used in-place.
 */
typedef struct _bomm_key {
    /**
     * Rotation mechanism
     */
    bomm_mechanism_t mechanism;

    /**
     * Index of the fast wheel slot when using the stepping mechanism.
     * Undefined, when the mechanism is not set to `BOMM_MECHANISM_STEPPING`.
     */
    unsigned int fast_wheel_slot;

    /**
     * Number of wheel slots; The first slot represents the reflector, the last
     * slot represents the entry wheel.
     */
    unsigned int num_slots;

    /**
     * Wheel instances matching the wheels of the key
     */
    bomm_wheel_t wheels[BOMM_MAX_NUM_SLOTS];

    /**
     * Whether wheel is rotating for each slot
     */
    bool rotating_slots[BOMM_MAX_NUM_SLOTS];

    /**
     * Ring setting (Ringstellung): Ring position of the wheel in each slot
     */
    unsigned int rings[BOMM_MAX_NUM_SLOTS];

    /**
     * Start position (Walzenstellung): Start position of the wheel in each slot
     */
    unsigned int positions[BOMM_MAX_NUM_SLOTS];

    /**
     * Plugboard setting (Steckerverbindungen) to be used
     */
    unsigned int plugboard[BOMM_ALPHABET_SIZE];
} bomm_key_t;

/**
 * Iterator struct that enables traversing a key space.
 */
typedef struct _bomm_key_iterator {
    /**
     * Key space iterator was created for
     */
    const bomm_key_space_t* key_space;

    /**
     * Index of the current key in the key space
     */
    unsigned long index;

    /**
     * Current key
     */
    bomm_key_t key;

    /**
     * Whether the scrambler portion of the key has changed during the last
     * `bomm_key_iterator_next` call (wheel indices, rings, positions).
     * Initially set to `true`.
     */
    bool scrambler_changed;

    /**
     * Wheel set index per slot
     */
    unsigned int wheel_indices[BOMM_MAX_NUM_SLOTS];

    /**
     * Shifting ring mask per slot
     */
    bomm_lettermask_t ring_masks[BOMM_MAX_NUM_SLOTS];

    /**
     * Shifting position mask per slot
     */
    bomm_lettermask_t position_masks[BOMM_MAX_NUM_SLOTS];

    /**
     * Solo plug counter
     */
    unsigned int solo_plug[2];
} bomm_key_iterator_t;

/**
 * Initialize a key space with the given mechanism and number of slots.
 */
bomm_key_space_t* bomm_key_space_init(
    bomm_key_space_t* key_space,
    bomm_mechanism_t mechanism,
    unsigned int num_slots
);

/**
 * Initialize a key space for the Enigma I model.
 */
bomm_key_space_t* bomm_key_space_init_enigma_i(bomm_key_space_t* key_space);

/**
 * Extract a key space from the given JSON object.
 * @param key_space Existing key space or NULL to allocate a new one
 * @param key_space_json JSON key space object
 * @param wheels Pointer to an array of wheels for lookup
 * @param num_wheels Number of elements in `wheels`
 */
bomm_key_space_t* bomm_key_space_init_with_json(
    bomm_key_space_t* key_space,
    json_t* key_space_json,
    bomm_wheel_t wheels[],
    unsigned int num_wheels
);

/**
 * Print the contents of the given key space to stdout. Useful for debugging.
 */
void bomm_key_space_debug(const bomm_key_space_t* key_space);

/**
 * Destroy the given key space.
 */
void bomm_key_space_destroy(bomm_key_space_t* key_space);

/**
 * Count the number of plugboard configurations in the given key space.
 */
static inline unsigned int bomm_key_space_plugboard_count(
    const bomm_key_space_t* key_space
) {
    unsigned int num_plugboards = bomm_lettermask_count(&key_space->plug_mask);
    return
        (BOMM_ALPHABET_SIZE - 1) * BOMM_ALPHABET_SIZE / 2 -
        (BOMM_ALPHABET_SIZE - 1 - num_plugboards) *
        (BOMM_ALPHABET_SIZE - num_plugboards) / 2 + 1;
}

/**
 * Count the number of elements in the given key space.
 */
unsigned long bomm_key_space_count(
    const bomm_key_space_t* key_space
);

/**
 * Split a key space into the given number of slices.
 * @return Actual number of slices
 */
unsigned int bomm_key_space_slice(
    const bomm_key_space_t* key_space,
    unsigned int num_slices,
    bomm_key_space_t* slices
);

/**
 * Initialize a key from the given key space. Wheels are initialized to the
 * first wheel in each wheel set (might not be valid for duplicate wheels).
 * @param key Pointer to an existing key in memory or null, if a new key
 * should be allocated and returned.
 */
bomm_key_t* bomm_key_init(bomm_key_t* key, const bomm_key_space_t* key_space);

/**
 * Iterate to the 'next' plugboard configuration in the key space.
 */
static inline bool bomm_key_iterator_plugboard_next(
    bomm_key_iterator_t* iterator
) {
    bomm_lettermask_t mask = iterator->key_space->plug_mask;
    if (mask == BOMM_LETTERMASK_NONE) {
        return true;
    }

    bool carry = false;

    // Unplug previous solo
    bomm_swap(
        &iterator->key.plugboard[iterator->solo_plug[0]],
        &iterator->key.plugboard[iterator->solo_plug[1]]
    );

    // Iterate to next relevant solo
    do {
        if (++iterator->solo_plug[1] == BOMM_ALPHABET_SIZE) {
            if (++iterator->solo_plug[0] == BOMM_ALPHABET_SIZE - 1) {
                carry = true;
                // Reset to initial self-steckered pair
                iterator->solo_plug[0] = 0;
                iterator->solo_plug[1] = 0;
            } else {
                iterator->solo_plug[1] = iterator->solo_plug[0] + 1;
            }
        }
    } while (
        !bomm_lettermask_has(&mask, iterator->solo_plug[0]) &&
        !bomm_lettermask_has(&mask, iterator->solo_plug[1]) &&
        !carry
    );

    // Plug new solo
    bomm_swap(
        &iterator->key.plugboard[iterator->solo_plug[0]],
        &iterator->key.plugboard[iterator->solo_plug[1]]
    );

    return carry;
}

/**
 * Apply initial values to a set of positions using the given shifting masks.
 * @return True, if no initial set of positions is available.
 */
static inline bool bomm_key_iterator_positions_init(
    unsigned int* positions,
    bomm_lettermask_t* shifting_masks,
    unsigned int num_positions
) {
    memset(positions, 0, num_positions * sizeof(bomm_letter_t));
    bool empty = false;
    for (unsigned int slot = 0; slot < num_positions; slot++) {
        if (!(empty = empty || shifting_masks[slot] == BOMM_LETTERMASK_NONE)) {
            while ((shifting_masks[slot] & 0x1) == 0) {
                shifting_masks[slot] = shifting_masks[slot] >> 1;
                positions[slot]++;
            }
        }
    }
    return empty;
}

/**
 * Increment a set of positions like an odometer using the given shifting masks.
 * @return Whether a full revolution was completed (carry)
 */
static inline bool bomm_key_iterator_positions_next(
    unsigned int* positions,
    bomm_lettermask_t* shifting_masks,
    unsigned int num_positions
) {
    bool carry = true;
    int slot = num_positions;
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
 * Validate the iterator wheel order, i.e. make sure no wheel appears twice.
 */
static inline bool bomm_key_iterator_wheels_validate(
    bomm_key_iterator_t* iterator
) {
    int num_slots = iterator->key_space->num_slots;
    for (int i = 0; i < num_slots; i++) {
        for (int j = i + 1; j < num_slots; j++) {
            if (strcmp(
                iterator->key_space->wheel_sets[i][iterator->wheel_indices[i]].name,
                iterator->key_space->wheel_sets[j][iterator->wheel_indices[j]].name
            ) == 0) {
                return false;
            }
        }
    }
    return true;
}

/**
 * Move to the next valid set of wheels (wheel order).
 * @param increment If set to false, does not increment a set of wheels that is
 * already valid.
 * @return Whether a full revolution was completed (carry)
 */
static inline bool bomm_key_iterator_wheels_next(
    bomm_key_iterator_t* iterator,
    bool increment
) {
    unsigned int num_slots = iterator->key_space->num_slots;
    unsigned int revolutions = 0;
    while (revolutions < 2 && (!bomm_key_iterator_wheels_validate(iterator) || increment)) {
        increment = false;
        bool carry = true;
        int slot = num_slots;
        while (carry && --slot >= 0) {
            iterator->wheel_indices[slot]++;
            if ((carry = (iterator->key_space->wheel_sets[slot][iterator->wheel_indices[slot]].name[0] == '\0'))) {
                iterator->wheel_indices[slot] = 0;
            }
            memcpy(
                &iterator->key.wheels[slot],
                &iterator->key_space->wheel_sets[slot][iterator->wheel_indices[slot]],
                sizeof(bomm_wheel_t)
            );
        }
        if (carry) {
            revolutions++;
        }
    }
    return revolutions > 0;
}

/**
 * Initialize an iterator for the given key space.
 * @return Pointer to iterator or NULL, if out of memory or if the key space
 * is empty.
 */
bomm_key_iterator_t* bomm_key_iterator_init(
    bomm_key_iterator_t* iterator,
    const bomm_key_space_t* key_space
);

/**
 * Determine the relevancy of the given key. An irrelevant key is one that is
 * equivalent to a canonicalized version of itself. Right now, keys equivalent
 * due to the double stepping anomaly are flagged as irrelevant.
 */
static inline bool bomm_key_is_relevant(bomm_key_t* key) {
    // TODO: Make ready for 4 rotor machines like M4
    return (
        key->mechanism != BOMM_MECHANISM_STEPPING ||
        !bomm_lettermask_has(&key->wheels[2].turnovers, key->positions[2])
    );
}

/**
 * Increment the given key iterator, ignoring the plugboard setting.
 * @return Whether a full revolution was completed (carry)
 */
static inline __attribute__((always_inline)) bool bomm_key_iterator_next(
    bomm_key_iterator_t* iterator
) {
    bomm_key_t* key = &iterator->key;
    unsigned int num_slots = key->num_slots;
    bool carry_out = false;
    bool scrambler_changed = false;
    do {
        bool plugboard_carry = bomm_key_iterator_plugboard_next(iterator);
        bool carry =
            plugboard_carry &&
            bomm_key_iterator_positions_next(
                key->positions, iterator->position_masks, num_slots) &&
            bomm_key_iterator_positions_next(
                key->rings, iterator->ring_masks, num_slots) &&
            bomm_key_iterator_wheels_next(
                iterator, true);
        scrambler_changed = scrambler_changed || plugboard_carry;
        carry_out = carry_out || carry;
    } while (!bomm_key_is_relevant(key));

    iterator->index++;
    iterator->scrambler_changed = scrambler_changed;

    if (carry_out || iterator->index >= iterator->key_space->limit) {
        // Reset iterator to re-establish initial key and offset
        bomm_key_iterator_init(iterator, iterator->key_space);
        return true;
    }

    return false;
}

/**
 * Export the given key to a string
 */
void bomm_key_stringify(char* str, size_t size, bomm_key_t* key);
void bomm_key_wheels_stringify(char* str, size_t size, bomm_key_t* key);
void bomm_key_rings_stringify(char* str, size_t size, bomm_key_t* key);
void bomm_key_positions_stringify(char* str, size_t size, bomm_key_t* key);

/**
 * Print the contents of the given key to stdout. Useful for debugging.
 */
void bomm_key_debug(const bomm_key_t* key);

#endif /* key_h */
