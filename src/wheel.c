//
//  wheel.c
//  Bomm
//
//  Created by Fränz Friederes on 11/04/2023.
//

#include "wheel.h"

bomm_wheel_t* bomm_wheel_init(
    bomm_wheel_t* wheel,
    const char* name,
    const char* wiring_string,
    const char* turnovers_string
) {
    if (wheel == NULL) {
        wheel = malloc(sizeof(bomm_wheel_t));
        if (wheel == NULL) {
            return NULL;
        }
    }
    
    bomm_strncpy(wheel->name, name, BOMM_WHEEL_NAME_MAX_LENGTH);
    bomm_wiring_extract(&wheel->wiring, wiring_string);
    
    if (turnovers_string != NULL) {
        bomm_lettermask_extract(&wheel->turnovers, turnovers_string);
    } else {
        wheel->turnovers = BOMM_LETTERMASK_NONE;
    }
    
    return wheel;
}

bomm_wheel_t* bomm_wheel_extract_json(bomm_wheel_t* wheel, json_t* wheel_json) {
    if (wheel_json->type != JSON_OBJECT) {
        return NULL;
    }
    
    json_t* name_json = json_object_get(wheel_json, "name");
    if (name_json == NULL || name_json->type != JSON_STRING) {
        return NULL;
    }
    
    json_t* wiring_json = json_object_get(wheel_json, "wiring");
    if (wiring_json == NULL || wiring_json->type != JSON_STRING) {
        return NULL;
    }
    
    json_t* turnovers_json = json_object_get(wheel_json, "turnovers");
    if (turnovers_json != NULL && turnovers_json->type != JSON_STRING) {
        return NULL;
    }
    
    wheel = bomm_wheel_init(
        wheel,
        json_string_value(name_json),
        json_string_value(wiring_json),
        turnovers_json != NULL ? json_string_value(turnovers_json) : NULL
    );
    
    return wheel;
}
