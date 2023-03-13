//
//  key.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef key_h
#define key_h

#include <stdio.h>
#include "model.h"

typedef struct _bomm_key {
    /**
     * Pointer to the model
     */
    bomm_model_t* model;
    
    /**
     * Plugboard wiring (Steckbrett) to be used
     */
    bomm_wiring_t plugboard_wiring;
    
    /**
     * Wheel selection and order (Walzenlage): Chosen rotor index for each slot
     */
    bomm_rotor_index_t slot_rotor[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Message setting: Initial position of the rotor in each slot
     */
    bomm_letter_t slot_positions[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Ring setting (Ringstellung): Ring position of the rotor in each slot
     */
    bomm_letter_t slot_rings[BOMM_MODEL_MAX_SLOT_COUNT];
} bomm_key_t;

#endif /* key_h */
