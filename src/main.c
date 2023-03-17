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
    
    char* original_message = "TSUNQ RTLIA BFTQR NUWLQ VNITR SCTNQ IPKLM AHEEF DCABC UWORU BSWYR BGGFD TXCBX DVDUC EZGFB KYILX AUNPQ RTUVS WM";
    
    bomm_message_t* message = bomm_alloc_message(original_message);
    
    char* message_string = bomm_describe_message(message);
    printf("Message: %s\n", message_string);
    free(message_string);
    
    printf("Length: %d\n", message->length);
    printf("IC: %f\n", bomm_message_calc_ic(message));
    
    bomm_model_t* model = bomm_alloc_model_enigma_i();
    bomm_model_attack_phase_1(model, message);
    
    // free(model);
    free(message);
    
    return 0;
}
