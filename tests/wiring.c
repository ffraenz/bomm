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

unsigned int bomm_test_latin_identity_plugboard_map[26] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25
};

unsigned int bomm_test_latin_example_plugboard_map[26] = {
     0,  2,  1, 23, 16, 11,  6, 24,  8,  9,
    10,  5, 14, 25, 12, 17,  4, 15, 19, 18,
    20, 22, 21,  3,  7, 13
};

Test(wiring, bomm_wiring_init) {
    bomm_test_skip_if_non_latin_alphabet;
    char* expected_wiring_string = "ekmflgdqvzntowyhxuspaibrcj";
    char actual_string[BOMM_ALPHABET_SIZE + 1];
    bomm_wiring_t* wiring = malloc(sizeof(bomm_wiring_t));
    bomm_wiring_init(wiring, expected_wiring_string);
    bomm_wiring_stringify(actual_string, sizeof(actual_string), wiring);
    cr_assert_str_eq(actual_string, expected_wiring_string);
    free(wiring);
}

Test(wiring, bomm_wiring_plugboard_init) {
    bomm_test_skip_if_non_latin_alphabet;

    bomm_plugboard_t stack_plugboard;
    cr_assert_eq(bomm_plugboard_init(&stack_plugboard, ""), &stack_plugboard);
    cr_assert_arr_eq(
        stack_plugboard.map,
        bomm_test_latin_identity_plugboard_map,
        sizeof(bomm_test_latin_identity_plugboard_map)
    );

    cr_assert_eq(bomm_plugboard_init(&stack_plugboard, "bc dx eq fl hy mo nz pr st vw"), &stack_plugboard);
    cr_assert_arr_eq(
        stack_plugboard.map,
        bomm_test_latin_example_plugboard_map,
        sizeof(bomm_test_latin_example_plugboard_map)
    );

    cr_expect_eq(bomm_plugboard_init(NULL, "abc"), NULL);
    bomm_plugboard_t* heap_plugboard = bomm_plugboard_init(NULL, "bc dx eq fl hy mo nz pr st vw");
    cr_assert_neq(heap_plugboard, NULL);
    cr_assert_arr_eq(
        heap_plugboard->map,
        bomm_test_latin_example_plugboard_map,
        sizeof(bomm_test_latin_example_plugboard_map)
    );
}

Test(wiring, bomm_wiring_plugboard_init_identity) {
    bomm_plugboard_t stack_plugboard;
    cr_assert_eq(bomm_plugboard_init_identity(&stack_plugboard), &stack_plugboard);
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        cr_expect_eq(stack_plugboard.map[i], i);
    }

    bomm_plugboard_t* heap_plugboard = bomm_plugboard_init_identity(NULL);
    cr_assert_neq(heap_plugboard, NULL);
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        cr_expect_eq(heap_plugboard->map[i], i);
    }
}

Test(key, bomm_wiring_plugboard_validate) {
    // Identity is valid
    bomm_plugboard_t plugboard;
    bomm_plugboard_init_identity(&plugboard);
    cr_assert_eq(bomm_plugboard_validate(&plugboard), true);

    // Swapping any pair in an identity plugboard is valid
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        for (unsigned int k = i + 1; k < BOMM_ALPHABET_SIZE; k++) {
            bomm_plugboard_init_identity(&plugboard);
            bomm_swap(&plugboard.map[i], &plugboard.map[k]);
            cr_assert_eq(bomm_plugboard_validate(&plugboard), true);
        }
    }

    // Violating the bijective rule is invalid
    bomm_plugboard_init_identity(&plugboard);
    plugboard.map[0] = 1;
    cr_assert_eq(bomm_plugboard_validate(&plugboard), false);

    // Violating the involution rule is invalid
    bomm_plugboard_init_identity(&plugboard);
    bomm_swap(&plugboard.map[0], &plugboard.map[1]);
    bomm_swap(&plugboard.map[1], &plugboard.map[2]);
    cr_assert_eq(bomm_plugboard_validate(&plugboard), false);
}

Test(key, bomm_wiring_plugboard_stringify) {
    bomm_test_skip_if_non_latin_alphabet;
    char actual_string[128];

    // Identity plugboard
    bomm_plugboard_t identity;
    bomm_plugboard_init_identity(&identity);
    bomm_plugboard_stringify(actual_string, sizeof(actual_string), &identity);
    cr_assert_str_eq(actual_string, "");

    // Example plugboard
    bomm_plugboard_t example;
    memcpy(
        example.map,
        bomm_test_latin_example_plugboard_map,
        sizeof(bomm_test_latin_example_plugboard_map)
    );
    bomm_plugboard_stringify(actual_string, sizeof(actual_string), &example);
    cr_assert_str_eq(actual_string, "bc dx eq fl hy mo nz pr st vw");
}
