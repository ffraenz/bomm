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
    memcpy(key->plugboard, &bomm_key_plugboard_identity, sizeof(bomm_letter_t) * BOMM_ALPHABET_SIZE);
    
    for (int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        key->plugboard[i] = i;
    }
    
    key->wheels[0] = 0; // UKW-B
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
    cr_assert_str_eq(key_string, "UKW-B,I,II,III,ABC aaaaa ameua                                        ");
    
    char actual_plaintext[bomm_message_serialize_size(plaintext_message)];
    bomm_message_serialize(actual_plaintext, -1, plaintext_message);
    cr_assert_str_eq(actual_plaintext, expected_plaintext);
    
    free(ciphertext_message);
    free(key);
    free(model);
}

Test(enigma, bomm_scrambler_generate) {
    char* ciphertext_string = "aaaaa";
    char expected_scrambler_ascii[5][27] = {
        "gmixryalcwzhbvstueopqnjdfk",
        "dsnalzyomrxeichtvjbpwqukgf",
        "xjyeduhgvbozpwkmstqrfinacl",
        "teqkbxolrpdhuzgjciwamysfvn",
        "zmdcpgfntlsjbhreyokivuxwqa"
    };
    
    bomm_key_t* key = malloc(sizeof(bomm_key_t));
    key->model = bomm_model_alloc_enigma_i();
    memcpy(key->plugboard, &bomm_key_plugboard_identity, sizeof(bomm_letter_t) * BOMM_ALPHABET_SIZE);
    
    key->wheels[0] = 0; // UKW-B
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
    
    bomm_message_t* ciphertext = bomm_message_alloc(ciphertext_string);
    
    bomm_scrambler_t* scrambler = malloc(bomm_scrambler_size(ciphertext->length));
    scrambler->length = ciphertext->length;
    bomm_scrambler_generate(scrambler, key);
    
    for (unsigned int i = 0; i < ciphertext->length; i++) {
        for (unsigned int j = 0; j < BOMM_ALPHABET_SIZE; j++) {
            char actual_ascii = bomm_message_letter_to_ascii(scrambler->map[i][j]);
            cr_assert_eq(actual_ascii, expected_scrambler_ascii[i][j]);
        }
    }
    
    free(ciphertext);
    free(scrambler);
    free(key->model);
    free(key);
}
