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
    bomm_wiring_init(&wheel->wiring, wiring_string);

    if (turnovers_string != NULL) {
        bomm_lettermask_from_string(&wheel->turnovers, turnovers_string);
    } else {
        wheel->turnovers = BOMM_LETTERMASK_NONE;
    }

    return wheel;
}

bomm_wheel_t* bomm_wheel_init_with_name(bomm_wheel_t* wheel, const char* name) {
    if (strcmp(name, "I") == 0) {
        return bomm_wheel_init(wheel, name, "ekmflgdqvzntowyhxuspaibrcj", "q");
    } else if (strcmp(name, "II") == 0) {
        return bomm_wheel_init(wheel, name, "ajdksiruxblhwtmcqgznpyfvoe", "e");
    } else if (strcmp(name, "III") == 0) {
        return bomm_wheel_init(wheel, name, "bdfhjlcprtxvznyeiwgakmusqo", "v");
    } else if (strcmp(name, "IV") == 0) {
        return bomm_wheel_init(wheel, name, "esovpzjayquirhxlnftgkdcmwb", "j");
    } else if (strcmp(name, "V") == 0) {
        return bomm_wheel_init(wheel, name, "vzbrgityupsdnhlxawmjqofeck", "z");
    } else if (strcmp(name, "ETW-ABC") == 0) {
        return bomm_wheel_init(wheel, name, "abcdefghijklmnopqrstuvwxyz", "");
    } else if (strcmp(name, "UKW-A") == 0) {
        return bomm_wheel_init(wheel, name, "ejmzalyxvbwfcrquontspikhgd", "");
    } else if (strcmp(name, "UKW-B") == 0) {
        return bomm_wheel_init(wheel, name, "yruhqsldpxngokmiebfzcwvjat", "");
    } else if (strcmp(name, "UKW-C") == 0) {
        return bomm_wheel_init(wheel, name, "fvpjiaoyedrzxwgctkuqsbnmhl", "");
    } else {
        return NULL;
    }
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
    bomm_wheel_t* wheel_set[],
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

        if (wheel == NULL) {
            fprintf(
                stderr,
                "Error: The wheel named '%s' cannot be found\n",
                name
            );
            return false;
        }

        wheel_set[i] = wheel;
    }

    return true;
}
