//
//  key.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "key.h"

void bomm_serialize_key(char* str, size_t size, bomm_key_t* key) {
    // Clear string
    str[0] = 0;
    
    // Compose wheel order
    for (unsigned int slot = 0; slot < key->model->slot_count; slot++) {
        char* name = key->model->rotors[key->model->slot_rotor_indices[slot][key->slot_rotor[slot]]].name;
        snprintf(str, size, "%s%s%s", str, slot == 0 ? "" : ":", name);
    }
    
    // TODO: Serialize ring setting and start position
}
