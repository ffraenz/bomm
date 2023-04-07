//
//  key.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "key.h"

const bomm_letter_t bomm_key_plugboard_identity[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
   20, 21, 22, 23, 24, 25
};

bomm_wheel_t* bomm_wheel_init(
    char* name,
    char* wiring_string,
    char* turnovers_string
) {
    bomm_wheel_t* wheel = malloc(sizeof(bomm_wheel_t));
    if (wheel == NULL) {
        return NULL;
    }
    
    bomm_strncpy(wheel->name, name, BOMM_WHEEL_NAME_MAX_LENGTH);
    bomm_wiring_extract(&wheel->wiring, wiring_string);
    bomm_lettermask_extract(&wheel->turnovers, turnovers_string);
    
    return wheel;
}

bomm_key_space_t* bomm_key_space_init(bomm_mechanism_t mechanism, unsigned int slot_count) {
    bomm_key_space_t* key_space = malloc(sizeof(bomm_key_space_t));
    if (key_space == NULL) {
        return NULL;
    }
    
    // Store meta data
    key_space->mechanism = mechanism;
    key_space->slot_count = slot_count;
    
    // Clear wheel sets
    size_t wheel_sets_mem_size =
        sizeof(bomm_wheel_t*) * BOMM_MAX_SLOT_COUNT * BOMM_MAX_WHEEL_SET_SIZE;
    memset(key_space->wheel_sets, 0, wheel_sets_mem_size);
    
    // By default, use the full mask for all wheels except for the first
    // wheel (representing the reflector) and the last wheel (representing
    // the entry wheel) that usually are stagnant and have no ring setting
    for (unsigned int i = 0; i < slot_count; i++) {
        bool is_stagnant = i == 0 || i == slot_count - 1;
        bomm_lettermask_t lettermask =
            is_stagnant ? BOMM_LETTERMASK_FIRST : BOMM_LETTERMASK_ALL;
        key_space->rotating_slots[i] = !is_stagnant;
        key_space->ring_masks[i] = lettermask;
        key_space->position_masks[i] = lettermask;
    }
    
    return key_space;
}

void bomm_key_space_destroy(bomm_key_space_t* key_space) {
    // TODO: Free wheels referenced by this key space
    free(key_space);
}

bomm_key_space_t* bomm_key_space_enigma_i_init(void) {
    bomm_key_space_t* key_space = bomm_key_space_init(BOMM_MECHANISM_STEPPING, 5);
    if (key_space == NULL) {
        return NULL;
    }
    
    // Initialize wheels
    // TODO: Check if initialization succeeded
    // TODO: Handle ownership of the wheels           abcdefghijklmnopqrstuvwxyz
    bomm_wheel_t* w_i     = bomm_wheel_init("I",     "ekmflgdqvzntowyhxuspaibrcj", "q");
    bomm_wheel_t* w_ii    = bomm_wheel_init("II",    "ajdksiruxblhwtmcqgznpyfvoe", "e");
    bomm_wheel_t* w_iii   = bomm_wheel_init("III",   "bdfhjlcprtxvznyeiwgakmusqo", "v");
    bomm_wheel_t* w_iv    = bomm_wheel_init("IV",    "esovpzjayquirhxlnftgkdcmwb", "j");
    bomm_wheel_t* w_v     = bomm_wheel_init("V",     "vzbrgityupsdnhlxawmjqofeck", "z");
    bomm_wheel_t* w_abc   = bomm_wheel_init("ABC",   "abcdefghijklmnopqrstuvwxyz", "");
    bomm_wheel_t* w_ukw_a = bomm_wheel_init("UKW-A", "ejmzalyxvbwfcrquontspikhgd", "");
    bomm_wheel_t* w_ukw_b = bomm_wheel_init("UKW-B", "yruhqsldpxngokmiebfzcwvjat", "");
    bomm_wheel_t* w_ukw_c = bomm_wheel_init("UKW-C", "fvpjiaoyedrzxwgctkuqsbnmhl", "");
    
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
    key_space->wheel_sets[4][0] = w_abc;
    
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
    
    // Reset ring settings and start positions to 0
    for (unsigned int slot = 0; slot < key_space->slot_count; slot++) {
        key->rings[slot] = 0;
        key->positions[slot] = 0;
    }
    
    // Initialize the key with the identity plugboard
    memcpy(&key->plugboard, &bomm_key_plugboard_identity, sizeof(bomm_key_plugboard_identity));
    
    return key;
}

void bomm_key_serialize(char* str, size_t size, bomm_key_t* key) {
    size_t wheel_order_string_size = (key->slot_count * BOMM_WHEEL_NAME_MAX_LENGTH + 1) + 1;
    char wheel_order_string[wheel_order_string_size];
    bomm_key_serialize_wheel_order(wheel_order_string, wheel_order_string_size, key);
    
    size_t rings_string_size = key->slot_count + 1;
    char rings_string[rings_string_size];
    bomm_key_serialize_ring_settings(rings_string, rings_string_size, key);
    
    size_t positions_string_size = key->slot_count + 1;
    char positions_string[positions_string_size];
    bomm_key_serialize_start_positions(positions_string, positions_string_size, key);
    
    char plugboard_string[39];
    bomm_key_serialize_plugboard(plugboard_string, 39, key);
    
    snprintf(str, size, "%s %s %s %-39s", wheel_order_string, rings_string, positions_string, plugboard_string);
}

void bomm_key_serialize_wheel_order(char* str, size_t size, bomm_key_t* key) {
    str[0] = 0;
    for (unsigned int slot = 0; slot < key->slot_count; slot++) {
        char* name = key->wheels[slot].name;
        snprintf(str, size, "%s%s%s", str, slot == 0 ? "" : ",", name);
    }
}

void bomm_key_serialize_ring_settings(char* str, size_t size, bomm_key_t* key) {
    for (unsigned int slot = 0; slot < key->slot_count && slot < size - 1; slot++) {
        str[slot] = bomm_message_letter_to_ascii(key->rings[slot]);
    }
    str[size - 1] = '\0';
}

void bomm_key_serialize_start_positions(char* str, size_t size, bomm_key_t* key) {
    for (unsigned int slot = 0; slot < key->slot_count && slot < size - 1; slot++) {
        str[slot] = bomm_message_letter_to_ascii(key->positions[slot]);
    }
    str[size - 1] = '\0';
}

void bomm_key_serialize_plugboard(char* str, size_t size, bomm_key_t* key) {
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
        bomm_key_serialize(key_string, 128, (bomm_key_t*) element->data);
        printf("%2d │ %72s │ %10f │ %30s\n", i + 1, key_string, element->score, element->preview);
    }
    
    // Unlock hold
    pthread_mutex_unlock(&hold->mutex);
}
