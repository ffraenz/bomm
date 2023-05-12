//
//  enigma.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef enigma_h
#define enigma_h

#include <stdio.h>
#include <stdlib.h>
#include "key.h"
#include "utility.h"

inline static void bomm_enigma_engage_mechanism(bomm_key_t* state);
inline static int bomm_enigma_scramble_letter(int x, bomm_key_t* state);

/**
 * Simulate the Enigma on the given original message and key.
 * Original and result message structs are assumed to be of same length.
 * The message frequency of the result is updated.
 * The key is used in-place during evaluation and is reset afterwards.
 */
inline static void bomm_enigma_encrypt(
    bomm_message_t* message,
    bomm_key_t* key,
    bomm_message_t* result
) {
    // Store the original positions as we will reinstate them afterwards
    unsigned int original_positions[key->slot_count];
    memcpy(&original_positions, key->positions, sizeof(original_positions));

    // Simulate the Enigma for each letter
    int letter;
    for (unsigned int index = 0; index < message->length; index++) {
        // Engaging the mechanism will change the key
        bomm_enigma_engage_mechanism(key);

        letter = message->letters[index];
        letter = key->plugboard[letter];
        letter = bomm_enigma_scramble_letter(letter, key);
        letter = key->plugboard[letter];

        result->letters[index] = letter;
    }

    // Reinstate original positions
    memcpy(key->positions, &original_positions, sizeof(original_positions));
}

/**
 * Generate letter maps for each letter in a message of the given scrambler
 * length. The key's plugboard is irrelevant when evaluating this function.
 * The key is used in-place during evaluation and is reset afterwards.
 */
inline static __attribute__((always_inline)) void bomm_enigma_generate_scrambler(
    bomm_scrambler_t* scrambler,
    bomm_key_t* key
) {
    // Store the original positions as we will reinstate them afterwards
    unsigned int original_positions[key->slot_count];
    memcpy(&original_positions, key->positions, sizeof(original_positions));

    unsigned int index, letter;
    for (index = 0; index < scrambler->length; index++) {
        // Engaging the mechanism will change the key
        bomm_enigma_engage_mechanism(key);

        // Create map for this index
        for (letter = 0; letter < BOMM_ALPHABET_SIZE; letter++) {
            scrambler->map[index][letter] =
                bomm_enigma_scramble_letter(letter, key);
        }
    }

    // Reinstate original positions
    memcpy(key->positions, &original_positions, sizeof(original_positions));
}

/**
 * Transition the given state to the next state in-place.
 * Assumption: Only positions are manipulated.
 */
inline static void bomm_enigma_engage_mechanism(bomm_key_t* state) {
    switch (state->mechanism) {
        case BOMM_MECHANISM_NONE: {
            break;
        }
        case BOMM_MECHANISM_STEPPING: {
            // The Enigma stepping rotation mechanism assumes 1 entry wheel,
            // 3 rotating wheels, and 1 reflector. The fast rotating wheel is
            // assumed to be in the second last slot.
            if (bomm_lettermask_has(
                &state->wheels[2].turnovers,
                state->positions[2] % BOMM_ALPHABET_SIZE
            )) {
                // If at middle wheel turnover: Step middle and left wheels
                // (double stepping anomaly)
                state->positions[2]++;
                state->positions[1]++;
            } else if (bomm_lettermask_has(
                &state->wheels[3].turnovers,
                state->positions[3] % BOMM_ALPHABET_SIZE
            )) {
                // If at right wheel turnover: Step middle wheel
                state->positions[2]++;
            }

            // Always step right (fast) wheel
            state->positions[3]++;
            break;
        }
        case BOMM_MECHANISM_ODOMETER: {
            // The gear rotation mechanism behaves exactly like an odometer
            // of a car
            bool step_next_slot = true;
            int slot = 0;
            while (step_next_slot && slot >= 0) {
                // Decide whether to step the next wheel on the left
                step_next_slot =
                    slot > 0 &&
                    state->rotating_slots[slot - 1] &&
                    bomm_lettermask_has(
                        &state->wheels[slot].turnovers,
                        state->positions[slot] % BOMM_ALPHABET_SIZE);

                // Step wheel
                state->positions[slot]++;
                slot--;
            }
            break;
        }
        default: {
            fprintf(stderr, "Encountered an unexpected mechanism\n");
            exit(1);
        }
    }
}

/**
 * Send the given letter through the Enigma scrambler (entry wheel, rotating
 * wheels, and reflector wheels) excluding the plugboard.
 */
inline static int bomm_enigma_scramble_letter(int x, bomm_key_t* state) {
    int slot_count = state->slot_count;
    int slot;

    // Wheels (entry wheel, wheels right to left, reflector wheel)
    for (slot = slot_count - 1; slot >= 0; slot--) {
        x += state->positions[slot] - state->rings[slot];
        x = state->wheels[slot].wiring.map[bomm_mod(x, BOMM_ALPHABET_SIZE)];
        x += state->rings[slot] - state->positions[slot];
    }

    // Wheels (wheels left to right, entry wheel)
    for (slot = 1; slot < slot_count; slot++) {
        x += state->positions[slot] - state->rings[slot];
        x = state->wheels[slot].wiring.rev[bomm_mod(x, BOMM_ALPHABET_SIZE)];
        x += state->rings[slot] - state->positions[slot];
    }

    return bomm_mod(x, BOMM_ALPHABET_SIZE);
}

#endif /* enigma_h */
