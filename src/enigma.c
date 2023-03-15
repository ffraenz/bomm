//
//  enigma.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "enigma.h"

void bomm_model_encrypt(bomm_message_t* original, bomm_key_t* key, bomm_message_t* result) {
    int slot_count = key->model->slot_count;
    bomm_model_t* model = key->model;
    
    // Enigma state
    bomm_rotor_index_t rotor_indices[slot_count];
    bomm_letter_t positions[slot_count];
    bomm_letter_t rings[slot_count];
    bomm_wiring_t plugboard_wiring;
    
    // Apply key
    for (int i = 0; i < slot_count; i++) {
        rotor_indices[i] = key->slot_rotor[i];
        positions[i] = key->slot_positions[i];
        rings[i] = key->slot_rings[i];
        memcpy(&plugboard_wiring, &key->plugboard_wiring, sizeof(bomm_wiring_t));
    }
    
    // Zero result letter frequency array
    memset(&result->frequency, 0, sizeof(unsigned int) * BOMM_ALPHABET_SIZE);
    
    // Simulate the Enigma for each letter
    int slot;
    bool step_next_slot;
    bomm_letter_t x;
    for (unsigned int index = 0; index < original->length; index++) {
        // Apply the following position
        if (model->rotation_mechanism == BOMM_ENIGMA_STEPPING) {
            // The Enigma stepping rotation mechanism assumes 3 rotating rotors
            // with the right-most slot being defined by `fast_rotating_slot`
            slot = model->fast_rotating_slot;
            if (bomm_lettermask_has(
                model->rotors[rotor_indices[slot - 1]].turnovers,
                positions[slot - 1]
            )) {
                // If at middle rotor turnover: Step middle and left rotors
                // (double stepping anomaly)
                positions[slot - 1] = (positions[slot - 1] + 1) % BOMM_ALPHABET_SIZE;
                positions[slot - 2] = (positions[slot - 2] + 1) % BOMM_ALPHABET_SIZE;
            } else if (bomm_lettermask_has(
                model->rotors[rotor_indices[slot]].turnovers,
                positions[slot]
            )) {
                // If at right rotor turnover: Step middle rotor
                positions[slot - 1] = (positions[slot - 1] + 1) % BOMM_ALPHABET_SIZE;
            }
            
            // Always step right (fast) rotor
            positions[slot] = (positions[slot] + 1) % BOMM_ALPHABET_SIZE;
        } else {
            // The gear rotation mechanism behaves exactly like an odometer
            // of a car
            step_next_slot = true;
            slot = model->fast_rotating_slot;
            while (step_next_slot && slot >= 0) {
                // Decide whether to step the next rotor on the left
                step_next_slot =
                    slot > 0 &&
                    model->rotors[rotor_indices[slot - 1]].rotating &&
                    bomm_lettermask_has(
                        model->rotors[rotor_indices[slot]].turnovers,
                        positions[slot]);
            
                // Step rotor this rotor
                positions[slot] = (positions[slot] + 1) % BOMM_ALPHABET_SIZE;
                slot--;
            }
        }
        
        // Source letter from original message
        x = original->letters[index];
        
        // Plugboard
        x = plugboard_wiring.forward_map[x];
        
        // Rotors (entry rotor, then rotors right to left, reflector rotor)
        for (slot = slot_count - 1; slot >= 0; slot--) {
            x = bomm_map_position_ring(x, positions[slot], rings[slot]);
            x = model->rotors[rotor_indices[slot]].wiring.forward_map[x];
            x = bomm_rev_position_ring(x, positions[slot], rings[slot]);
        }
        
        // Rotors (rotors left to right, then entry rotor)
        for (slot = 1; slot < slot_count; slot++) {
            x = bomm_map_position_ring(x, positions[slot], rings[slot]);
            x = model->rotors[rotor_indices[slot]].wiring.backward_map[x];
            x = bomm_rev_position_ring(x, positions[slot], rings[slot]);
        }
        
        // Plugboard
        x = plugboard_wiring.backward_map[x];
        
        // Store letter in result message
        result->letters[index] = x;
        result->frequency[x]++;
    }
}
