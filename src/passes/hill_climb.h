//
//  hill_climb.h
//  Bomm
//
//  Created by Fränz Friederes on 23/05/2023.
//

#ifndef passes_hill_climb_h
#define passes_hill_climb_h

#include <jansson.h>
#include "../measure.h"
#include "../message.h"
#include "../wiring.h"

/**
 * Struct representing a set of values configuring a hill climb pass
 */
typedef struct _bomm_pass_hill_climb_config {
    /**
     * Measure to be used to evaluate the first plugs
     */
    bomm_measure_t measure;

    /**
     * Measure to be used to evaluate the last plugs
     */
    bomm_measure_t final_measure;

    /**
     * Minimum number of plugs that need to be assigned before switching
     * from the initial measure to the final measure
     */
    unsigned int final_measure_min_num_plugs;

    /**
     * Minimum number of plugs that need to be assigned before evaluating
     * backtracking steps (i.e. removing plugs)
     */
    unsigned int backtracking_min_num_plugs;
} bomm_pass_hill_climb_config_t;

/**
 * Run a hill climb pass on the given plugboard and scrambler
 */
double bomm_pass_hill_climb_run(
    bomm_pass_hill_climb_config_t* config,
    bomm_plugboard_t* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext,
    unsigned int* num_decrypts
);

/**
 * Init a pass config from the given JSON value
 */
bomm_pass_hill_climb_config_t* bomm_pass_hill_climb_config_init_with_json(
    bomm_pass_hill_climb_config_t* config,
    json_t* config_json
);

#endif /* passes_hill_climb_h */
