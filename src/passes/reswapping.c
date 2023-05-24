//
//  reswapping.c
//  Bomm
//
//  Created by Fränz Friederes on 23/05/2023.
//

#include "reswapping.h"
#include "../utility.h"

double bomm_pass_reswapping_run(
    bomm_pass_reswapping_config_t* config,
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
) {
    bomm_measure_t measure = config->measure;

    double score;
    double best_score = bomm_measure_scrambler(
        measure, scrambler, plugboard, ciphertext);

    unsigned int i, k, x;
    unsigned int best_reswap[4];

    bool found_improvement = true;
    while (found_improvement) {
        found_improvement = false;

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
                        score = bomm_measure_scrambler(
                            measure, scrambler, plugboard, ciphertext);
                        if (score > best_score) {
                            best_score = score;
                            best_reswap[0] = i;
                            best_reswap[1] = k;
                            best_reswap[2] = i;
                            best_reswap[3] = x;
                            found_improvement = true;
                        }
                        bomm_swap(&plugboard[i], &plugboard[x]);

                        // Measure stecker k, x
                        bomm_swap(&plugboard[k], &plugboard[x]);
                        score = bomm_measure_scrambler(
                            measure, scrambler, plugboard, ciphertext);
                        if (score > best_score) {
                            best_score = score;
                            best_reswap[0] = i;
                            best_reswap[1] = k;
                            best_reswap[2] = k;
                            best_reswap[3] = x;
                            found_improvement = true;
                        }
                        bomm_swap(&plugboard[k], &plugboard[x]);
                    }
                }

                // Add stecker i, k
                bomm_swap(&plugboard[i], &plugboard[k]);
            }
        }

        // Apply best scoring reswap, if any
        if (found_improvement) {
            bomm_swap(&plugboard[best_reswap[0]], &plugboard[best_reswap[1]]);
            bomm_swap(&plugboard[best_reswap[2]], &plugboard[best_reswap[3]]);
        }
    }

    return best_score;
}

bomm_pass_reswapping_config_t* bomm_pass_reswapping_config_init_json(
    bomm_pass_reswapping_config_t* config,
    json_t* config_json
) {
    json_t* val_json;

    // Config defaults
    bomm_pass_reswapping_config_t working_config;
    working_config.measure = BOMM_MEASURE_IC;

    if (config_json->type != JSON_OBJECT) {
        return NULL;
    }

    if ((val_json = json_object_get(config_json, "measure"))) {
        working_config.measure = bomm_measure_from_json(val_json);
        if (working_config.measure == BOMM_MEASURE_NONE) {
            return NULL;
        }
    }

    if (!config && !(config = malloc(sizeof(working_config)))) {
        return NULL;
    }

    memcpy(config, &working_config, sizeof(working_config));
    return config;
}
