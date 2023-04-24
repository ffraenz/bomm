//
//  key.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "key.h"

// TODO: Make this generic (depending on `BOMM_ALPHABET_SIZE`)
const unsigned int bomm_key_plugboard_identity[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
   20, 21, 22, 23, 24, 25
};

bomm_key_space_t* bomm_key_space_init(
    bomm_key_space_t* key_space,
    bomm_mechanism_t mechanism,
    unsigned int slot_count
) {
    if (key_space == NULL) {
        key_space = malloc(sizeof(bomm_key_space_t));
        if (key_space == NULL) {
            return NULL;
        }
    }

    // Store meta data
    key_space->mechanism = mechanism;
    key_space->slot_count = slot_count;

    // Clear wheel sets
    size_t wheel_sets_mem_size =
        sizeof(bomm_wheel_t*) * BOMM_MAX_SLOT_COUNT * BOMM_MAX_WHEEL_SET_SIZE;
    memset(key_space->wheel_sets, 0, wheel_sets_mem_size);

    // By default, use the full ring setting and start position masks for all
    // slots except for the first (housing the reflector) and the last (housing
    // the entry wheel) that usually are stagnant
    for (unsigned int slot = 0; slot < slot_count; slot++) {
        bool is_stagnant = slot == 0 || slot == slot_count - 1;
        bomm_lettermask_t lettermask =
            is_stagnant ? BOMM_LETTERMASK_FIRST : BOMM_LETTERMASK_ALL;
        key_space->rotating_slots[slot] = !is_stagnant;
        key_space->ring_masks[slot] = lettermask;
        key_space->position_masks[slot] = lettermask;
    }

    return key_space;
}

bomm_mechanism_t bomm_key_mechanism_from_string(const char* mechanism_string) {
    if (strcmp(mechanism_string, "stepping") == 0) {
        return BOMM_MECHANISM_STEPPING;
    } else if (strcmp(mechanism_string, "odometer") == 0) {
        return BOMM_MECHANISM_ODOMETER;
    } else {
        return BOMM_MECHANISM_NONE;
    }
}

bomm_key_space_t* bomm_key_space_init_with_json(
    bomm_key_space_t* key_space_ptr,
    json_t* key_space_json,
    bomm_wheel_t wheels[],
    unsigned int wheel_count
) {
    if (key_space_json->type != JSON_OBJECT) {
        return NULL;
    }

    // Read mechanism
    json_t* mechanism_json = json_object_get(key_space_json, "mechanism");
    bomm_mechanism_t mechanism = BOMM_MECHANISM_STEPPING;
    if (mechanism_json != NULL) {
        mechanism = bomm_key_mechanism_from_string(json_string_value(mechanism_json));
    }

    // Read slot count
    json_t* slots_json = json_object_get(key_space_json, "slots");
    if (slots_json->type != JSON_ARRAY) {
        return NULL;
    }
    unsigned int slot_count = (unsigned int) json_array_size(slots_json);
    if (slot_count > BOMM_MAX_SLOT_COUNT) {
        return NULL;
    }

    // Init key space
    bomm_key_space_t* key_space = bomm_key_space_init(key_space_ptr, mechanism, slot_count);

    bool error = false;
    unsigned int slot = 0;
    while (!error && slot < slot_count) {
        json_t* slot_json = json_array_get(slots_json, slot);
        if (slot_json->type == JSON_OBJECT) {
            // Read wheel set
            json_t* wheel_set_json = json_object_get(slot_json, "wheels");
            if (!bomm_wheel_set_init_with_json(
                key_space->wheel_sets[slot],
                BOMM_MAX_WHEEL_SET_SIZE,
                wheel_set_json,
                wheels,
                wheel_count
            )) {
                error = true;
            }

            // Read ring mask
            json_t* ring_mask_json = json_object_get(slot_json, "rings");
            if (ring_mask_json != NULL) {
                if (ring_mask_json->type == JSON_STRING) {
                    if (bomm_lettermask_from_string(
                        &key_space->ring_masks[slot],
                        json_string_value(ring_mask_json)
                    ) == NULL) {
                        error = true;
                    }
                } else {
                    error = true;
                }
            }

            // Read position mask
            json_t* position_mask_json = json_object_get(slot_json, "positions");
            if (position_mask_json != NULL) {
                if (position_mask_json->type == JSON_STRING) {
                    if (bomm_lettermask_from_string(
                        &key_space->position_masks[slot],
                        json_string_value(position_mask_json)
                    ) == NULL) {
                        error = true;
                    }
                } else {
                    error = true;
                }
            }

            // Read slot rotating (optional)
            json_t* rotating_json = json_object_get(slot_json, "rotating");
            if (rotating_json != NULL) {
                if (rotating_json->type == JSON_TRUE || rotating_json->type == JSON_FALSE) {
                    key_space->rotating_slots[slot] = rotating_json->type == JSON_TRUE;
                } else {
                    error = true;
                }
            }
        } else {
            error = true;
        }

        slot++;
    }

    if (error) {
        if (key_space_ptr == NULL) {
            free(key_space);
        }
        return NULL;
    }

    return key_space;
}

void bomm_key_space_destroy(bomm_key_space_t* key_space) {
    free(key_space);
}

bomm_key_space_t* bomm_key_space_init_enigma_i(void) {
    bomm_key_space_t* key_space = bomm_key_space_init(NULL, BOMM_MECHANISM_STEPPING, 5);
    if (key_space == NULL) {
        return NULL;
    }

    // Initialize wheels
    // TODO: Check if initialization succeeded
    // TODO: Handle ownership of the wheels
    bomm_wheel_t* w_i       = bomm_wheel_init_with_name(NULL, "I");
    bomm_wheel_t* w_ii      = bomm_wheel_init_with_name(NULL, "II");
    bomm_wheel_t* w_iii     = bomm_wheel_init_with_name(NULL, "III");
    bomm_wheel_t* w_iv      = bomm_wheel_init_with_name(NULL, "IV");
    bomm_wheel_t* w_v       = bomm_wheel_init_with_name(NULL, "V");
    bomm_wheel_t* w_etw_abc = bomm_wheel_init_with_name(NULL, "ETW-ABC");
    bomm_wheel_t* w_ukw_a   = bomm_wheel_init_with_name(NULL, "UKW-A");
    bomm_wheel_t* w_ukw_b   = bomm_wheel_init_with_name(NULL, "UKW-B");
    bomm_wheel_t* w_ukw_c   = bomm_wheel_init_with_name(NULL, "UKW-C");

    // Set of reflectors (in the order they are tested)
    key_space->wheel_sets[0][0] = w_ukw_b;
    key_space->wheel_sets[0][1] = w_ukw_c;
    key_space->wheel_sets[0][2] = w_ukw_a;

    // Wheel sets for slots 1, 2, and 3
    for (int slot = 1; slot <= 3; slot++) {
        key_space->wheel_sets[slot][0] = w_i;
        key_space->wheel_sets[slot][1] = w_ii;
        key_space->wheel_sets[slot][2] = w_iii;
        key_space->wheel_sets[slot][3] = w_iv;
        key_space->wheel_sets[slot][4] = w_v;
    }

    // Set of entry wheels
    key_space->wheel_sets[4][0] = w_etw_abc;

    // Don't test ring settings for the left and middle wheel
    // as they can be neglected
    key_space->ring_masks[1] = BOMM_LETTERMASK_FIRST;
    key_space->ring_masks[2] = BOMM_LETTERMASK_FIRST;

    return key_space;
}

bomm_key_t* bomm_key_init(bomm_key_t* key, bomm_key_space_t* key_space) {
    if (key == NULL) {
        key = malloc(sizeof(bomm_key_t));
        if (key == NULL) {
            return NULL;
        }
    }

    // Initialize key from key space meta data
    key->mechanism = key_space->mechanism;
    key->slot_count = key_space->slot_count;
    memcpy(&key->rotating_slots, &key_space->rotating_slots, sizeof(key->rotating_slots));

    // Reset the wheel order, ring settings, and start positions
    for (unsigned int slot = 0; slot < key_space->slot_count; slot++) {
        memcpy(
            &key->wheels[slot],
            key_space->wheel_sets[slot][0],
            sizeof(bomm_wheel_t)
        );
        key->rings[slot] = 0;
        key->positions[slot] = 0;
    }

    // Initialize the key with the identity plugboard
    memcpy(
        &key->plugboard,
        &bomm_key_plugboard_identity,
        sizeof(bomm_key_plugboard_identity)
    );

    return key;
}

bomm_key_iterator_t* bomm_key_iterator_init(
    bomm_key_iterator_t* iterator,
    bomm_key_space_t* key_space
) {
    unsigned int slot_count = key_space->slot_count;
    bool empty = false;

    // Validate wheel sets are not empty
    for (unsigned int slot = 0; slot < slot_count; slot++) {
        empty = empty || key_space->wheel_sets[slot][0] == NULL;
    }
    if (empty) {
        return NULL;
    }

    bool owning = iterator == NULL;
    if (owning) {
        if ((iterator = malloc(sizeof(bomm_key_iterator_t))) == NULL) {
            return NULL;
        }
    }

    // Reference key space
    iterator->key_space = key_space;

    // Init key
    bomm_key_init(&iterator->key, key_space);

    // Reset wheels, ring masks, and position masks
    size_t masks_size = sizeof(bomm_lettermask_t) * slot_count;
    memset(&iterator->wheel_indices, 0, sizeof(unsigned int) * slot_count);
    memcpy(&iterator->ring_masks, key_space->ring_masks, masks_size);
    memcpy(&iterator->position_masks, key_space->position_masks, masks_size);

    // Find initial sets of wheels, rings, and positions
    empty = (
        empty ||
        bomm_key_iterator_wheels_next(
            iterator, false) ||
        bomm_key_iterator_positions_init(
            iterator->key.rings, iterator->ring_masks, slot_count) ||
        bomm_key_iterator_positions_init(
            iterator->key.positions, iterator->position_masks, slot_count)
    );

    if (empty) {
        if (owning) {
            free(iterator);
        }
        return NULL;
    }

    return iterator;
}

void bomm_key_stringify(char* str, size_t size, bomm_key_t* key) {
    size_t wheel_order_string_size = (key->slot_count * BOMM_WHEEL_NAME_MAX_LENGTH + 1) + 1;
    char wheel_order_string[wheel_order_string_size];
    bomm_key_wheels_stringify(wheel_order_string, wheel_order_string_size, key);

    size_t rings_string_size = key->slot_count + 1;
    char rings_string[rings_string_size];
    bomm_key_rings_stringify(rings_string, rings_string_size, key);

    size_t positions_string_size = key->slot_count + 1;
    char positions_string[positions_string_size];
    bomm_key_positions_stringify(positions_string, positions_string_size, key);

    char plugboard_string[39];
    bomm_key_plugboard_stringify(plugboard_string, 39, key);

    snprintf(str, size, "%s %s %s %-39s", wheel_order_string, rings_string, positions_string, plugboard_string);
}

void bomm_key_wheels_stringify(char* str, size_t size, bomm_key_t* key) {
    str[0] = 0;
    for (unsigned int slot = 0; slot < key->slot_count; slot++) {
        char* name = key->wheels[slot].name;
        snprintf(str, size, "%s%s%s", str, slot == 0 ? "" : ",", name);
    }
}

void bomm_key_rings_stringify(char* str, size_t size, bomm_key_t* key) {
    for (unsigned int slot = 0; slot < key->slot_count && slot < size - 1; slot++) {
        str[slot] = bomm_message_letter_to_ascii(key->rings[slot]);
    }
    str[size - 1] = '\0';
}

void bomm_key_positions_stringify(char* str, size_t size, bomm_key_t* key) {
    for (unsigned int slot = 0; slot < key->slot_count && slot < size - 1; slot++) {
        str[slot] = bomm_message_letter_to_ascii(key->positions[slot]);
    }
    str[size - 1] = '\0';
}

void bomm_key_plugboard_stringify(char* str, size_t size, bomm_key_t* key) {
    unsigned int i = 0;
    unsigned int j = 0;

    bomm_lettermask_t used_letters = BOMM_LETTERMASK_NONE;

    while (i < BOMM_ALPHABET_SIZE && j + 3 < size) {
        if (key->plugboard[i] != i && !bomm_lettermask_has(&used_letters, i)) {
            bomm_lettermask_set(&used_letters, key->plugboard[i]);
            str[j++] = bomm_message_letter_to_ascii(i);
            str[j++] = bomm_message_letter_to_ascii(key->plugboard[i]);
            str[j++] = ' ';
        }
        i++;
    }

    str[j > 0 ? j - 1 : 0] = '\0';
}

void bomm_key_hold_print(bomm_hold_t* hold) {
    // Lock hold while printing
    pthread_mutex_lock(&hold->mutex);

    if (hold->count == 0) {
        printf("Empty key hold\n");
        return;
    }
    char key_string[128];
    bomm_hold_element_t* element;
    for (unsigned int i = 0; i < hold->count; i++) {
        element = bomm_hold_at(hold, i);
        bomm_key_stringify(key_string, 128, (bomm_key_t*) element->data);
        printf("%2d │ %80s │ %10f │ %30s\n", i + 1, key_string, element->score, element->preview);
    }

    // Unlock hold
    pthread_mutex_unlock(&hold->mutex);
}
