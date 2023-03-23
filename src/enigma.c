//
//  enigma.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "enigma.h"

/**
 * Engage the Enigma mechanism, transitioning to the next state.
 */
inline static void _engage_mechanism(bomm_key_t* state, bomm_wheel_t* wheels) {
    if (state->model->mechanism == BOMM_MECHANISM_STEPPING) {
        // The Enigma stepping rotation mechanism assumes 1 entry wheel,
        // 3 rotating wheels, and 1 reflector. The fast rotating wheel is
        // assumed to be in the second last slot.
        if (bomm_lettermask_has(
            &wheels[2].turnovers,
            state->positions[2] % BOMM_ALPHABET_SIZE
        )) {
            // If at middle wheel turnover: Step middle and left wheels
            // (double stepping anomaly)
            state->positions[2]++;
            state->positions[1]++;
        } else if (bomm_lettermask_has(
            &wheels[3].turnovers,
            state->positions[3] % BOMM_ALPHABET_SIZE
        )) {
            // If at right wheel turnover: Step middle wheel
            state->positions[2]++;
        }
        
        // Always step right (fast) wheel
        state->positions[3]++;
    } else {
        // The gear rotation mechanism behaves exactly like an odometer
        // of a car
        bool step_next_slot = true;
        int slot = 0;
        while (step_next_slot && slot >= 0) {
            // Decide whether to step the next wheel on the left
            step_next_slot =
                slot > 0 &&
                state->model->rotating_slots[slot - 1] &&
                bomm_lettermask_has(
                    &wheels[slot].turnovers,
                    state->positions[slot] % BOMM_ALPHABET_SIZE);
        
            // Step wheel
            state->positions[slot]++;
            slot--;
        }
    }
}

/**
 * Send the given letter through the Enigma scrambler (entry wheel, rotating
 * wheels, and reflector wheels) excluding the plugboard.
 */
inline static int _scramble_letter(int x, bomm_key_t* state, bomm_wheel_t* wheels) {
    int slot_count = state->model->slot_count;
    int slot;
    
    // Wheels (entry wheel, wheels right to left, reflector wheel)
    for (slot = slot_count - 1; slot >= 0; slot--) {
        x += state->positions[slot] - state->rings[slot];
        x = wheels[slot].wiring.map[bomm_mod(x, BOMM_ALPHABET_SIZE)];
        x += state->rings[slot] - state->positions[slot];
    }
    
    // Wheels (wheels left to right, entry wheel)
    for (slot = 1; slot < slot_count; slot++) {
        x += state->positions[slot] - state->rings[slot];
        x = wheels[slot].wiring.rev[bomm_mod(x, BOMM_ALPHABET_SIZE)];
        x += state->rings[slot] - state->positions[slot];
    }
    
    return bomm_mod(x, BOMM_ALPHABET_SIZE);
}

void bomm_encrypt(bomm_message_t* message, bomm_key_t* key, bomm_message_t* result) {
    // Initiate Enigma state from key
    bomm_key_t state;
    memcpy(&state, key, sizeof(bomm_key_t));
    
    // Copy rotor wirings to local variable
    bomm_wheel_t wheels[key->model->slot_count];
    for (unsigned int slot = 0; slot < key->model->slot_count; slot++) {
        memcpy(
            &wheels[slot],
            &key->model->wheels[key->model->wheel_sets[slot][state.wheels[slot]]],
            sizeof(bomm_wheel_t)
        );
    }
    
    // Reset result message frequency
    memset(&result->frequency, 0, sizeof(unsigned int) * BOMM_ALPHABET_SIZE);
    
    // Simulate the Enigma for each letter
    int letter;
    for (unsigned int index = 0; index < message->length; index++) {
        _engage_mechanism(&state, wheels);
        
        letter = message->letters[index];
        
        letter = state.plugboard[letter];
        letter = _scramble_letter(letter, &state, wheels);
        letter = state.plugboard[letter];
        
        result->letters[index] = letter;
        result->frequency[letter]++;
    }
}

void bomm_scrambler_generate(bomm_scrambler_t* scrambler, bomm_key_t* key) {
    // Initiate Enigma state from key
    bomm_key_t state;
    memcpy(&state, key, sizeof(bomm_key_t));
    
    // Copy rotor wirings to local variable
    bomm_wheel_t wheels[key->model->slot_count];
    for (unsigned int slot = 0; slot < key->model->slot_count; slot++) {
        memcpy(
            &wheels[slot],
            &key->model->wheels[key->model->wheel_sets[slot][state.wheels[slot]]],
            sizeof(bomm_wheel_t)
        );
    }
    
    unsigned int index, letter;
    for (index = 0; index < scrambler->length; index++) {
        // Engage Enigma mechanism
        _engage_mechanism(&state, wheels);
        
        // Create map for this index
        for (letter = 0; letter < BOMM_ALPHABET_SIZE; letter++) {
            scrambler->map[index][letter] = _scramble_letter(letter, &state, wheels);
        }
    }
}
