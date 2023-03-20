//
//  enigma.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "enigma.h"

void bomm_encrypt(bomm_message_t* message, bomm_key_t* key, bomm_message_t* result) {
    bomm_model_t* model = key->model;
    int slot_count = model->slot_count;
    int slot;
    unsigned int index;
    
    // Initiate state from key
    bomm_key_t state;
    memcpy(&state, key, sizeof(bomm_key_t));
    
    // Copy rotor wirings to local variable
    bomm_wheel_t wheels[slot_count];
    for (slot = 0; slot < slot_count; slot++) {
        memcpy(
            &wheels[slot],
            &model->wheels[model->wheel_sets[slot][state.wheels[slot]]],
            sizeof(bomm_wheel_t)
        );
    }
    
    // Initiate result frequency
    memset(&result->frequency, 0, sizeof(unsigned int) * BOMM_ALPHABET_SIZE);
    
    // Simulate the Enigma for each letter
    bool step_next_slot;
    bomm_letter_t x;
    for (index = 0; index < message->length; index++) {
        // Apply the following position
        if (model->mechanism == BOMM_MECHANISM_STEPPING) {
            // The Enigma stepping rotation mechanism assumes 3 rotating wheels
            // with the right-most slot being defined by `fast_rotating_slot`
            slot = slot_count - 2;
            if (bomm_lettermask_has(wheels[slot - 1].turnovers, state.positions[slot - 1])) {
                // If at middle wheel turnover: Step middle and left wheels
                // (double stepping anomaly)
                state.positions[slot - 1] =
                    (state.positions[slot - 1] + 1) % BOMM_ALPHABET_SIZE;
                state.positions[slot - 2] =
                    (state.positions[slot - 2] + 1) % BOMM_ALPHABET_SIZE;
            } else if (bomm_lettermask_has(wheels[slot].turnovers, state.positions[slot])) {
                // If at right wheel turnover: Step middle wheel
                state.positions[slot - 1] =
                    (state.positions[slot - 1] + 1) % BOMM_ALPHABET_SIZE;
            }
            
            // Always step right (fast) wheel
            state.positions[slot] =
                (state.positions[slot] + 1) % BOMM_ALPHABET_SIZE;
        } else {
            // The gear rotation mechanism behaves exactly like an odometer
            // of a car
            step_next_slot = true;
            slot = 0;
            while (step_next_slot && slot >= 0) {
                // Decide whether to step the next wheel on the left
                step_next_slot =
                    slot > 0 &&
                    model->rotating_slots[slot - 1] &&
                    bomm_lettermask_has(
                        wheels[slot].turnovers,
                        state.positions[slot]);
            
                // Step wheel
                state.positions[slot] =
                    (state.positions[slot] + 1) % BOMM_ALPHABET_SIZE;
                slot--;
            }
        }
        
        // Source letter from original message
        x = message->letters[index];
        
        // Plugboard
        x = state.plugboard.map[x];
        
        // Wheels (entry wheel, wheels right to left, reflector wheel)
        for (slot = slot_count - 1; slot >= 0; slot--) {
            x = bomm_map_position_ring(x, state.positions[slot], state.rings[slot]);
            x = wheels[slot].wiring.map[x];
            x = bomm_rev_position_ring(x, state.positions[slot], state.rings[slot]);
        }
        
        // Wheels (wheels left to right, entry wheel)
        for (slot = 1; slot < slot_count; slot++) {
            x = bomm_map_position_ring(x, state.positions[slot], state.rings[slot]);
            x = wheels[slot].wiring.rev[x];
            x = bomm_rev_position_ring(x, state.positions[slot], state.rings[slot]);
        }
        
        // Plugboard
        x = state.plugboard.rev[x];
        
        // Store letter in result message
        result->letters[index] = x;
        result->frequency[x]++;
    }
}
