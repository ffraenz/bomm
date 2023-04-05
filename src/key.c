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

void bomm_key_serialize(char* str, size_t size, bomm_key_t* key) {
    size_t wheel_order_string_size = (key->model->slot_count * BOMM_WHEEL_NAME_MAX_LENGTH + 1) + 1;
    char wheel_order_string[wheel_order_string_size];
    bomm_key_serialize_wheel_order(wheel_order_string, wheel_order_string_size, key);
    
    size_t rings_string_size = key->model->slot_count + 1;
    char rings_string[rings_string_size];
    bomm_key_serialize_ring_settings(rings_string, rings_string_size, key);
    
    size_t positions_string_size = key->model->slot_count + 1;
    char positions_string[positions_string_size];
    bomm_key_serialize_start_positions(positions_string, positions_string_size, key);
    
    char plugboard_string[39];
    bomm_key_serialize_plugboard(plugboard_string, 39, key);
    
    snprintf(str, size, "%s %s %s %-39s", wheel_order_string, rings_string, positions_string, plugboard_string);
}

void bomm_key_serialize_wheel_order(char* str, size_t size, bomm_key_t* key) {
    str[0] = 0;
    for (unsigned int slot = 0; slot < key->model->slot_count; slot++) {
        char* name = key->model->wheels[key->model->wheel_sets[slot][key->wheels[slot]]].name;
        snprintf(str, size, "%s%s%s", str, slot == 0 ? "" : ",", name);
    }
}

void bomm_key_serialize_ring_settings(char* str, size_t size, bomm_key_t* key) {
    for (unsigned int slot = 0; slot < key->model->slot_count && slot < size - 1; slot++) {
        str[slot] = bomm_message_letter_to_ascii(key->rings[slot]);
    }
    str[size - 1] = '\0';
}

void bomm_key_serialize_start_positions(char* str, size_t size, bomm_key_t* key) {
    for (unsigned int slot = 0; slot < key->model->slot_count && slot < size - 1; slot++) {
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
}
