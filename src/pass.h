//
//  pass.h
//  Bomm
//
//  Created by Fränz Friederes on 23/05/2023.
//

#ifndef pass_h
#define pass_h

#include "passes/hill_climb.h"
#include "passes/reswapping.h"

/**
 * Union of all possible pass config structs
 */
typedef union _bomm_pass_config {
    /**
     * Hill climb pass config
     */
    bomm_pass_hill_climb_config_t hill_climb;

    /**
     * Reswapping pass config
     */
    bomm_pass_reswapping_config_t reswapping;
} bomm_pass_config_t;

/**
 * Enum identifying a pass
 */
typedef enum {
    BOMM_PASS_NONE = 0,
    BOMM_PASS_HILL_CLIMB,
    BOMM_PASS_RESWAPPING
} bomm_pass_type_t;

/**
 * Struct identifying a pass and configuring it
 */
typedef struct _bomm_pass {
    /**
     * Pass type
     */
    bomm_pass_type_t type;

    /**
     * Pass config
     */
    bomm_pass_config_t config;
} bomm_pass_t;

/**
 * Run a pass on the given plugboard and scrambler
 */
inline static double bomm_pass_run(
    bomm_pass_t* pass,
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext
) {
    switch (pass->type) {
        case BOMM_PASS_HILL_CLIMB: {
            return bomm_pass_hill_climb_run(
                &pass->config.hill_climb,
                plugboard,
                scrambler,
                ciphertext
            );
        }
        case BOMM_PASS_RESWAPPING: {
            return bomm_pass_reswapping_run(
                &pass->config.reswapping,
                plugboard,
                scrambler,
                ciphertext
            );
        }
        case BOMM_PASS_NONE: {
            return 0;
        }
    }
}

/**
 * Return the measure returned by the given pass
 */
inline static bomm_measure_t bomm_pass_result_measure(bomm_pass_t* pass) {
    switch (pass->type) {
        case BOMM_PASS_HILL_CLIMB: {
            return pass->config.hill_climb.final_measure;
        }
        case BOMM_PASS_RESWAPPING: {
            return pass->config.reswapping.measure;
        }
        case BOMM_PASS_NONE: {
            return BOMM_MEASURE_NONE;
        }
    }
}

/**
 * Init the default pass: A hill climb using the IC measure for the first
 * 5 plugs and Sinkov's trigram measure for the remaining ones. Backtracking
 * is allowed after the first 5 plugs have been allocated.
 */
bomm_pass_t* bomm_pass_init(bomm_pass_t* pass);

/**
 * Init a pass from the given JSON value
 */
bomm_pass_t* bomm_pass_init_json(
    bomm_pass_t* pass,
    json_t* pass_json
);

#endif /* pass_h */
