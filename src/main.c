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
    char* ciphertext_string = "gdbyy sgjxd xmalb myoye qchrn wefia jtzkn hwuwb zzexw njuuc riyhf covsa cueqa xowgk qnnae seojx ngeze mdefd jgkvp rnjbq waemb qskqs xqivk ogohm ferqr pimua qcqvk pvaka qzzph rbljn ajcaw vxhaj sxgtx xqfwe tytrx itcpn hbci";
    
    bomm_message_t* ciphertext = bomm_message_init(ciphertext_string);
    
    bomm_key_space_t* key_space = bomm_key_space_enigma_i_init();
    bomm_attack_key_space(key_space, ciphertext);
    
    bomm_key_space_destroy(key_space);
    
    return 0;
}
