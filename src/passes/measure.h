//
//  measure.h
//  Bomm
//
//  Created by Fränz Friederes on 06/07/2023.
//

#ifndef passes_measure_h
#define passes_measure_h

#include "../measure.h"

/**
 * Struct representing a set of values configuring a measure pass
 */
typedef struct _bomm_pass_measure_config {
    /**
     * Measure
     */
    bomm_measure_t measure;
} bomm_pass_measure_config_t;

/**
 * Run a measure pass on the given plugboard and scrambler
 */
inline static double bomm_pass_measure_climb_run(
    bomm_pass_measure_config_t* config,
    bomm_plugboard_t* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext,
    unsigned int* num_decrypts
) {
    bomm_measure_t measure = config->measure;
    (*num_decrypts)++;
    return bomm_measure_scrambler(measure, scrambler, plugboard, ciphertext);
}

/**
 * Init a pass config from the given JSON value
 */
bomm_pass_measure_config_t* bomm_pass_measure_config_init_with_json(
    bomm_pass_measure_config_t* config,
    json_t* config_json
);

#endif /* passes_measure_h */
