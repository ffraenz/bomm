//
//  enigma.c
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#include <criterion/criterion.h>
#include "../src/enigma.h"

void _load_test_key(bomm_key_t* key) {
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);
    bomm_key_init(key, &key_space);

    unsigned int wheel_indices[] = {0, 0, 1, 2, 0};
    for (unsigned int slot = 0; slot < 5; slot++) {
        memcpy(&key->wheels[slot], &key_space.wheel_sets[slot][wheel_indices[slot]], sizeof(bomm_wheel_t));
    }

    key->positions[1] = 12;
    key->positions[2] = 4;
    key->positions[3] = 20;
}

Test(enigma, bomm_enigma_encrypt) {
    bomm_key_t key, original_key;
    _load_test_key(&key);
    memcpy(&original_key, &key, sizeof(original_key));

    char key_string[128];
    bomm_key_stringify(key_string, 128, &key);
    cr_assert_str_eq(key_string, "UKW-B,I,II,III,ETW-ABC aaaaa ameua ");

    bomm_message_t* ciphertext = bomm_message_init("aaaaa");
    bomm_message_t* plaintext = bomm_message_init_with_length(ciphertext->length);

    bomm_enigma_encrypt(ciphertext, &key, plaintext);
    cr_assert_arr_eq(&key, &original_key, sizeof(original_key));

    char actual_plaintext_string[bomm_message_serialize_size(plaintext)];
    bomm_message_stringify(actual_plaintext_string, -1, plaintext);
    cr_assert_str_eq(actual_plaintext_string, "gdxtz");

    free(ciphertext);
    free(plaintext);
}

Test(enigma, bomm_enigma_generate_scrambler) {
    char expected_scrambler_ascii[5][27] = {
        "gmixryalcwzhbvstueopqnjdfk",
        "dsnalzyomrxeichtvjbpwqukgf",
        "xjyeduhgvbozpwkmstqrfinacl",
        "teqkbxolrpdhuzgjciwamysfvn",
        "zmdcpgfntlsjbhreyokivuxwqa"
    };

    bomm_key_t key, original_key;
    _load_test_key(&key);
    memcpy(&original_key, &key, sizeof(original_key));

    bomm_message_t* ciphertext = bomm_message_init("aaaaa");

    bomm_scrambler_t* scrambler = malloc(bomm_scrambler_size(ciphertext->length));
    scrambler->length = ciphertext->length;
    bomm_enigma_generate_scrambler(scrambler, &key);
    cr_assert_arr_eq(&key, &original_key, sizeof(original_key));

    for (unsigned int i = 0; i < ciphertext->length; i++) {
        for (unsigned int j = 0; j < BOMM_ALPHABET_SIZE; j++) {
            char actual_ascii = bomm_message_letter_to_ascii(scrambler->map[i][j]);
            cr_assert_eq(actual_ascii, expected_scrambler_ascii[i][j]);
        }
    }

    free(ciphertext);
    free(scrambler);
}
