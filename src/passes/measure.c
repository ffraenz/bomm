//
//  measure.c
//  Bomm
//
//  Created by Fränz Friederes on 06/07/2023.
//

#include "measure.h"

bomm_pass_measure_config_t* bomm_pass_measure_config_init_with_json(
    bomm_pass_measure_config_t* config,
    json_t* config_json
) {
    if (config_json->type != JSON_OBJECT) {
        return NULL;
    }

    bomm_measure_t measure = BOMM_MEASURE_IC;
    json_t* base_measure_json = json_object_get(config_json, "measure");
    if (base_measure_json != NULL) {
        measure = bomm_measure_from_json(base_measure_json);
        if (measure == BOMM_MEASURE_NONE) {
            return NULL;
        }
    }

    if (!config && !(config = malloc(sizeof(bomm_pass_measure_config_t)))) {
        return NULL;
    }

    config->measure = measure;
    return config;
}
