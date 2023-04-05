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
    
    size_t element_size = sizeof(unsigned int);
    unsigned int data;
    float score_boundary;
    bomm_hold_element_t* element;
    
    bomm_hold_t* hold = bomm_hold_init(element_size, 5);
    
    data = 1337;
    score_boundary = bomm_hold_add(hold, 6, &data, "Element 1337");
    // cr_assert_eq(score_boundary, -INFINITY);
    // cr_assert_eq(hold->count, 1);
    
    element = bomm_hold_at(hold, 0);
    printf("element->score = %f\n", element->score);
    // cr_assert_eq(element->score, 6);
    // cr_assert_eq(&(unsigned int*)element->data, 1337);
    // cr_assert_str_eq(element->preview, "Element 1337");
    
    
    
    bomm_hold_destroy(hold);
    
    /*
    printf("Hello, World.\n");
    
    char* original_message = "ABTWU GWDMP OGKMQ KBHGK HROUP RMYQY INHSA MWFBP CDQRG LDBFK YNXPP DIQHE AOIFQ AOLRZ ZFPDJ MCGEC TAHHQ MVUYA JIAWM WSOYU UTLEP AVZKG HJWCD LOQHW IMSTC LQDNP VCFCN FRUYR GSSJH ORQMU IFFYU WYNTA XPYIX MYTEE FTDCV EHUOA DCPLM APCAU JJYUK";
    
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
     */
    
    return 0;
}
