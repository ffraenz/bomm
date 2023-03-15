//
//  main.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include <stdio.h>
#include "wiring.h"
#include "lettermask.h"
#include "enigma.h"

int main(int argc, char *argv[]) {
    printf("Hello, World.\n");
    
    // Test wiring
    char* original_wiring_string = "ekmflgdqvzntowyhxuspaibrcj";
    bomm_wiring_t* wiring = malloc(sizeof(bomm_wiring_t));
    bomm_load_wiring(wiring, original_wiring_string);
    char* wiring_string = bomm_describe_wiring(wiring);
    printf("Original wiring: %s\n", original_wiring_string);
    printf("Wiring: %s\n", wiring_string);
    free(wiring_string);
    free(wiring);
    
    // Test turnovers
    char* original_turnovers_string = "swzfhmq";
    bomm_lettermask_t turnovers;
    bomm_load_lettermask(&turnovers, original_turnovers_string);
    char* turnover_string = bomm_describe_lettermask(&turnovers);
    printf("Original turnovers: %s\n", original_turnovers_string);
    printf("Actual turnovers:   %s\n", turnover_string);
    free(turnover_string);

    // Test Enigma
    char* ciphertext = "aaaaa";
    char* expected_plaintext = "gdxtz";
    
    // Load key
    bomm_model_t* model = bomm_alloc_model_enigma_i();
    bomm_key_t* key = malloc(sizeof(bomm_key_t));
    key->model = model;
    bomm_load_wiring(&key->plugboard_wiring, "abcdefghijklmnopqrstuvwxyz");
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
    printf("Expected plaintext: %s\n", expected_plaintext);
    printf("Actual plaintext:   %s\n", actual_plaintext);
    
    free(actual_plaintext);
    free(ciphertext_message);
    free(key);
    free(model);
    
    return 0;
}
