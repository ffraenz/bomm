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
     * Pointer to the model holding the rotor specs
     */
    bomm_model_t* model;
    
    /**
     * Wheel order (Walzenlage): Chosen rotor index for each slot
     */
    bomm_rotor_index_t slot_rotor[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Ring setting (Ringstellung): Ring position of the rotor in each slot
     */
    bomm_letter_t slot_rings[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Start position (Walzenstellung): Start position of the rotor in each slot
     */
    bomm_letter_t slot_positions[BOMM_MODEL_MAX_SLOT_COUNT];
    
    /**
     * Plugboard setting (Steckerverbindungen) to be used
     */
    bomm_wiring_t plugboard_wiring;
} bomm_key_t;

#endif /* key_h */
