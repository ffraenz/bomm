//
//  pass.c
//  Bomm
//
//  Created by Fränz Friederes on 23/05/2023.
//

#include "pass.h"

bomm_pass_t* bomm_pass_init(bomm_pass_t* pass) {
    if (!pass && !(pass = malloc(sizeof(bomm_pass_t)))) {
        return NULL;
    }
    pass->type = BOMM_PASS_HILL_CLIMB;
    pass->config.hill_climb.measure = BOMM_MEASURE_IC;
    pass->config.hill_climb.final_measure = BOMM_MEASURE_SINKOV_TRIGRAM;
    pass->config.hill_climb.final_measure_min_num_plugs = 5;
    pass->config.hill_climb.backtracking_min_num_plugs = 5;
    return pass;
}

bomm_pass_t* bomm_pass_init_json(
    bomm_pass_t* pass,
    json_t* pass_json
) {
    json_t* val_json;

    if (pass_json->type != JSON_OBJECT) {
        return NULL;
    }

    const char* type;
    if ((val_json = json_object_get(pass_json, "type"))) {
        if (val_json->type == JSON_STRING) {
            type = json_string_value(val_json);
        }
    }

    bool error = false;
    bomm_pass_t new_pass;
    if (strcmp(type, "hill_climb") == 0) {
        new_pass.type = BOMM_PASS_HILL_CLIMB;
        error = NULL == bomm_pass_hill_climb_config_init_json(
            &new_pass.config.hill_climb,
            pass_json
        );
    } else if (strcmp(type, "reswapping") == 0) {
        new_pass.type = BOMM_PASS_RESWAPPING;
        error = NULL == bomm_pass_reswapping_config_init_json(
            &new_pass.config.reswapping,
            pass_json
        );
    } else {
        error = true;
    }

    if (error) {
        return NULL;
    }

    if (!pass && !(pass = malloc(sizeof(new_pass)))) {
        return NULL;
    }

    memcpy(pass, &new_pass, sizeof(new_pass));
    return pass;
}
