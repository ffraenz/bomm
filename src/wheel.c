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
    if (!wheel && !(wheel = malloc(sizeof(bomm_wheel_t)))) {
        return NULL;
    }

    bomm_strncpy(wheel->name, name, BOMM_WHEEL_NAME_MAX_LENGTH);
    bomm_wiring_init(&wheel->wiring, wiring_string);

    if (turnovers_string != NULL) {
        bomm_lettermask_from_string(&wheel->turnovers, turnovers_string);
    } else {
        wheel->turnovers = BOMM_LETTERMASK_NONE;
    }

    return wheel;
}

bomm_wheel_t* bomm_wheel_init_with_name(bomm_wheel_t* wheel, const char* name) {
    unsigned int index = 0;
    unsigned int num_kown_wheels =
        sizeof(bomm_known_wheels) / sizeof(bomm_known_wheels[0]);

    const bomm_wheel_spec_t* wheel_spec = NULL;
    while (!wheel_spec && index < num_kown_wheels) {
        if (strcmp(bomm_known_wheels[index].name, name) == 0) {
            wheel_spec = &bomm_known_wheels[index];
        } else {
            index++;
        }
    }

    if (!wheel_spec) {
        return NULL;
    }

    return bomm_wheel_init(wheel, name, wheel_spec->wiring, wheel_spec->turnovers);
}

bomm_wheel_t* bomm_wheel_init_with_json(bomm_wheel_t* wheel, json_t* wheel_json) {
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

bool bomm_wheel_set_init_with_json(
    bomm_wheel_t wheel_set[],
    unsigned int wheel_set_size,
    json_t* wheel_set_json,
    bomm_wheel_t wheels[],
    unsigned int wheel_count
) {
    if (wheel_set_json == NULL || wheel_set_json->type != JSON_ARRAY) {
        fprintf(
            stderr,
            "Error: Wheel set is expected to be an array of strings\n"
        );
        return false;
    }

    unsigned int size = (unsigned int) json_array_size(wheel_set_json);
    if (size > wheel_set_size) {
        fprintf(
            stderr,
            "Error: The wheel set is limited to %d wheels\n",
            wheel_set_size
        );
        return false;
    }

    for (unsigned int i = 0; i < size; i++) {
        json_t* name_json = json_array_get(wheel_set_json, i);
        if (name_json->type != JSON_STRING) {
            fprintf(
                stderr,
                "Error: Wheel set is expected to be an array of strings\n"
            );
            return false;
        }

        const char* name = json_string_value(name_json);
        bomm_wheel_t* wheel = bomm_lookup_string(
            wheels,
            sizeof(bomm_wheel_t),
            wheel_count,
            name
        );

        if (wheel != NULL) {
            memcpy(&wheel_set[i], wheel, sizeof(bomm_wheel_t));
        } else if (!bomm_wheel_init_with_name(&wheel_set[i], name)) {
            fprintf(
                stderr,
                "Error: The wheel with name '%s' cannot be found among " \
                "custom or known wheels\n",
                name
            );
            return false;
        }
    }

    return true;
}
