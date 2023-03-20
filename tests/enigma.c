//
//  enigma.c
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#include <criterion/criterion.h>
#include "../src/enigma.h"

Test(enigma, bomm_model_encrypt) {
    char* ciphertext = "aaaaa";
    char* expected_plaintext = "gdxtz";
    
    bomm_model_t* model = bomm_model_alloc_enigma_i();
    bomm_key_t* key = malloc(sizeof(bomm_key_t));
    key->model = model;
    bomm_wiring_extract(&key->plugboard, BOMM_WIRING_IDENTITY);
    key->wheels[0] = 1; // UKW-B
    key->wheels[1] = 0; // I
    key->wheels[2] = 1; // II
    key->wheels[3] = 2; // III
    key->wheels[4] = 0; // ETW-ABC
    key->rings[0] = 0;
    key->rings[1] = 0;
    key->rings[2] = 0;
    key->rings[3] = 0;
    key->rings[4] = 0;
    key->positions[0] = 0;
    key->positions[1] = 12;
    key->positions[2] = 4;
    key->positions[3] = 20;
    key->positions[4] = 0;
    
    bomm_message_t* ciphertext_message = bomm_message_alloc(ciphertext);
    bomm_message_t* plaintext_message = bomm_message_alloc_length(ciphertext_message->length);
    bomm_encrypt(ciphertext_message, key, plaintext_message);
    
    char key_string[128];
    bomm_key_serialize(key_string, 128, key);
    cr_assert_str_eq(key_string, "UKW-B,I,II,III,ABC aaaaa ameua");
    
    char actual_plaintext[bomm_message_serialize_size(plaintext_message)];
    bomm_message_serialize(actual_plaintext, -1, plaintext_message);
    cr_assert_str_eq(actual_plaintext, expected_plaintext);
    
    free(ciphertext_message);
    free(key);
    free(model);
}
