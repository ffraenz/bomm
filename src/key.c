//
//  key.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "key.h"

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
    key_space->plug_mask = BOMM_LETTERMASK_NONE;
    key_space->count = 0;
    key_space->offset = 0;
    key_space->limit = LONG_MAX;

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

    // Read plug mask
    json_t* plug_mask_json = json_object_get(key_space_json, "plugs");
    if (plug_mask_json != NULL) {
        if (plug_mask_json->type == JSON_STRING) {
            if (bomm_lettermask_from_string(
                &key_space->plug_mask,
                json_string_value(plug_mask_json)
            ) == NULL) {
                error = true;
            }
        } else {
            error = true;
        }
    }

    // Read offset
    json_t* offset_json = json_object_get(key_space_json, "offset");
    if (offset_json != NULL) {
        if (offset_json->type == JSON_INTEGER) {
            key_space->offset = json_integer_value(offset_json);
        } else {
            error = true;
        }
    }

    // Read limit
    json_t* limit_json = json_object_get(key_space_json, "limit");
    if (limit_json != NULL) {
        if (limit_json->type == JSON_INTEGER) {
            key_space->limit = json_integer_value(limit_json);
        } else {
            error = true;
        }
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
    bomm_wheel_t* w_ukw_b   = bomm_wheel_init_with_name(NULL, "UKW-B");

    // Set of reflectors (in the order they are tested)
    key_space->wheel_sets[0][0] = w_ukw_b;

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

unsigned int bomm_key_space_slice(
    const bomm_key_space_t* key_space,
    unsigned int split_count,
    bomm_key_space_t* slices
) {
    if (split_count == 0) {
        return 0;
    }

    unsigned long key_count = bomm_key_space_count(key_space);

    // Make sure every slice gets at least 1 key to work on
    unsigned int actual_split_count =
        key_count < (unsigned long) split_count
            ? (unsigned int) key_count
            : split_count;

    unsigned long slice_count = key_count / actual_split_count;
    unsigned long offset = key_space->offset;
    for (unsigned int i = 0; i < actual_split_count; i++) {
        memcpy(&slices[i], key_space, sizeof(bomm_key_space_t));
        slices[i].offset = offset;
        if (i < actual_split_count - 1) {
            slices[i].count = slices[i].limit = slice_count;
            offset += slice_count;
        } else {
            slices[i].count = slices[i].limit = key_count - offset;
        }
    }

    return actual_split_count;
}

unsigned long bomm_key_space_count(
    const bomm_key_space_t* key_space
) {
    // Return count if known
    if (key_space->count != 0) {
        return key_space->count;
    }

    // Derive a key space without plug mask
    bomm_key_space_t counting_key_space;
    memcpy(&counting_key_space, key_space, sizeof(counting_key_space));
    counting_key_space.plug_mask = BOMM_LETTERMASK_NONE;

    // Create a new iterator for it
    bomm_key_iterator_t key_iterator;
    if (bomm_key_iterator_init(&key_iterator, &counting_key_space) == NULL) {
        // The key space is empty
        return 0;
    }

    // Count scrambler keys
    unsigned long count = 1;
    while (!bomm_key_iterator_next(&key_iterator)) {
        count++;
    }

    // Multiply the scrambler keys with the number of solo plugs
    count *= bomm_key_space_plugboard_count(key_space);

    if (count <= key_space->offset) {
        return 0;
    }

    count -= key_space->offset;

    if (count > key_space->limit) {
        return key_space->limit;
    }

    return count;
}

bomm_key_t* bomm_key_init(bomm_key_t* key, const bomm_key_space_t* key_space) {
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

    // Reset the wheel order, ring settings, and start positions of all slots,
    // including the unused ones to remove undefined memory and thus make keys
    // comparable as a whole using `memcmp`.
    for (unsigned int slot = 0; slot < BOMM_MAX_SLOT_COUNT; slot++) {
        key->rings[slot] = 0;
        key->positions[slot] = 0;
        if (slot < key_space->slot_count) {
            key->rotating_slots[slot] = key_space->rotating_slots[slot];
            memcpy(
                &key->wheels[slot],
                key_space->wheel_sets[slot][0],
                sizeof(bomm_wheel_t)
            );
        } else {
            key->rotating_slots[slot] = false;
            memset(&key->wheels[slot], 0, sizeof(bomm_wheel_t));
        }
    }

    // Initialize the key with the identity plugboard
    bomm_wiring_plugboard_init_identity(key->plugboard);
    return key;
}

bomm_key_iterator_t* bomm_key_iterator_init(
    bomm_key_iterator_t* iterator,
    const bomm_key_space_t* key_space
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
    iterator->index = 0;

    // Init key
    bomm_key_init(&iterator->key, key_space);
    iterator->scrambler_changed = true;

    // Reset wheels, ring masks, and position masks
    size_t masks_size = sizeof(bomm_lettermask_t) * slot_count;
    memset(&iterator->wheel_indices, 0, sizeof(unsigned int) * slot_count);
    memcpy(&iterator->ring_masks, key_space->ring_masks, masks_size);
    memcpy(&iterator->position_masks, key_space->position_masks, masks_size);
    memset(&iterator->solo_plug, 0, sizeof(unsigned int) * 2);

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

    // Skip key space offset
    unsigned long offset = key_space->offset + 1;
    while (--offset > 0 && !empty) {
        // When calling `bomm_key_iterator_next` then `iterator->index` will
        // be incremented and checked against the limit
        iterator->index = -1;
        empty = bomm_key_iterator_next(iterator);
    }

    if (empty) {
        if (owning) {
            free(iterator);
        }
        return NULL;
    }

    return iterator;
}

void bomm_key_stringify(char* str, size_t size, bomm_key_t* key) {
    char wheel_order_string[size];
    bomm_key_wheels_stringify(wheel_order_string, size, key);
    char rings_string[size];
    bomm_key_rings_stringify(rings_string, size, key);
    char positions_string[size];
    bomm_key_positions_stringify(positions_string, size, key);
    char plugboard_string[size];
    bomm_wiring_plugboard_stringify(plugboard_string, size, key->plugboard);
    snprintf(str, size, "%s %s %s %s", wheel_order_string, rings_string, positions_string, plugboard_string);
}

void bomm_key_wheels_stringify(char* str, size_t size, bomm_key_t* key) {
    unsigned int i = 0;
    unsigned int slot = 0;

    char* name;
    size_t name_len;

    while (
        slot < key->slot_count &&
        (name = key->wheels[slot].name) &&
        (name_len = strlen(name)) > 0 &&
        i + (slot > 0 ? 1 : 0) + name_len < size - 1
    ) {
        if (slot > 0) {
            str[i++] = ',';
        }
        memcpy(&str[i], name, name_len);
        i += name_len;
        slot++;
    }

    if (size > 0) {
        str[i] = '\0';
    }
}

void bomm_key_rings_stringify(char* str, size_t size, bomm_key_t* key) {
    unsigned int i = 0;
    while (i < key->slot_count && i < size - 1) {
        str[i] = bomm_message_letter_to_ascii(key->rings[i]);
        i++;
    }
    str[i] = '\0';
}

void bomm_key_positions_stringify(char* str, size_t size, bomm_key_t* key) {
    unsigned int i = 0;
    while (i < key->slot_count && i < size - 1) {
        str[i] = bomm_message_letter_to_ascii(key->positions[i]);
        i++;
    }
    str[i] = '\0';
}
