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
    
    char* original_message = "iower tnjkn xaqal ovfme tyxgm xultn ixfxr mmcpb rkilc fqyvb etmvj mzhzi lgdoj xmldd lvskq hznkg ghuni cjqkm mclvv hxhuq ujhjv casbj sxnsq okfvq vrhbm jhxky vpolm gsmvj yjrbx ltidg fshxc gbstq gxacw olupr fsfzy ikgzp dajog cwzrf lssgm sjjvk kngio jtusr xonjx qqqbs nbsma qegjz mhuka wxnsm igeod vxmsl ntabz hzxgk dkapr sryaq zkklv kzgdx nyaki sgduh vinbz rwjwn gbwlq vxofd fisvk ovucv bvlul lwoxz iwhvl rbxbc yqyzu jwanz leigk gnfsf zvbnt agmyc pshqi lspfx zkzen humcc uvvvx wvzgr gsdka yhpvo wjheu gwcoe gujwj mqfid ozwgv dymql vvilq njomj vgvux rzbyh rorbd bipcm uhtvf rjqgl mxvvv rdzvb frkym wpqic hrgys ypuvq jjyjz svfsg subuv bzsul vaite znkjq wuueb pnatv wzsab k";
    
    bomm_message_t* message = bomm_alloc_message(original_message);
    
    char message_string[bomm_message_serialize_size(message)];
    bomm_message_serialize(message_string, -1, message);
    printf("Message: %s\n", message_string);
    
    printf("Length: %d\n", message->length);
    printf("IC: %f\n", bomm_message_calc_ic(message));
    
    bomm_model_t* model = bomm_alloc_model_enigma_i();
    bomm_model_attack_phase_1(model, message);
    
    free(model);
    free(message);
    
    return 0;
}
