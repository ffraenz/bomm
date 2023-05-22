//
//  attack.c
//  Bomm
//
//  Created by Fränz Friederes on 20/03/2023.
//

#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE

#include "attack.h"

void* bomm_attack_thread(void* arg) {
    // The argument is assumed to be an attack
    bomm_attack_t* attack = (bomm_attack_t*) arg;
    bomm_attack_key_space(attack);
    return NULL;
}

void bomm_attack_key_space(bomm_attack_t* attack) {
    bool cancelling = false;
    double score;
    double min_score = -INFINITY;
    unsigned int working_plugboard[BOMM_ALPHABET_SIZE];
    char hold_preview[BOMM_HOLD_PREVIEW_SIZE];

    // Allocate messages on the stack
    size_t message_size = bomm_message_size_for_length(attack->ciphertext->length);
    bomm_message_t *plaintext = alloca(message_size);
    bomm_message_t *ciphertext = alloca(message_size);
    memcpy(ciphertext, attack->ciphertext, message_size);

    // Allocate scrambler on the stack
    bomm_scrambler_t *scrambler = alloca(bomm_scrambler_size(ciphertext->length));
    scrambler->length = ciphertext->length;

    const bomm_measure_t measures[] = {
        BOMM_MEASURE_IC,
        BOMM_MEASURE_SINKOV_TRIGRAM,
        0
    };

    double start_timestamp = bomm_timestamp_sec();
    double batch_start_timestamp = start_timestamp;
    double batch_duration_sec;

    unsigned long key_space_size = bomm_key_space_count(&attack->key_space);
    unsigned int batch_count = 0;
    unsigned int batch_size = 26 * 26 * 8;

    bomm_key_iterator_t key_iterator;
    if (bomm_key_iterator_init(&key_iterator, &attack->key_space) == NULL) {
        // Key space is empty
        return;
    }

    // Initial progress update
    pthread_mutex_lock(&attack->mutex);
    attack->progress.batch_unit_size = batch_size;
    attack->progress.unit_count = key_space_size;
    attack->progress.completed_unit_count = 0;
    attack->progress.batch_duration_sec = 0;
    attack->progress.duration_sec = 0;
    pthread_mutex_unlock(&attack->mutex);

    // Iterate over keys in the key space
    do {
        if (key_iterator.scrambler_changed) {
            bomm_enigma_generate_scrambler(scrambler, &key_iterator.key);

            // TODO: Remove debugging code
            // bomm_key_debug(&key_iterator.key);
        }

        // Make a working copy of the plugboard
        memcpy(working_plugboard, key_iterator.key.plugboard, sizeof(working_plugboard));

        // Attack ciphertext
        score = bomm_attack_plugboard_hill_climb(
            BOMM_ATTACK_PLUGBOARD_BEST_IMPROVEMENT,
            measures,
            working_plugboard,
            scrambler,
            ciphertext
        );

        // Add key and plugboard to the hold, if it scores good enough
        if (score > min_score) {
            bomm_scrambler_encrypt(scrambler, working_plugboard, ciphertext, plaintext);
            bomm_message_stringify(hold_preview, BOMM_HOLD_PREVIEW_SIZE, plaintext);

            bomm_key_t key;
            memcpy(&key, &key_iterator.key, sizeof(key));
            memcpy(key.plugboard, working_plugboard, sizeof(working_plugboard));
            min_score = bomm_hold_add(attack->query->hold, score, &key, hold_preview);
        }

        // Report the progress every time a batch has been finalized
        if (++batch_count >= batch_size) {
            // Measure time
            batch_duration_sec = batch_start_timestamp;
            batch_start_timestamp = bomm_timestamp_sec();
            batch_duration_sec = batch_start_timestamp - batch_duration_sec;

            // Intermediate progress update
            pthread_mutex_lock(&attack->mutex);
            attack->progress.completed_unit_count += batch_count;
            attack->progress.duration_sec = batch_start_timestamp - start_timestamp;
            attack->progress.batch_duration_sec = batch_duration_sec;
            cancelling = attack->state == BOMM_ATTACK_STATE_CANCELLING;
            pthread_mutex_unlock(&attack->mutex);

            // Reset batch count
            batch_count = 0;
        }
    } while (!cancelling && !bomm_key_iterator_next(&key_iterator));

    // Final progress update
    pthread_mutex_lock(&attack->mutex);
    attack->progress.completed_unit_count += batch_count;
    attack->state = cancelling ? BOMM_ATTACK_STATE_CANCELLED : BOMM_ATTACK_STATE_COMPLETED;
    pthread_mutex_unlock(&attack->mutex);
}

double bomm_attack_plugboard_hill_climb(
    bomm_attack_plugboard_strategy_t strategy,
    const bomm_measure_t* measures,
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
) {
    // Action values encode swap operations that can be applied to a set of
    // 4 plugs (the chosen pair and up to two letters that may be connected to
    // them). The special value `0xf` marks when improvement should be
    // evaluated. Finally, 0x0 marks the end of the array. Every set of actions
    // below apply valid swap operations on the 4 plugs and make sure the
    // original arrangement is restored at the end of the array.
    //
    // Act | Description
    // --- | -----------
    // 0x0 | End of array
    // 0xb | Swap __23
    // 0x7 | Swap _1_3
    // 0x6 | Swap _12_
    // 0x3 | Swap 0__3
    // 0x2 | Swap 0_2_
    // 0x1 | Swap 01__
    // 0xf | Evaluate improvement

    // Case 1: Both plugs are either self-steckered or form a steckered pair
    //
    // Legend: `/` ignored, `*` self-steckered, `AA` steckered with each other
    //
    // N | Act | Description
    // - | --- | -----------
    // 0 | 0x6 | Swap /AA/ -> /**/ or /**/ -> /AA/
    // 1 | 0xf | Evaluate improvement
    // 2 | 0x6 | Swap /**/ -> /AA/ or /AA/ -> /**/
    // 3 | 0x0 | End of array (original plugboard restored)
    const unsigned char case_1_actions[] = {
        0x6, 0xf, 0x6, 0x0
    };

    // Case 2: The first plug is self-steckered and the second one is steckered
    //
    // N | Act | Description
    // - | --- | -----------
    // 0 | 0xb | Swap /*AA -> /***
    // 1 | 0x6 | Swap /*** -> /AA*
    // 2 | 0xf | Evaluate improvement (KZ_IK)
    // 2 | 0x6 | Swap /AA* -> /***
    // 3 | 0x7 | Swap /*** -> /A*A
    // 4 | 0xf | Evaluate improvement (KZ_IZ)
    // 5 | 0x7 | Swap /A*A -> /***
    // 6 | 0xb | Swap /*** -> /*AA
    // 7 | 0x0 | End of array (original plugboard restored)
    const unsigned char case_2_actions[] = {
        0xb, 0x6, 0xf, 0x6, 0x7, 0xf, 0x7, 0xb, 0x0
    };

    // Case 3: Both plugs are steckered separately
    //
    //  N | Act | Description
    // -- | --- | -----------
    //  0 | 0x1 | Swap AABB -> **AA
    //  1 | 0xb | Swap **AA -> ****
    //  2 | 0x6 | Swap **** -> *AA*
    //  3 | 0xf | Evaluate improvement (IXKZ_IK)
    //  4 | 0x3 | Swap *AA* -> BAAB
    //  5 | 0xf | Evaluate improvement (IXKZ_IKXZ)
    //  6 | 0x3 | Swap BAAB -> *AA*
    //  7 | 0x6 | Swap *AA* -> ****
    //  8 | 0x7 | Swap **** -> *A*A
    //  9 | 0xf | Evaluate improvement (IXKZ_IZ)
    // 10 | 0x2 | Swap *A*A -> BABA
    // 11 | 0xf | Evaluate improvement (IXKZ_IZXK)
    // 12 | 0x2 | Swap BABA -> *A*A
    // 13 | 0x7 | Swap *A*A -> ****
    // 14 | 0xb | Swap **** -> **AA
    // 15 | 0x1 | Swap **AA -> AABB
    // 16 | 0x0 | End of array (original plugboard restored)
    const unsigned char case_3_actions[] = {
        0x1, 0xb, 0x6, 0xf, 0x3, 0xf, 0x3, 0x6, 0x7, 0xf,
        0x2, 0xf, 0x2, 0x7, 0xb, 0x1, 0x0
    };

    const unsigned char* action;
    const unsigned char* actions_begin;

    // TODO: Remove debugging code
    // char string[128];

    // Set of plugs and set of actions needed to recreate the best result
    unsigned int* best_plugs[4];
    const unsigned char* best_actions_begin;
    const unsigned char* best_actions_end;

    double best_score = 0;
    double score;

    // Enumerate measures
    const bomm_measure_t* measure = measures;
    const bomm_measure_t* last_measure = NULL;
    while (*measure != 0) {
        best_actions_begin = NULL;
        best_actions_end = NULL;
        bool found_improvement = false;

        // Take an initial measurement when switching the measure
        if (measure != last_measure) {
            last_measure = measure;
            best_score = bomm_measure_scrambler(
                *measure,
                scrambler,
                plugboard,
                ciphertext
            );

            // TODO: Remove debugging code
            // bomm_wiring_plugboard_stringify(string, sizeof(string), plugboard);
            // printf("Initial me.: %+016.12f '%s'\n", best_score, string);
        }

        // Enumerate all possible plugboard pairs
        for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
            for (unsigned int k = i + 1; k < BOMM_ALPHABET_SIZE; k++) {
                // "We need to consider the chosen pair of letters, and also
                // other letters that may already be connected to them",
                // SullivanWeierud2005, 198.

                // Selected plugs 4-tuple (`i` partner, `i`, `k`, `k` partner)
                unsigned int* plugs[4] = {
                    &plugboard[plugboard[i]],
                    &plugboard[i],
                    &plugboard[k],
                    &plugboard[plugboard[k]]
                };

                // Determine the set of actions applicable to the selected plugs
                if (plugs[0] == plugs[1] && plugs[2] == plugs[3]) {
                    // Both `i`, `k` are self-steckered
                    actions_begin = case_1_actions;
                } else if (plugs[0] == plugs[2] && plugs[1] == plugs[3]) {
                    // `i` and `k` form a steckered pair
                    actions_begin = case_1_actions;
                } else if (plugs[0] == plugs[1] && plugs[2] != plugs[3]) {
                    // `i` is self-steckered while `k` is not
                    actions_begin = case_2_actions;
                } else if (plugs[0] != plugs[1] && plugs[2] == plugs[3]) {
                    // `k` is self-steckered while `i` is not
                    // Swap such that `i` becomes self-steckered, reusing case 2
                    bomm_swap_pointer((void**) &plugs[1], (void**) &plugs[2]);
                    bomm_swap_pointer((void**) &plugs[0], (void**) &plugs[3]);
                    actions_begin = case_2_actions;
                } else {
                    // Both `i`, `k` are steckered separately
                    actions_begin = case_3_actions;
                }

                // TODO: Remove debugging code
                // unsigned int original_plugboard[BOMM_ALPHABET_SIZE];
                // memcpy(original_plugboard, plugboard, sizeof(original_plugboard));

                // Enumerate the set of actions
                for (action = actions_begin; *action != 0x0; action++) {
                    if (*action < 0xf) {
                        // The two least significant bits signify the first plug
                        // and the next two bits the second plug to be swapped
                        bomm_swap(plugs[*action & 0x03], plugs[*action >> 2]);
                    } else {
                        // Take a measurement and compare it
                        score = bomm_measure_scrambler(
                            *measure,
                            scrambler,
                            plugboard,
                            ciphertext
                        );

                        if (score > best_score) {
                            best_score = score;
                            found_improvement = true;

                            // TODO: Remove debugging code
                            // bomm_wiring_plugboard_stringify(string, sizeof(string), plugboard);
                            // printf("Measure:     %+016.12f '%s'\n", score, string);

                            if (strategy == BOMM_ATTACK_PLUGBOARD_FIRST_IMPROVEMENT) {
                                // Choose first improvement and immediately
                                // restart the plugboard pair enumeration
                                // Using goto to facilitate `break 3`
                                goto end_outer_loop;
                            }

                            // Store info necessary to reproduce best result
                            memcpy(best_plugs, plugs, sizeof(best_plugs));
                            best_actions_begin = actions_begin;
                            best_actions_end = action;
                        }
                    }
                }

                // TODO: Remove debugging code
                // if (memcmp(plugboard, original_plugboard, sizeof(original_plugboard)) != 0) {
                //     printf("Action set is not restoring original plugboard!\n");
                //     exit(1);
                // }
            }
        } // End: Enumerate all possible plugboard pairs

        if (best_actions_end != NULL && strategy == BOMM_ATTACK_PLUGBOARD_BEST_IMPROVEMENT) {
            // Choose the best performing result for all pairs
            for (action = best_actions_begin; action < best_actions_end; action++) {
                if (*action < 0xf) {
                    bomm_swap(best_plugs[*action & 0x03], best_plugs[*action >> 2]);
                }
            }
            best_actions_begin = NULL;
            best_actions_end = NULL;

            // TODO: Remove debugging code
            // bomm_wiring_plugboard_stringify(string, sizeof(string), plugboard);
            // printf("Apply best:  %+016.12f '%s'\n", best_score, string);
        }

end_outer_loop:
        if (!found_improvement) {
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
    float best_score = bomm_measure_scrambler_sinkov(3, scrambler, plugboard, ciphertext);
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
                        score = bomm_measure_scrambler_sinkov(3, scrambler, plugboard, ciphertext);
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
                        score = bomm_measure_scrambler_sinkov(3, scrambler, plugboard, ciphertext);
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
