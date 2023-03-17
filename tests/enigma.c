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
    
    bomm_model_t* model = bomm_alloc_model_enigma_i();
    bomm_key_t* key = malloc(sizeof(bomm_key_t));
    key->model = model;
    bomm_wiring_extract(&key->plugboard_wiring, "abcdefghijklmnopqrstuvwxyz");
    key->slot_rotor[0] = 7; // UKW-B
    key->slot_rotor[1] = 0; // I
    key->slot_rotor[2] = 1; // II
    key->slot_rotor[3] = 2; // III
    key->slot_rotor[4] = 5; // ETW-ABC
    key->slot_positions[0] = 0;
    key->slot_positions[1] = 12;
    key->slot_positions[2] = 4;
    key->slot_positions[3] = 20;
    key->slot_positions[4] = 0;
    key->slot_rings[0] = 0;
    key->slot_rings[1] = 0;
    key->slot_rings[2] = 0;
    key->slot_rings[3] = 0;
    key->slot_rings[4] = 0;
    
    bomm_message_t* ciphertext_message = bomm_alloc_message(ciphertext);
    bomm_message_t* plaintext_message = bomm_alloc_message_with_length(ciphertext_message->length);
    bomm_model_encrypt(ciphertext_message, key, plaintext_message);
    
    char* actual_plaintext = bomm_describe_message(plaintext_message);
    cr_assert_str_eq(actual_plaintext, expected_plaintext);
    
    free(actual_plaintext);
    free(ciphertext_message);
    free(key);
    free(model);
}
