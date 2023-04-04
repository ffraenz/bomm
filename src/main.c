//
//  main.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include <stdio.h>
#include "attack.h"
#include "message.h"
#include "measure.h"

int main(void) {
    printf("Hello, World.\n");
    
    char* original_message = "PBNXA SMDAX NOOYH RCZGV VZCBI GIBGW HMXKR RVQCF JCZPT UNSWA DDSTI GQQCS AGPKR XXLOM GFXAP HHMRF SDKYT MYPMV ROHAS QYRWF WVAVG CCUDB IBXXD YZSAC JSYOT MWUCN WOMHH JPYWD CCLUP GSWCL MBCZS SYXPG MGMQX AUFUL NOZEQ ENHEI ZZAKL C";
    
    bomm_message_t* message = bomm_message_alloc(original_message);
    
    char message_string[bomm_message_serialize_size(message)];
    bomm_message_serialize(message_string, -1, message);
    printf("Message: %s\n", message_string);
    
    printf("Length: %d\n", message->length);
    printf("IC: %f\n", bomm_measure_ic(message->frequency, message->length, BOMM_ALPHABET_SIZE));
    
    bomm_model_t* model = bomm_model_alloc_enigma_i();
    bomm_attack_phase_1(model, message);
    
    free(model);
    free(message);
    
    return 0;
}
