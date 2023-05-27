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
    if (!key_space && !(key_space = malloc(sizeof(bomm_key_space_t)))) {
        return NULL;
    }

    key_space->mechanism = mechanism;
    key_space->slot_count = slot_count;
    key_space->plug_mask = BOMM_LETTERMASK_NONE;
    key_space->count = 0;
    key_space->offset = 0;
    key_space->limit = LONG_MAX;

    memset(key_space->wheel_sets, 0, sizeof(key_space->wheel_sets));

    for (unsigned int slot = 0; slot < slot_count; slot++) {
        key_space->ring_masks[slot] = BOMM_LETTERMASK_FIRST;
        key_space->position_masks[slot] = BOMM_LETTERMASK_FIRST;
        key_space->rotating_slots[slot] = false;
    }

    return key_space;
}

bomm_key_space_t* bomm_key_space_init_enigma_i(bomm_key_space_t* key_space) {
    if (!bomm_key_space_init(key_space, BOMM_MECHANISM_STEPPING, 5)) {
        return NULL;
    }

    // Rotating slots
    key_space->rotating_slots[1] = true;
    key_space->rotating_slots[2] = true;
    key_space->rotating_slots[3] = true;

    // Set of reflectors (in the order they are tested)
    bomm_wheel_init_with_name(&key_space->wheel_sets[0][0], "UKW-B");

    // Wheel sets for slots 1, 2, and 3
    for (int slot = 1; slot <= 3; slot++) {
        bomm_wheel_init_with_name(&key_space->wheel_sets[slot][0], "I");
        bomm_wheel_init_with_name(&key_space->wheel_sets[slot][1], "II");
        bomm_wheel_init_with_name(&key_space->wheel_sets[slot][2], "III");
        bomm_wheel_init_with_name(&key_space->wheel_sets[slot][3], "IV");
        bomm_wheel_init_with_name(&key_space->wheel_sets[slot][4], "V");
    }

    // Set of entry wheels
    bomm_wheel_init_with_name(&key_space->wheel_sets[4][0], "ETW-ABC");

    // Ring settings to be enumerated
    key_space->ring_masks[3] = BOMM_LETTERMASK_ALL;

    // Positions to be enumerated
    key_space->position_masks[1] = BOMM_LETTERMASK_ALL;
    key_space->position_masks[2] = BOMM_LETTERMASK_ALL;
    key_space->position_masks[3] = BOMM_LETTERMASK_ALL;

    return key_space;
}

bomm_key_space_t* bomm_key_space_init_with_json(
    bomm_key_space_t* key_space,
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
    if (mechanism_json != NULL && mechanism_json->type == JSON_STRING) {
        mechanism =
            bomm_key_mechanism_from_string(json_string_value(mechanism_json));
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
    bool owning_key_space = key_space == NULL;
    key_space = bomm_key_space_init(key_space, mechanism, slot_count);

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
        if (owning_key_space) {
            free(key_space);
        }
        return NULL;
    }

    return key_space;
}

void bomm_key_space_debug(const bomm_key_space_t* key_space) {
    printf("Debug key space %p:\n", (void*) key_space);

    for (unsigned int slot = 0; slot < key_space->slot_count; slot++) {
        printf(
            "  Slot %d%s:\n",
            slot + 1,
            key_space->rotating_slots[slot] ? " (rotating)" : ""
        );

        const bomm_wheel_t* wheel = &key_space->wheel_sets[slot][0];
        while (wheel->name[0] != '\0') {
            char wiring_string[128];
            bomm_wiring_stringify(wiring_string, sizeof(wiring_string), &wheel->wiring);
            char turnovers_string[128];
            bomm_lettermask_stringify(turnovers_string, sizeof(turnovers_string), &wheel->turnovers);
            printf(
                "    - %s (%s) '%s'\n",
                wiring_string,
                turnovers_string,
                wheel->name
            );
            wheel++;
        }
    }
}

void bomm_key_space_destroy(bomm_key_space_t* key_space) {
    free(key_space);
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

    if (actual_split_count == 0) {
        return 0;
    }

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
    bool owning = key == NULL;
    if (!key && !(key = malloc(sizeof(bomm_key_t)))) {
        return NULL;
    }

    // Initialize key from key space meta data
    key->mechanism = key_space->mechanism;
    key->slot_count = key_space->slot_count;
    memcpy(&key->rotating_slots, &key_space->rotating_slots, sizeof(key->rotating_slots));

    // Stepping mechanism: Determine the fast rotor slot
    if (key->mechanism == BOMM_MECHANISM_STEPPING) {
        key->fast_wheel_slot = key->slot_count - 1;
        while (key->fast_wheel_slot > 0 && !key->rotating_slots[key->fast_wheel_slot]) {
            key->fast_wheel_slot--;
        }
        if (key->fast_wheel_slot < 3) {
            fprintf(
                stderr,
                "Invalid scrambler configuration for the stepping mechanism\n"
            );
            if (owning) {
                free(key);
            }
            return NULL;
        }
    }

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
                &key_space->wheel_sets[slot][0],
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
        empty = empty || key_space->wheel_sets[slot][0].name[0] == '\0';
    }
    if (empty) {
        return NULL;
    }

    bool owning = iterator == NULL;
    if (!iterator && !(iterator = malloc(sizeof(bomm_key_iterator_t)))) {
        return NULL;
    }

    // Reference key space
    iterator->key_space = key_space;
    iterator->scrambler_changed = true;
    iterator->index = 0;

    // Init key
    if (bomm_key_init(&iterator->key, key_space) == NULL) {
        // If no key can be created from the key space, the key space is
        // considered empty
        if (owning) {
            free(iterator);
        }
        return NULL;
    }

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
            iterator->key.positions, iterator->position_masks, slot_count) ||
        !bomm_key_is_relevant(&iterator->key)
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

void bomm_key_debug(const bomm_key_t* key) {
    printf("Debug key %p:\n", (void*) key);

    char plugboard_string[128];
    bomm_wiring_plugboard_stringify(plugboard_string, sizeof(plugboard_string), key->plugboard);
    printf("  Mechanism: %s\n", bomm_key_mechanism_string(key->mechanism));
    printf("  Plugboard: '%s'\n", plugboard_string);

    char wiring_string[128];
    char turnovers_string[128];
    for (unsigned int slot = 0; slot < key->slot_count; slot++) {
        const bomm_wheel_t* wheel = &key->wheels[slot];
        bomm_wiring_stringify(wiring_string, sizeof(wiring_string), &wheel->wiring);
        bomm_lettermask_stringify(turnovers_string, sizeof(turnovers_string), &wheel->turnovers);
        printf(
            "  Slot %d: Ring %2d/%c Position %2d/%c Wheel %-7s (%s, %s)%s\n",
            slot + 1,
            key->rings[slot] + 1,
            bomm_message_letter_to_ascii(key->rings[slot]),
            key->positions[slot] + 1,
            bomm_message_letter_to_ascii(key->positions[slot]),
            wheel->name,
            wiring_string,
            turnovers_string,
            key->rotating_slots[slot] ? " (rotating)" : ""
        );
    }
}
