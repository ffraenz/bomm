//
//  wiring.c
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#include <criterion/criterion.h>
#include "shared/helpers.h"
#include "../src/wiring.h"
#include "../src/utility.h"

Test(wiring, bomm_wiring_init, BOMM_TEST_DISABLE_FOR_NON_LATIN_ALPHABET) {
    char* expected_wiring_string = "ekmflgdqvzntowyhxuspaibrcj";
    char actual_string[BOMM_ALPHABET_SIZE + 1];
    bomm_wiring_t* wiring = malloc(sizeof(bomm_wiring_t));
    bomm_wiring_init(wiring, expected_wiring_string);
    bomm_wiring_stringify(actual_string, sizeof(actual_string), wiring);
    cr_assert_str_eq(actual_string, expected_wiring_string);
    free(wiring);
}

Test(wiring, bomm_wiring_plugboard_init_identity, BOMM_TEST_DISABLE_FOR_NON_LATIN_ALPHABET) {
    unsigned int plugboard[BOMM_ALPHABET_SIZE];
    bomm_wiring_plugboard_init_identity(plugboard);
    const unsigned int expected_plugboard[] = {
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
       10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
       20, 21, 22, 23, 24, 25
    };
    cr_assert_arr_eq(plugboard, expected_plugboard, sizeof(expected_plugboard));
}

Test(key, bomm_wiring_plugboard_validate) {
    // Identity is valid
    unsigned int plugboard[BOMM_ALPHABET_SIZE];
    bomm_wiring_plugboard_init_identity(plugboard);
    cr_assert_eq(bomm_wiring_plugboard_validate(plugboard), true);

    // Swapping any pair in an identity plugboard is valid
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        for (unsigned int k = i + 1; k < BOMM_ALPHABET_SIZE; k++) {
            bomm_wiring_plugboard_init_identity(plugboard);
            bomm_swap(&plugboard[i], &plugboard[k]);
            cr_assert_eq(bomm_wiring_plugboard_validate(plugboard), true);
        }
    }

    // Violating the bijective rule is invalid
    bomm_wiring_plugboard_init_identity(plugboard);
    plugboard[0] = 1;
    cr_assert_eq(bomm_wiring_plugboard_validate(plugboard), false);

    // Violating the involution rule is invalid
    bomm_wiring_plugboard_init_identity(plugboard);
    bomm_swap(&plugboard[0], &plugboard[1]);
    bomm_swap(&plugboard[1], &plugboard[2]);
    cr_assert_eq(bomm_wiring_plugboard_validate(plugboard), false);
}

Test(key, bomm_wiring_plugboard_stringify, BOMM_TEST_DISABLE_FOR_NON_LATIN_ALPHABET) {
    char actual_string[128];

    // Identity plugboard
    unsigned int plugboard[BOMM_ALPHABET_SIZE];
    bomm_wiring_plugboard_init_identity(plugboard);
    bomm_wiring_plugboard_stringify(actual_string, sizeof(actual_string), plugboard);
    cr_assert_str_eq(actual_string, "");

    // Plugboard sample
    const unsigned int example_plugboard[] = {
         0,  2,  1, 23, 16, 11,  6, 24,  8,  9,
        10,  5, 14, 25, 12, 17,  4, 15, 19,
        18, 20, 22, 21,  3,  7, 13
    };
    bomm_wiring_plugboard_stringify(actual_string, sizeof(actual_string), example_plugboard);
    cr_assert_str_eq(actual_string, "bc dx eq fl hy mo nz pr st vw");
}
