//
//  reswapping.h
//  Bomm
//
//  Created by Fränz Friederes on 23/05/2023.
//

#ifndef passes_reswapping_h
#define passes_reswapping_h

#include "../measure.h"
#include "../message.h"
#include "../wiring.h"

/**
 * Struct representing a set of values configuring a reswapping pass
 */
typedef struct _bomm_pass_reswapping_config {
    /**
     * Measure to be used
     */
    bomm_measure_t measure;
} bomm_pass_reswapping_config_t;

/**
 * Run a reswapping pass on the given plugboard and scrambler. Reverse
 * engineered from the reswapping pass implemented in the enigma-suite project.
 * @see https://www.bytereef.org/enigma-suite.html
 */
double bomm_pass_reswapping_run(
    bomm_pass_reswapping_config_t* config,
    bomm_plugboard_t* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext,
    unsigned int* num_decrypts
);

/**
 * Init a pass config from the given JSON value
 */
bomm_pass_reswapping_config_t* bomm_pass_reswapping_config_init_with_json(
    bomm_pass_reswapping_config_t* config,
    json_t* config_json
);

#endif /* passes_reswapping_h */
