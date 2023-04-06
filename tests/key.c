//
//  key.c
//  Bomm
//
//  Created by Fränz Friederes on 23/03/2023.
//

#include <criterion/criterion.h>
#include "../src/key.h"

Test(key, bomm_key_serialize_plugboard) {
    char actual_plugboard_string[39];
    char* expected_plugboard_string;
    bomm_key_t key;
    
    // Identity plugboard
    expected_plugboard_string = "";
    memcpy(&key.plugboard, &bomm_key_plugboard_identity, sizeof(bomm_key_plugboard_identity));
    bomm_key_serialize_plugboard(actual_plugboard_string, 39, &key);
    cr_assert_str_eq(actual_plugboard_string, expected_plugboard_string);
    
    // Plugboard sample
    expected_plugboard_string = "bc dx eq fl hy mo nz pr st vw";
    bomm_letter_t example_plugboard[] = {
         0,  2,  1, 23, 16, 11,  6, 24,  8,  9,
        10,  5, 14, 25, 12, 17,  4, 15, 19,
        18, 20, 22, 21,  3,  7, 13
    };
    memcpy(&key.plugboard, &example_plugboard, sizeof(bomm_letter_t) * BOMM_ALPHABET_SIZE);
    bomm_key_serialize_plugboard(actual_plugboard_string, 39, &key);
    cr_assert_str_eq(actual_plugboard_string, expected_plugboard_string);
}
