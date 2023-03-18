//
//  enigma.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "enigma.h"

static inline void _enum_lettermask_init(unsigned char slot_count, bomm_letter_t* positions, bomm_lettermask_t* shifting_masks) {
    memset(positions, 0, BOMM_MODEL_MAX_SLOT_COUNT * sizeof(bomm_letter_t));
    for (int i = 0; i < slot_count; i++) {
        while ((shifting_masks[i] & 0x1) == 0) {
            shifting_masks[i] = shifting_masks[i] >> 1;
            positions[i]++;
        }
    }
}

static inline bool _enum_lettermask(unsigned char slot_count, bomm_letter_t* positions, bomm_lettermask_t* shifting_masks) {
    bool carry = true;
    int i = slot_count;
    while (carry && --i >= 0) {
        // Optimization: If there's only one bit set on the mask we can skip the
        // loops and literally carry on to the next slot
        if (shifting_masks[i] != BOMM_LETTERMASK_FIRST) {
            // Increment position as well as shift and wrap shifting mask
            positions[i]++;
            shifting_masks[i] = BOMM_LETTERMASK_LAST | (shifting_masks[i] >> 1);
            
            // Increment position and shift mask until the next valid position
            // is found
            while ((shifting_masks[i] & 0x1) == 0) {
                positions[i]++;
                shifting_masks[i] = shifting_masks[i] >> 1;
            }
            
            // If position exceeded the alphabet size, we have a carry
            if ((carry = positions[i] >= BOMM_ALPHABET_SIZE)) {
                positions[i] -= BOMM_ALPHABET_SIZE;
            }
        }
    }
    return carry;
}

void bomm_model_attack_phase_1(bomm_model_t* model, bomm_message_t* ciphertext) {
    int slot_count = model->slot_count;
    int i, j;
    bool carry, relevant;
    char key_string[128];
    char message_string[bomm_message_serialize_size(ciphertext)];
    float score;
    float min_score = -INFINITY;
    
    // Prepare leaderboard
    bomm_key_leaderboard_t* leaderboard = bomm_key_leaderboard_alloc(20);
    
    // Prepare working key instance
    bomm_key_t key;
    memset(&key, 0, sizeof(bomm_key_t));
    key.model = model;
    bomm_wiring_extract(&key.plugboard_wiring, BOMM_WIRING_IDENTITY);
    
    // Prepare result message (on the stack)
    char plaintext_store[bomm_message_size_for_length(ciphertext->length)];
    bomm_message_t *plaintext = (bomm_message_t*) &plaintext_store;
    plaintext->length = ciphertext->length;
    
    // Prepare initial ring settings and shifting ring masks
    bomm_lettermask_t slot_shifting_ring_masks[slot_count];
    memcpy(&slot_shifting_ring_masks, model->slot_ring_mask, sizeof(bomm_lettermask_t) * slot_count);
    _enum_lettermask_init(slot_count, key.slot_rings, slot_shifting_ring_masks);
    
    // Prepare initial start positions and shifting position masks
    bomm_lettermask_t slot_shifting_position_masks[slot_count];
    memcpy(&slot_shifting_position_masks, model->slot_position_mask, sizeof(bomm_lettermask_t) * slot_count);
    _enum_lettermask_init(slot_count, key.slot_positions, slot_shifting_position_masks);
    
    // 1. Enumerate relevant wheel orders
    do {
        // Validate if the current wheel order is relevant by checking if no
        // wheel appears twice
        i = -1;
        relevant = true;
        while (relevant && ++i < slot_count - 1) {
            j = i;
            while (relevant && ++j < slot_count) {
                relevant = (
                    model->slot_rotor_indices[i][key.slot_rotor[i]] !=
                    model->slot_rotor_indices[j][key.slot_rotor[j]]
                );
            }
        }
        
        if (relevant) {
            // 2. Enumerate relevant ring settings
            do {
                // 3. Enumerate relevant start positions
                do {
                    // TODO: When the stepping mechanism is used, middle rotor
                    // positions that are equal to one of the middle rotor
                    // turnovers can be neglected as the following middle rotor
                    // position is identical
                    relevant = true;
                    
                    if (relevant) {
                        bomm_model_encrypt(ciphertext, &key, plaintext);
                        score = bomm_message_calc_ic(plaintext);
                        
                        if (score > min_score) {
                            min_score = bomm_key_leaderboard_add(leaderboard, &key, score);
                        }
                    }
                    
                } while (!_enum_lettermask(slot_count, key.slot_positions, slot_shifting_position_masks));
            } while (!_enum_lettermask(slot_count, key.slot_rings, slot_shifting_ring_masks));
            
            // Print progress update
            printf("Working on wheel order:\n");
            bomm_key_serialize(key_string, 128, &key);
            bomm_message_serialize(message_string, -1, plaintext);
            printf("Key: %s\n", key_string);
            printf("Plaintext: %s\n", message_string);
        }
        
        // Iterate to next wheel order
        carry = true;
        i = slot_count;
        while (carry && --i >= 0) {
            key.slot_rotor[i]++;
            if ((carry = model->slot_rotor_indices[i][key.slot_rotor[i]] == 255)) {
                key.slot_rotor[i] = 0;
            }
        }
    } while (!carry);
    
    // Print final leaderboard
    bomm_key_leaderboard_print(leaderboard);
    
    // Clean up
    free(leaderboard);
}

void bomm_model_encrypt(bomm_message_t* message, bomm_key_t* key, bomm_message_t* result) {
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
    for (unsigned int index = 0; index < message->length; index++) {
        // Apply the following position
        if (model->rotation_mechanism == BOMM_ENIGMA_STEPPING) {
            // The Enigma stepping rotation mechanism assumes 3 rotating rotors
            // with the right-most slot being defined by `fast_rotating_slot`
            slot = model->fast_rotating_slot;
            if (bomm_lettermask_has(
                model->rotors[model->slot_rotor_indices[slot - 1][rotor_indices[slot - 1]]].turnovers,
                positions[slot - 1]
            )) {
                // If at middle rotor turnover: Step middle and left rotors
                // (double stepping anomaly)
                positions[slot - 1] = (positions[slot - 1] + 1) % BOMM_ALPHABET_SIZE;
                positions[slot - 2] = (positions[slot - 2] + 1) % BOMM_ALPHABET_SIZE;
            } else if (bomm_lettermask_has(
                model->rotors[model->slot_rotor_indices[slot][rotor_indices[slot]]].turnovers,
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
                    model->rotors[model->slot_rotor_indices[slot - 1][rotor_indices[slot - 1]]].rotating &&
                    bomm_lettermask_has(
                        model->rotors[model->slot_rotor_indices[slot][rotor_indices[slot]]].turnovers,
                        positions[slot]);
            
                // Step rotor this rotor
                positions[slot] = (positions[slot] + 1) % BOMM_ALPHABET_SIZE;
                slot--;
            }
        }
        
        // Source letter from original message
        x = message->letters[index];
        
        // Plugboard
        x = plugboard_wiring.forward_map[x];
        
        // Rotors (entry rotor, then rotors right to left, reflector rotor)
        for (slot = slot_count - 1; slot >= 0; slot--) {
            x = bomm_map_position_ring(x, positions[slot], rings[slot]);
            x = model->rotors[model->slot_rotor_indices[slot][rotor_indices[slot]]].wiring.forward_map[x];
            x = bomm_rev_position_ring(x, positions[slot], rings[slot]);
        }
        
        // Rotors (rotors left to right, then entry rotor)
        for (slot = 1; slot < slot_count; slot++) {
            x = bomm_map_position_ring(x, positions[slot], rings[slot]);
            x = model->rotors[model->slot_rotor_indices[slot][rotor_indices[slot]]].wiring.backward_map[x];
            x = bomm_rev_position_ring(x, positions[slot], rings[slot]);
        }
        
        // Plugboard
        x = plugboard_wiring.backward_map[x];
        
        // Store letter in result message
        result->letters[index] = x;
        result->frequency[x]++;
    }
}
