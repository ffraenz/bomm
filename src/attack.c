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
    
    // Copy ciphertext to the stack
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
                        score = bomm_attack_plugboard_enigma_suite(key.plugboard, scrambler, ciphertext);
                        
                        // Reswapping
                        score = bomm_attack_plugboard_enigma_suite_reswapping(key.plugboard, scrambler, ciphertext);

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
const unsigned int _plug_order[] = {
     4, 13, 17, 23, 18,  8,  0,  1,  2,  3,
     5,  6,  7,  9, 10, 11, 12, 14, 15, 16,
    19, 20, 21, 22, 24, 25
};

float bomm_attack_plugboard(
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
) {
    unsigned int i, j, a, b, best_b;
    float score, best_score;

    // Reset plugboard
    memcpy(plugboard, &bomm_key_plugboard_identity, sizeof(bomm_key_plugboard_identity));

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

float bomm_attack_plugboard_enigma_suite(
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
) {
    // Action encoding: The most significant hex digit signifies whether a
    // measurement should be taken after the swap (non-zero value) or not
    // (zero value). The remaining, least significant hex digit describes the
    // two plugs in the `plugs` 4-tuple to be swapped like so:
    //
    // Swap | Swap bin | 0123
    // ---- | -------- | ----
    //  0xb |    10 11 | __23
    //  0x7 |    01 11 | _1_3
    //  0x6 |    01 10 | _12_
    //  0x3 |    00 11 | 0__3
    //  0x2 |    00 10 | 0_2_
    //  0x1 |    00 01 | 01__
    
    // Case 1: Both plugs are either self-steckered or form a steckered pair
    //
    // Legend: / not relevant, * self-steckered, AA steckered with each other
    //
    // N | Action | 0123 | 0123 | Measure
    // - | ------ | ---- | ---- | -------
    // / |      / | /**/ | /AA/ | /
    // 0 |   0x16 | /AA/ | /**/ | Yes
    const unsigned char case_1_actions[] =
        { 0x16, 0 };
    
    // Case 2: The first plug is self-steckered and the second one is steckered
    //
    // N | Action | 0123 | Measure
    // - | ------ | ---- | -------
    // / |      / | /*AA | /
    // 0 |   0x0b | /*** | No
    // 1 |   0x16 | /AA* | Yes (KZ_IK)
    // 2 |   0x06 | /*** | No
    // 3 |   0x17 | /A*A | Yes (KZ_IZ)
    const unsigned char case_2_actions[] =
        { 0x0b, 0x16, 0x06, 0x17, 0 };
    
    // Case 3: Both plugs are steckered separately
    //
    // N | Action | 0123 | Measure
    // - | ------ | ---- | -------
    // / |      / | AABB | /
    // 0 |   0x01 | ABCC | No
    // 1 |   0x0b | **** | No
    // 2 |   0x16 | *AA* | Yes (IXKZ_IK)
    // 3 |   0x13 | BAAB | Yes (IXKZ_IKXZ)
    // 4 |   0x06 | B**B | No
    // 5 |   0x03 | **** | No
    // 6 |   0x17 | *A*A | Yes (IXKZ_IZ)
    // 7 |   0x12 | BABA | Yes (IXKZ_IZXK)
    const unsigned char case_3_actions[] =
        { 0x01, 0x0b, 0x16, 0x13, 0x06, 0x03, 0x17, 0x12, 0 };
    
    float best_score = 0;
    float score;
    
    const unsigned char* action;
    const unsigned char* best_action;
    const unsigned char* actions;
    
    // Reset plugboard
    memcpy(plugboard, &bomm_key_plugboard_identity, sizeof(bomm_key_plugboard_identity));
    
    // Enumerate measurements
    unsigned int measure = 0;
    unsigned int last_measure = -1;
    bool new_best_score = true;
    while (measure < 3) {
        new_best_score = false;
        
        // Take an initial measurement when switching the measure
        if (measure != last_measure) {
            last_measure = measure;
            best_score = bomm_scrambler_measure(measure, scrambler, plugboard, ciphertext);
        }
    
        // Enumerate all possible plugboard pairs exactly once
        for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
            for (unsigned int k = i + 1; k < BOMM_ALPHABET_SIZE; k++) {
                // Selected plugs 4-tuple (`i` partner, `i`, `k`, `k` partner)
                unsigned int* plugs[4];
                plugs[0] = &plugboard[plugboard[_plug_order[i]]]; // x
                plugs[1] = &plugboard[_plug_order[i]]; // i
                plugs[2] = &plugboard[_plug_order[k]]; // k
                plugs[3] = &plugboard[plugboard[_plug_order[k]]]; // z
                
                // Determine what case we are in
                if (
                    (plugs[1] == plugs[0] && plugs[2] == plugs[3]) ||
                    plugs[2] == plugs[0]
                ) {
                    actions = case_1_actions;
                } else if (
                    (plugs[1] == plugs[0] && plugs[2] != plugs[3]) ||
                    (plugs[2] == plugs[3] && plugs[1] != plugs[0])
                ) {
                    // Swap such that `i` becomes self-steckered
                    if (plugs[2] == plugs[3]) {
                        bomm_swap_pointer((void**) &plugs[1], (void**) &plugs[2]);
                        bomm_swap_pointer((void**) &plugs[0], (void**) &plugs[3]);
                    }
                    actions = case_2_actions;
                } else {
                    actions = case_3_actions;
                }
                
                // Set initial action pointer
                action = actions;
                best_action = actions - 1;
                
                while (*action != 0) {
                    // The two least significant bits signify the first plug and
                    // the next two bits the second plug to be swapped
                    bomm_swap(plugs[*action & 0x03], plugs[(*action >> 2) & 0x03]);
                    
                    if ((*action >> 4) != 0) {
                        // Take a measurement and compare it
                        score = bomm_scrambler_measure(measure, scrambler, plugboard, ciphertext);
                        if (score > best_score) {
                            best_score = score;
                            best_action = action;
                            new_best_score = true;
                        }
                    }
                    
                    action++;
                }
                
                // Rollback actions up to the one that scored best
                while (--action != best_action) {
                    bomm_swap(plugs[*action & 0x03], plugs[(*action >> 2) & 0x03]);
                }
                
                // When removing a plug, skip tests for reinsertion
                if (best_action == &case_1_actions[0]) {
                    continue;
                }
            }
        }
        
        if (!new_best_score) {
            measure++;
        }
    }
    
    return best_score;
}

float bomm_attack_plugboard_enigma_suite_reswapping(
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
) {
    float best_score = bomm_measure_scrambler_ngram(3, scrambler, plugboard, ciphertext);
    float score;
    
    unsigned int i, k, x;
    unsigned int best_reswap[4];
    
    bool new_best_score = true;
    while (new_best_score) {
        new_best_score = false;
        
        // Enumerate unique steckered pairs i, k in the plugboard
        for (i = 0; i < BOMM_ALPHABET_SIZE; i++) {
            if (plugboard[i] > i) {
                k = plugboard[i];
                
                // Remove stecker i, k
                bomm_swap(&plugboard[i], &plugboard[k]);
                
                // Enumerate self-steckered letters x
                for (x = 0; x < BOMM_ALPHABET_SIZE; x++) {
                    if (plugboard[x] == x) {
                        // Measure stecker i, x
                        bomm_swap(&plugboard[i], &plugboard[x]);
                        score = bomm_measure_scrambler_ngram(3, scrambler, plugboard, ciphertext);
                        if (score > best_score) {
                            best_score = score;
                            best_reswap[0] = i;
                            best_reswap[1] = k;
                            best_reswap[2] = i;
                            best_reswap[3] = x;
                            new_best_score = true;
                        }
                        bomm_swap(&plugboard[i], &plugboard[x]);
                        
                        // Measure stecker k, x
                        bomm_swap(&plugboard[k], &plugboard[x]);
                        score = bomm_measure_scrambler_ngram(3, scrambler, plugboard, ciphertext);
                        if (score > best_score) {
                            best_score = score;
                            best_reswap[0] = i;
                            best_reswap[1] = k;
                            best_reswap[2] = k;
                            best_reswap[3] = x;
                            new_best_score = true;
                        }
                        bomm_swap(&plugboard[k], &plugboard[x]);
                    }
                }
                
                // Add stecker i, k
                bomm_swap(&plugboard[i], &plugboard[k]);
            }
        }
        
        // Apply best scoring reswap, if any
        if (new_best_score) {
            bomm_swap(&plugboard[best_reswap[0]], &plugboard[best_reswap[1]]);
            bomm_swap(&plugboard[best_reswap[2]], &plugboard[best_reswap[3]]);
        }
    }
    
    return best_score;
}
