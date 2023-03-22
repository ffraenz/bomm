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

int main() {
    printf("Hello, World.\n");
    
    unsigned char* ngram_map = bomm_measure_ngram_map_alloc(2, "/Users/ff/Projects/Bachelor/bomm/data/1941-bigram.txt");
    free(ngram_map);
    
    ngram_map = bomm_measure_ngram_map_alloc(3, "/Users/ff/Projects/Bachelor/bomm/data/1941-trigram.txt");
    free(ngram_map);
    
    /*
    char* original_message = "iower tnjkn xaqal ovfme tyxgm xultn ixfxr mmcpb rkilc fqyvb etmvj mzhzi lgdoj xmldd lvskq hznkg ghuni cjqkm mclvv hxhuq ujhjv casbj sxnsq okfvq vrhbm jhxky vpolm gsmvj yjrbx ltidg fshxc gbstq gxacw olupr fsfzy ikgzp dajog cwzrf lssgm sjjvk kngio jtusr xonjx qqqbs nbsma qegjz mhuka wxnsm igeod vxmsl ntabz hzxgk dkapr sryaq zkklv kzgdx nyaki sgduh vinbz rwjwn gbwlq vxofd fisvk ovucv bvlul lwoxz iwhvl rbxbc yqyzu jwanz leigk gnfsf zvbnt agmyc pshqi lspfx zkzen humcc uvvvx wvzgr gsdka yhpvo wjheu gwcoe gujwj mqfid ozwgv dymql vvilq njomj vgvux rzbyh rorbd bipcm uhtvf rjqgl mxvvv rdzvb frkym wpqic hrgys ypuvq jjyjz svfsg subuv bzsul vaite znkjq wuueb pnatv wzsab k";
    
    bomm_message_t* message = bomm_message_alloc(original_message);
    
    char message_string[bomm_message_serialize_size(message)];
    bomm_message_serialize(message_string, -1, message);
    printf("Message: %s\n", message_string);
    
    printf("Length: %d\n", message->length);
    printf("IC: %f\n", bomm_message_calc_ic(message));
    
    bomm_model_t* model = bomm_model_alloc_enigma_i();
    bomm_attack_phase_1(model, message);
    
    free(model);
    free(message);
    */
    
    return 0;
}
