//
//  model.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "model.h"

void bomm_load_rotor_spec_args(
    bomm_rotor_spec_t* ptr,
    char* name,
    char* wiring_string,
    char* turnovers_string,
    bool rotating
) {
    size_t actual_name_length = strlen(name);
    size_t name_length =
        actual_name_length < BOMM_ROTOR_NAME_MAX_LENGTH - 1
            ? actual_name_length
            : BOMM_ROTOR_NAME_MAX_LENGTH;
    memcpy(ptr->name, name, name_length);
    bomm_wiring_extract(&ptr->wiring, wiring_string);
    bomm_lettermask_extract(&ptr->turnovers, turnovers_string);
    ptr->rotating = rotating;
}

bomm_model_t* bomm_alloc_model(unsigned char rotor_count) {
    size_t model_size =
        sizeof(bomm_model_t) +
        sizeof(bomm_rotor_spec_t) * rotor_count;
    size_t slot_rotor_indices_size =
        sizeof(unsigned char) *
        BOMM_MODEL_MAX_SLOT_COUNT *
        BOMM_MODEL_MAX_ROTOR_COUNT_PER_SLOT;
    
    bomm_model_t* model = (bomm_model_t*) malloc(model_size);
    model->rotor_count = rotor_count;
    
    // Clear slot rotor indices
    memset(model->slot_rotor_indices, 255, slot_rotor_indices_size);
    
    return model;
}

bomm_model_t* bomm_alloc_model_enigma_i(void) {
    unsigned char rotor_count = 9;
    bomm_model_t* model = bomm_alloc_model(rotor_count);
    model->rotation_mechanism = BOMM_ENIGMA_STEPPING;
    model->slot_count = 5;
    model->fast_rotating_slot = 3;
    model->rotor_count = rotor_count;

    // Load available rotors
    bomm_load_rotor_spec_args(&model->rotors[0], "I",       "ekmflgdqvzntowyhxuspaibrcj", "q", true);
    bomm_load_rotor_spec_args(&model->rotors[1], "II",      "ajdksiruxblhwtmcqgznpyfvoe", "e", true);
    bomm_load_rotor_spec_args(&model->rotors[2], "III",     "bdfhjlcprtxvznyeiwgakmusqo", "v", true);
    bomm_load_rotor_spec_args(&model->rotors[3], "IV",      "esovpzjayquirhxlnftgkdcmwb", "j", true);
    bomm_load_rotor_spec_args(&model->rotors[4], "V",       "vzbrgityupsdnhlxawmjqofeck", "z", true);
    bomm_load_rotor_spec_args(&model->rotors[5], "ETW-ABC", "abcdefghijklmnopqrstuvwxyz", "",  false);
    bomm_load_rotor_spec_args(&model->rotors[6], "UKW-A",   "ejmzalyxvbwfcrquontspikhgd", "",  false);
    bomm_load_rotor_spec_args(&model->rotors[7], "UKW-B",   "yruhqsldpxngokmiebfzcwvjat", "",  false);
    bomm_load_rotor_spec_args(&model->rotors[8], "UKW-C",   "fvpjiaoyedrzxwgctkuqsbnmhl", "",  false);
    
    // Available reflectors: UKW-A, UKW-B, UKW-C
    model->slot_rotor_indices[0][0] = 6;
    model->slot_rotor_indices[0][1] = 7;
    model->slot_rotor_indices[0][2] = 8;
    
    // Available rotors for slots 1, 2, and 3: I, II, III, IV, V
    for (int slot = 1; slot <= 3; slot++) {
        for (int index = 0; index < 5; index++) {
            model->slot_rotor_indices[slot][index] = index;
        }
    }
    
    // Available entry rotors: ETW-ABC
    model->slot_rotor_indices[4][0] = 5;
    
    // Test ring settings of the right rotor, only
    model->slot_ring_mask[0] = BOMM_LETTERMASK_FIRST;
    model->slot_ring_mask[1] = BOMM_LETTERMASK_FIRST;
    model->slot_ring_mask[2] = 0x00080020;
    model->slot_ring_mask[3] = BOMM_LETTERMASK_ALL;
    model->slot_ring_mask[4] = BOMM_LETTERMASK_FIRST;
    
    // Test all wheel positions
    model->slot_position_mask[0] = BOMM_LETTERMASK_FIRST;
    model->slot_position_mask[1] = BOMM_LETTERMASK_ALL;
    model->slot_position_mask[2] = BOMM_LETTERMASK_ALL;
    model->slot_position_mask[3] = BOMM_LETTERMASK_ALL;
    model->slot_position_mask[4] = BOMM_LETTERMASK_FIRST;

    return model;
}
