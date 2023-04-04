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
    int i, j, k;
    bool carry, relevant;
    char key_string[128];
    float score;
    float min_score = -INFINITY;
    time_t start_time, time;
    int slot_count = model->slot_count;
    int wheel_order_count;

    // Load ngram map
    bomm_ngram_map_t* ngram_map = bomm_measure_ngram_map_alloc(3, "/Users/ff/Projects/Bachelor/bomm/data/enigma1941-trigram.txt");

    // Prepare leaderboard
    bomm_key_leaderboard_t* leaderboard = bomm_key_leaderboard_alloc(40);

    // Prepare preview text
    char preview[32];

    // Prepare working key instance
    bomm_key_t key;
    memset(&key, 0, sizeof(bomm_key_t));
    key.model = model;

    // Prepare scrambler
    char scrambler_store[bomm_scrambler_size(ciphertext->length)];
    bomm_scrambler_t *scrambler = (bomm_scrambler_t*) &scrambler_store;
    scrambler->length = ciphertext->length;

    // Prepare initial ring settings and shifting ring masks
    bomm_lettermask_t slot_shifting_ring_masks[slot_count];
    memcpy(&slot_shifting_ring_masks, model->ring_masks, sizeof(bomm_lettermask_t) * slot_count);
    _enum_lettermask_init(slot_count, key.rings, slot_shifting_ring_masks);

    // Prepare initial start positions and shifting position masks
    bomm_lettermask_t slot_shifting_position_masks[slot_count];
    memcpy(&slot_shifting_position_masks, model->position_masks, sizeof(bomm_lettermask_t) * slot_count);
    _enum_lettermask_init(slot_count, key.positions, slot_shifting_position_masks);

    // Start the timer
    start_time = clock();
    wheel_order_count = 0;

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
            // Print progress update
            bomm_key_serialize_wheel_order(key_string, 128, &key);
            printf("Next wheel order: %s\n", key_string);
            time = clock();
            printf("Total time elapsed: %.1fs\n", (double) (time - start_time) / CLOCKS_PER_SEC);
            printf("Average time per wheel order: %.1fs\n", (double) (time - start_time) / CLOCKS_PER_SEC / wheel_order_count);

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
                        // Generate scrambler
                        bomm_scrambler_generate(scrambler, &key);

                        // Attack ciphertext
                        score = bomm_attack_phase_2(key.plugboard, scrambler, ciphertext, ngram_map);

                        if (score > min_score) {
                            // Generate preview
                            for (k = 0; k < BOMM_KEY_SCORED_PREVIEW_SIZE - 1 && k < (int) ciphertext->length; k++) {
                                preview[k] = bomm_message_letter_to_ascii(key.plugboard[scrambler->map[k][key.plugboard[ciphertext->letters[k]]]]);
                            }
                            preview[k] = '\0';

                            // TODO: Render a short preview for the leaderboard
                            min_score = bomm_key_leaderboard_add(leaderboard, &key, score, preview);

                            // Print updated leaderboard
                            printf("\n");
                            printf("Leaderboard:\n");
                            bomm_key_leaderboard_print(leaderboard);
                        }
                    }
                } while (!_enum_lettermask(slot_count, key.positions, slot_shifting_position_masks));
            } while (!_enum_lettermask(slot_count, key.rings, slot_shifting_ring_masks));
            wheel_order_count++;
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

/**
 * Score the given ciphertext using n-gram.
 * @param n 1 for unigram, 2 for bigram, 3 for bigram, etc.
 * @param scrambler Scrambler maps
 * @param plugboard Plugboard mapping
 * @param ciphertext Ciphertext message to map and score
 * @param ngram_map Frequency map
 * @return Score
 */
static inline float _measure_scrambler_ngram(
    unsigned int n,
    bomm_scrambler_t* scrambler,
    bomm_letter_t* plugboard,
    bomm_message_t* ciphertext,
    bomm_ngram_map_t* ngram_map
) {
    unsigned int map_size = pow(BOMM_ALPHABET_SIZE, n);
    unsigned int index, letter;

    float score = 0;
    unsigned int map_index = 0;

    for (index = 0; index < ciphertext->length; index++) {
        letter = ciphertext->letters[index];
        letter = plugboard[letter];
        letter = scrambler->map[index][letter];
        letter = plugboard[letter];

        map_index = (map_index * BOMM_ALPHABET_SIZE + letter) % map_size;
        
        if (index >= n - 1) {
            score += ngram_map->map[map_index];
        }
    }

    return score / (ciphertext->length - n + 1);
}

/*
static inline float _measure_scrambler_ic(
    bomm_scrambler_t* scrambler,
    bomm_letter_t* plugboard,
    bomm_message_t* ciphertext
) {
    unsigned int index, letter;
    unsigned int letter_frequencies[BOMM_ALPHABET_SIZE];
    memset(letter_frequencies, 0, BOMM_ALPHABET_SIZE * sizeof(unsigned int));
    
    for (index = 0; index < ciphertext->length; index++) {
        letter = ciphertext->letters[index];
        letter = plugboard[letter];
        letter = scrambler->map[index][letter];
        letter = plugboard[letter];
        letter_frequencies[letter]++;
    }
    
    return bomm_measure_ic(letter_frequencies, ciphertext->length, BOMM_ALPHABET_SIZE);
}
*/

// During the hillclimb we exhaust the following plugs in order
// The I-Stecker strategy starts with E, N, R, X, S, I
const bomm_letter_t _plug_order[] = {
     4, 13, 17, 23, 18,  8,  0,  1,  2,  3,
     5,  6,  7,  9, 10, 11, 12, 14, 15, 16,
    19, 20, 21, 22, 24, 25
};

float bomm_attack_phase_2(
    bomm_letter_t* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext,
    bomm_ngram_map_t* ngram_map
) {
    unsigned int i, j, a, b, best_b;
    float score, best_score;

    // Reset plugboard
    memcpy(plugboard, &bomm_key_plugboard_identity, sizeof(bomm_letter_t) * BOMM_ALPHABET_SIZE);

    // Score empty plugboard
    best_score = _measure_scrambler_ngram(3, scrambler, plugboard, ciphertext, ngram_map);
    // best_score = _measure_scrambler_ic(scrambler, plugboard, ciphertext);

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
            score = _measure_scrambler_ngram(3, scrambler, plugboard, ciphertext, ngram_map);
            // score = _measure_scrambler_ic(scrambler, plugboard, ciphertext);
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
