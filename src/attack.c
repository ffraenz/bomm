//
//  attack.c
//  Bomm
//
//  Created by Fränz Friederes on 20/03/2023.
//

#include "attack.h"

static inline void _enum_lettermask_init(unsigned char slot_count, unsigned int* positions, bomm_lettermask_t* shifting_masks) {
    memset(positions, 0, BOMM_MODEL_MAX_SLOT_COUNT * sizeof(bomm_letter_t));
    for (int i = 0; i < slot_count; i++) {
        while ((shifting_masks[i] & 0x1) == 0) {
            shifting_masks[i] = shifting_masks[i] >> 1;
            positions[i]++;
        }
    }
}

static inline bool _enum_lettermask(unsigned char slot_count, unsigned int* positions, bomm_lettermask_t* shifting_masks) {
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

void bomm_attack_phase_1(bomm_model_t* model, bomm_message_t* ciphertext) {
    int i, j;
    bool carry, relevant;
    char key_string[128];
    float score;
    float min_score = -INFINITY;
    time_t start_time;
    float duration;
    int slot_count = model->slot_count;
    
    // Prepare leaderboard
    bomm_key_leaderboard_t* leaderboard = bomm_key_leaderboard_alloc(20);
    
    // Prepare working key instance
    bomm_key_t key;
    memset(&key, 0, sizeof(bomm_key_t));
    key.model = model;
    bomm_wiring_extract(&key.plugboard, BOMM_WIRING_IDENTITY);
    
    // Prepare result message (on the stack)
    char plaintext_store[bomm_message_size_for_length(ciphertext->length)];
    bomm_message_t *plaintext = (bomm_message_t*) &plaintext_store;
    plaintext->length = ciphertext->length;
    
    // Prepare initial ring settings and shifting ring masks
    bomm_lettermask_t slot_shifting_ring_masks[slot_count];
    memcpy(&slot_shifting_ring_masks, model->ring_masks, sizeof(bomm_lettermask_t) * slot_count);
    _enum_lettermask_init(slot_count, key.rings, slot_shifting_ring_masks);
    
    // Prepare initial start positions and shifting position masks
    bomm_lettermask_t slot_shifting_position_masks[slot_count];
    memcpy(&slot_shifting_position_masks, model->position_masks, sizeof(bomm_lettermask_t) * slot_count);
    _enum_lettermask_init(slot_count, key.positions, slot_shifting_position_masks);
    
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
                    model->wheel_sets[i][key.wheels[i]] !=
                    model->wheel_sets[j][key.wheels[j]]
                );
            }
        }
        
        if (relevant) {
            start_time = clock();
            
            // 2. Enumerate relevant ring settings
            do {
                // 3. Enumerate relevant start positions
                do {
                    // TODO: When the stepping mechanism is used, middle wheel
                    // positions that are equal to one of the middle wheel
                    // turnovers can be neglected as the following middle wheel
                    // position is identical
                    relevant = true;
                    
                    if (relevant) {
                        bomm_encrypt(ciphertext, &key, plaintext);
                        score = bomm_message_calc_ic(plaintext);
                        
                        if (score > min_score) {
                            min_score = bomm_key_leaderboard_add(leaderboard, &key, score);
                        }
                    }
                    
                } while (!_enum_lettermask(slot_count, key.positions, slot_shifting_position_masks));
            } while (!_enum_lettermask(slot_count, key.rings, slot_shifting_ring_masks));
            
            // Print progress update
            duration = (float) (clock() - start_time) / CLOCKS_PER_SEC;
            
            bomm_key_serialize_wheel_order(key_string, 128, &key);
            printf("\n");
            printf("Wheel order: %s\n", key_string);
            printf("Duration: %f seconds\n", duration);
            printf("Leaderboard:\n");
            bomm_key_leaderboard_print(leaderboard);
        }
        
        // Iterate to next wheel order
        carry = true;
        i = slot_count;
        while (carry && --i >= 0) {
            key.wheels[i]++;
            if ((carry = model->wheel_sets[i][key.wheels[i]] == 255)) {
                key.wheels[i] = 0;
            }
        }
    } while (!carry);
    
    // Clean up
    free(leaderboard);
}
