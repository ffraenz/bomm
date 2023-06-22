//
//  hill_climb.c
//  Bomm
//
//  Created by Fränz Friederes on 23/05/2023.
//

#include "hill_climb.h"
#include "../utility.h"

double bomm_pass_hill_climb_run(
    bomm_pass_hill_climb_config_t* config,
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
    // 0x1 | Swap 01__
    // 0x2 | Swap 0_2_
    // 0x3 | Swap 0__3
    // 0x6 | Swap _12_
    // 0x7 | Swap _1_3
    // 0xb | Swap __23
    // 0xf | Evaluate improvement (Removing/adding)

    // Case 1: Both plugs are self-steckered
    //
    // Legend: `/` ignored, `*` self-steckered, `AA` steckered with each other
    //
    // N | Con | Act | Description
    // - | --- | --- | -----------
    // 0 |  +1 | 0x6 | Swap /**/ -> /AA/
    // 1 |   0 | 0xf | Evaluate improvement
    // 2 |  -1 | 0x6 | Swap /AA/ -> /**/
    // 3 |   0 | 0x0 | End of array (original plugboard restored)
    const unsigned char case_1_actions[] = {
        0x16, 0x0f, 0x26, 0x00
    };

    // Case 1a: Both plugs form a steckered pair
    //
    // N | Con | Act | Description
    // - | --- | --- | -----------
    // 0 |  -1 | 0x6 | Swap /AA/ -> /**/
    // 1 |   0 | 0xf | Evaluate improvement
    // 2 |  +1 | 0x6 | Swap /**/ -> /AA/
    // 3 |   0 | 0x0 | End of array (original plugboard restored)
    const unsigned char case_1a_actions[] = {
        0x26, 0x0f, 0x16, 0x00
    };

    // Case 2: The first plug is self-steckered and the second one is steckered
    //
    // N | Con | Act | Description
    // - | --- | --- | -----------
    // 0 |  -1 | 0xb | Swap /*AA -> /***
    // 1 |  +1 | 0x6 | Swap /*** -> /AA*
    // 2 |   0 | 0xf | Evaluate improvement (KZ_IK)
    // 3 |  -1 | 0x6 | Swap /AA* -> /***
    // 4 |  +1 | 0x7 | Swap /*** -> /A*A
    // 5 |   0 | 0xf | Evaluate improvement (KZ_IZ)
    // 6 |  -1 | 0x7 | Swap /A*A -> /***
    // 7 |  +1 | 0xb | Swap /*** -> /*AA
    // 8 |   0 | 0x0 | End of array (original plugboard restored)
    const unsigned char case_2_actions[] = {
        0x2b, 0x16, 0x0f, 0x26, 0x17, 0x0f, 0x27, 0x1b, 0x00
    };

    // Case 3: Both plugs are steckered separately
    //
    //  N | Con | Act | Description
    // -- | --- | --- | -----------
    //  0 |  -1 | 0x1 | Swap AABB -> **AA
    //  1 |  -1 | 0xb | Swap **AA -> ****
    //  2 |  +1 | 0x6 | Swap **** -> *AA*
    //  3 |   0 | 0xf | Evaluate improvement (IXKZ_IK)
    //  4 |  +1 | 0x3 | Swap *AA* -> BAAB
    //  5 |   0 | 0xf | Evaluate improvement (IXKZ_IKXZ)
    //  6 |  -1 | 0x3 | Swap BAAB -> *AA*
    //  7 |  -1 | 0x6 | Swap *AA* -> ****
    //  8 |  +1 | 0x7 | Swap **** -> *A*A
    //  9 |   0 | 0xf | Evaluate improvement (IXKZ_IZ)
    // 10 |  +1 | 0x2 | Swap *A*A -> BABA
    // 11 |   0 | 0xf | Evaluate improvement (IXKZ_IZXK)
    // 12 |  -1 | 0x2 | Swap BABA -> *A*A
    // 13 |  -1 | 0x7 | Swap *A*A -> ****
    // 14 |  +1 | 0xb | Swap **** -> **AA
    // 15 |  +1 | 0x1 | Swap **AA -> AABB
    // 16 |   0 | 0x0 | End of array (original plugboard restored)
    const unsigned char case_3_actions[] = {
        0x21, 0x2b, 0x16, 0x0f, 0x13, 0x0f, 0x23, 0x26, 0x17, 0x0f,
        0x12, 0x0f, 0x22, 0x27, 0x1b, 0x11, 0x00
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
    unsigned int num_plugs = 0;

    unsigned int backtracking_min_num_plugs = config->backtracking_min_num_plugs;
    bomm_measure_t measure = config->measure;
    bomm_measure_t last_measure = BOMM_MEASURE_NONE;

    bool found_improvement = true;
    while (found_improvement) {
        // Check if the measure should be switched
        if (
            measure == config->measure &&
            num_plugs >= config->final_measure_min_num_plugs
        ) {
            measure = config->final_measure;
        }

        // Take an initial measurement, if the measure changes
        if (measure != last_measure) {
            last_measure = measure;
            best_score = bomm_measure_scrambler(
                measure,
                scrambler,
                plugboard,
                ciphertext
            );

            // printf("Switch to measure %d\n", measure);

            // TODO: Remove debugging code
            // bomm_wiring_plugboard_stringify(string, sizeof(string), plugboard);
            // printf("Initial me.: %+016.12f '%s' (%d)\n", best_score, string, num_plugs);
        }

        // Enumerate all possible plugboard pairs
        best_actions_begin = NULL;
        best_actions_end = NULL;
        found_improvement = false;
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
                } else if (num_plugs < backtracking_min_num_plugs) {
                    // Removing plugs is not allowed, yet.
                    continue;
                } else if (plugs[0] == plugs[2] && plugs[1] == plugs[3]) {
                    // `i` and `k` form a steckered pair
                    actions_begin = case_1a_actions;
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
                for (action = actions_begin; *action != 0x00; action++) {
                    // The two least significant bits signify the first plug
                    // and the next two bits the second plug to be swapped
                    bomm_swap(plugs[*action & 0x3], plugs[(*action >> 2) & 0x3]);
                    num_plugs += (*action & 0x20) == 0x20 ? -1 : (*action >> 4);

                    if (*action == 0x0f) {
                        // Take a measurement and compare it
                        score = bomm_measure_scrambler(
                            measure,
                            scrambler,
                            plugboard,
                            ciphertext
                        );

                        if (score > best_score) {
                            best_score = score;
                            found_improvement = true;

                            // TODO: Remove debugging code
                            // bomm_wiring_plugboard_stringify(string, sizeof(string), plugboard);
                            // printf("Measure:     %+016.12f '%s' (%d)\n", score, string, num_plugs);

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

        if (best_actions_end != NULL) {
            // Choose the best performing result for all pairs
            for (action = best_actions_begin; action < best_actions_end; action++) {
                bomm_swap(best_plugs[*action & 0x3], best_plugs[(*action >> 2) & 0x3]);
                num_plugs += (*action & 0x20) == 0x20 ? -1 : (*action >> 4);
            }
            best_actions_begin = NULL;
            best_actions_end = NULL;

            // TODO: Remove debugging code
            // bomm_wiring_plugboard_stringify(string, sizeof(string), plugboard);
            // printf("Apply best:  %+016.12f '%s' (%d)\n", best_score, string, num_plugs);
        }
    }

    if (measure == config->final_measure) {
        return best_score;
    }

    // The threshold for the final measure may not be reached
    return bomm_measure_scrambler(
        config->final_measure,
        scrambler,
        plugboard,
        ciphertext
    );
}

bomm_pass_hill_climb_config_t* bomm_pass_hill_climb_config_init_with_json(
    bomm_pass_hill_climb_config_t* config,
    json_t* config_json
) {
    json_t* val_json;

    // Config defaults
    bomm_pass_hill_climb_config_t working_config;
    working_config.measure = BOMM_MEASURE_IC;
    working_config.final_measure = BOMM_MEASURE_SINKOV_TRIGRAM;
    working_config.final_measure_min_num_plugs =
        (unsigned int) floor(BOMM_ALPHABET_SIZE * 0.3);
    working_config.backtracking_min_num_plugs =
        (unsigned int) floor(BOMM_ALPHABET_SIZE * 0.2);

    if (config_json->type != JSON_OBJECT) {
        return NULL;
    }

    if ((val_json = json_object_get(config_json, "measure"))) {
        working_config.measure = bomm_measure_from_json(val_json);
        if (working_config.measure == BOMM_MEASURE_NONE) {
            return NULL;
        }
    }

    if ((val_json = json_object_get(config_json, "finalMeasure"))) {
        working_config.final_measure = bomm_measure_from_json(val_json);
        if (working_config.final_measure == BOMM_MEASURE_NONE) {
            return NULL;
        }
    } else {
        working_config.final_measure = working_config.measure;
    }

    if ((val_json = json_object_get(config_json, "finalMeasureMinNumPlugs"))) {
        if (val_json->type == JSON_INTEGER) {
            long long val = json_integer_value(val_json);
            if (val >= 0 && val <= BOMM_ALPHABET_SIZE / 2) {
                working_config.final_measure_min_num_plugs = (unsigned int) val;
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
    }

    if ((val_json = json_object_get(config_json, "backtrackingMinNumPlugs"))) {
        if (val_json->type == JSON_INTEGER) {
            long long val = json_integer_value(val_json);
            if (val >= 0 && val <= BOMM_ALPHABET_SIZE / 2) {
                working_config.backtracking_min_num_plugs = (unsigned int) val;
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
    }

    if (!config && !(config = malloc(sizeof(working_config)))) {
        return NULL;
    }

    memcpy(config, &working_config, sizeof(working_config));
    return config;
}
