//
//  model.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "model.h"

void bomm_load_wheel_args(
    bomm_wheel_t* ptr,
    char* name,
    char* wiring_string,
    char* turnovers_string
) {
    size_t actual_name_length = strlen(name);
    size_t name_length =
        actual_name_length < BOMM_WHEEL_NAME_MAX_LENGTH - 1
            ? actual_name_length
            : BOMM_WHEEL_NAME_MAX_LENGTH;
    memcpy(ptr->name, name, name_length);
    bomm_wiring_extract(&ptr->wiring, wiring_string);
    bomm_lettermask_extract(&ptr->turnovers, turnovers_string);
}

bomm_model_t* bomm_model_alloc(unsigned char wheel_count) {
    bomm_model_t* model = (bomm_model_t*) malloc(bomm_model_size(wheel_count));
    if (model == NULL) {
        return NULL;
    }
    
    model->wheel_count = wheel_count;
    
    size_t wheel_sets_size =
        sizeof(unsigned char) *
        BOMM_MODEL_MAX_SLOT_COUNT *
        BOMM_MODEL_MAX_WHEEL_SET_SIZE;
    memset(model->wheel_sets, 255, wheel_sets_size);
    
    return model;
}

bomm_model_t* bomm_model_alloc_enigma_i(void) {
    unsigned char wheel_count = 9;
    bomm_model_t* model = bomm_model_alloc(wheel_count);
    model->mechanism = BOMM_MECHANISM_STEPPING;
    model->slot_count = 5;
    model->wheel_count = wheel_count;

    // Load available wheels
    bomm_load_wheel_args(&model->wheels[0], "I",     "ekmflgdqvzntowyhxuspaibrcj", "q");
    bomm_load_wheel_args(&model->wheels[1], "II",    "ajdksiruxblhwtmcqgznpyfvoe", "e");
    bomm_load_wheel_args(&model->wheels[2], "III",   "bdfhjlcprtxvznyeiwgakmusqo", "v");
    bomm_load_wheel_args(&model->wheels[3], "IV",    "esovpzjayquirhxlnftgkdcmwb", "j");
    bomm_load_wheel_args(&model->wheels[4], "V",     "vzbrgityupsdnhlxawmjqofeck", "z");
    bomm_load_wheel_args(&model->wheels[5], "ABC",   "abcdefghijklmnopqrstuvwxyz", "");
    bomm_load_wheel_args(&model->wheels[6], "UKW-A", "ejmzalyxvbwfcrquontspikhgd", "");
    bomm_load_wheel_args(&model->wheels[7], "UKW-B", "yruhqsldpxngokmiebfzcwvjat", "");
    bomm_load_wheel_args(&model->wheels[8], "UKW-C", "fvpjiaoyedrzxwgctkuqsbnmhl", "");
    
    // Set rotating slots
    model->rotating_slots[0] = false;
    model->rotating_slots[1] = true;
    model->rotating_slots[2] = true;
    model->rotating_slots[3] = true;
    model->rotating_slots[4] = false;
    
    // Available reflectors: UKW-A, UKW-B, UKW-C
    model->wheel_sets[0][0] = 6;
    model->wheel_sets[0][1] = 7;
    model->wheel_sets[0][2] = 8;
    
    // Wheel sets for slots 1, 2, and 3: I, II, III, IV, V
    for (int slot = 1; slot <= 3; slot++) {
        for (int index = 0; index < 5; index++) {
            model->wheel_sets[slot][index] = index;
        }
    }
    
    // Available entry wheels: ETW-ABC
    model->wheel_sets[4][0] = 5;
    
    // Test ring settings of the right wheel, only
    model->ring_masks[0] = BOMM_LETTERMASK_FIRST;
    model->ring_masks[1] = BOMM_LETTERMASK_FIRST;
    model->ring_masks[2] = BOMM_LETTERMASK_FIRST;
    model->ring_masks[3] = BOMM_LETTERMASK_ALL;
    model->ring_masks[4] = BOMM_LETTERMASK_FIRST;
    
    // Test all wheel positions for rotating wheels
    model->position_masks[0] = BOMM_LETTERMASK_FIRST;
    model->position_masks[1] = BOMM_LETTERMASK_ALL;
    model->position_masks[2] = BOMM_LETTERMASK_ALL;
    model->position_masks[3] = BOMM_LETTERMASK_ALL;
    model->position_masks[4] = BOMM_LETTERMASK_FIRST;

    return model;
}
