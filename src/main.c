//
//  main.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include <stdio.h>
#include "enigma.h"
#include "message.h"


int main() {
    printf("Hello, World.\n");
    
    bomm_message_t* message = bomm_alloc_message("Hello, World.");
    bomm_model_t* model = bomm_alloc_model_enigma_i();
    bomm_model_attack_phase_1(model, message);
    
    free(model);
    free(message);
    
    return 0;
}
