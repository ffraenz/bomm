//
//  attack.c
//  Bomm
//
//  Created by Fränz Friederes on 20/03/2023.
//

#include "attack.h"

static inline void _enum_lettermask_init(unsigned char slot_count, unsigned int* positions, bomm_lettermask_t* shifting_masks) {
    memset(positions, 0, BOMM_MAX_SLOT_COUNT * sizeof(bomm_letter_t));
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

void bomm_attack_destroy(bomm_attack_t* attack) {
    free(attack);
}

void* bomm_attack_execute(void* arg) {
    // The argument is assumed to be an attack slice
    bomm_attack_t* attack = (bomm_attack_t*) arg;
    bomm_attack_key_space(attack);
    return NULL;
}

void bomm_attack_key_space(bomm_attack_t* attack) {
    int i, j;
    bool carry, relevant;
    char key_string[128];
    float score;
    float min_score = -INFINITY;

    // Prepare working key instance
    bomm_key_space_t* key_space = &attack->key_space;
    bomm_mechanism_t mechanism = key_space->mechanism;
    bomm_hold_t* hold = attack->query->hold;
    int slot_count = key_space->slot_count;
    bomm_key_t key;
    bomm_key_init(&key, key_space);
    unsigned int wheel_indices[slot_count];
    memset(wheel_indices, 0, sizeof(unsigned int) * slot_count);
    
    // Copy ciphertext to stack
    char ciphertext_store[bomm_message_size_for_length(attack->ciphertext->length)];
    memcpy(&ciphertext_store, attack->ciphertext, sizeof(ciphertext_store));
    bomm_message_t *ciphertext = (bomm_message_t*) &ciphertext_store;
    
    // Reserve space for plaintext on the stack
    char plaintext_store[bomm_message_size_for_length(ciphertext->length)];
    bomm_message_t *plaintext = (bomm_message_t*) &plaintext_store;
    char hold_preview[BOMM_HOLD_PREVIEW_SIZE];

    // Prepare scrambler
    char scrambler_store[bomm_scrambler_size(ciphertext->length)];
    bomm_scrambler_t *scrambler = (bomm_scrambler_t*) &scrambler_store;
    scrambler->length = ciphertext->length;

    // Prepare initial ring settings and shifting ring masks
    bomm_lettermask_t slot_shifting_ring_masks[slot_count];
    memcpy(&slot_shifting_ring_masks, key_space->ring_masks, sizeof(bomm_lettermask_t) * slot_count);
    _enum_lettermask_init(slot_count, key.rings, slot_shifting_ring_masks);

    // Prepare initial start positions and shifting position masks
    bomm_lettermask_t slot_shifting_position_masks[slot_count];
    memcpy(&slot_shifting_position_masks, key_space->position_masks, sizeof(bomm_lettermask_t) * slot_count);
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
                    key_space->wheel_sets[i][wheel_indices[i]] !=
                    key_space->wheel_sets[j][wheel_indices[j]]
                );
            }
        }

        if (relevant) {
            // Load wheels into the working key
            // TODO: Optimize by only loading changing wheels
            for (i = 0; i < slot_count; i++) {
                memcpy(&key.wheels[i], key_space->wheel_sets[i][wheel_indices[i]], sizeof(bomm_wheel_t));
            }
            
            // Print progress update
            bomm_key_serialize_wheel_order(key_string, 128, &key);

            // 2. Enumerate relevant ring settings
            do {
                // 3. Enumerate relevant start positions
                do {
                    // Skip redundant positions caused by the double stepping anomaly
                    relevant =
                        mechanism != BOMM_MECHANISM_STEPPING ||
                        !bomm_lettermask_has(&key.wheels[2].turnovers, key.positions[2]);

                    if (relevant) {
                        // Generate scrambler
                        bomm_enigma_generate_scrambler(scrambler, &key);

                        // Attack ciphertext
                        score = bomm_attack_plugboard(key.plugboard, scrambler, ciphertext);

                        if (score > min_score) {
                            // Generate preview
                            bomm_scrambler_encrypt(scrambler, key.plugboard, ciphertext, plaintext);
                            bomm_message_serialize(hold_preview, BOMM_HOLD_PREVIEW_SIZE, plaintext);
                            
                            // Add to hold
                            min_score = bomm_hold_add(hold, score, &key, hold_preview);
                        }
                    }
                } while (!_enum_lettermask(slot_count, key.positions, slot_shifting_position_masks));
            } while (!_enum_lettermask(slot_count, key.rings, slot_shifting_ring_masks));
        }

        // Iterate to next wheel order
        carry = true;
        i = slot_count;
        while (carry && --i >= 0) {
            wheel_indices[i]++;
            if ((carry = key_space->wheel_sets[i][wheel_indices[i]] == NULL)) {
                wheel_indices[i] = 0;
            }
        }
    } while (!carry);
}

// During the hillclimb we exhaust the following plugs in order
// The I-Stecker strategy starts with E, N, R, X, S, I
const bomm_letter_t _plug_order[] = {
     4, 13, 17, 23, 18,  8,  0,  1,  2,  3,
     5,  6,  7,  9, 10, 11, 12, 14, 15, 16,
    19, 20, 21, 22, 24, 25
};

float bomm_attack_plugboard(
    bomm_letter_t* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
) {
    unsigned int i, j, a, b, best_b;
    float score, best_score;

    // Reset plugboard
    memcpy(plugboard, &bomm_key_plugboard_identity, sizeof(bomm_letter_t) * BOMM_ALPHABET_SIZE);

    // Score empty plugboard
    best_score = bomm_measure_scrambler_ngram(3, scrambler, plugboard, ciphertext);

    // Enumerate over the first plug
    for (i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        a = _plug_order[i];

        // Skip if plug is already in use
        if (plugboard[a] != a) {
            continue;
        }

        // Start with the self-steckered right letter and the score from the
        // initialization or previous iteration
        best_b = a;

        // Enumerate over the second plug
        for (j = i; j < BOMM_ALPHABET_SIZE; j++) {
            b = _plug_order[j];

            // Skip if plug is already in use
            if (plugboard[b] != b) {
                continue;
            }

            // Apply plug
            plugboard[a] = b;
            plugboard[b] = a;

            // Measure score and compare it to the previous best score
            score = bomm_measure_scrambler_ngram(3, scrambler, plugboard, ciphertext);
            if (score > best_score) {
                best_score = score;
                best_b = b;
            }

            // Undo plug
            plugboard[a] = a;
            plugboard[b] = b;
        }

        // Choose best option to go forward
        plugboard[a] = best_b;
        plugboard[best_b] = a;
    }

    return best_score;
}
