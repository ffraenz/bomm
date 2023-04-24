//
//  key.c
//  Bomm
//
//  Created by Fränz Friederes on 23/03/2023.
//

#include <criterion/criterion.h>
#include "../src/key.h"

Test(key, bomm_key_plugboard_stringify) {
    char actual_plugboard_string[39];
    char* expected_plugboard_string;
    bomm_key_t key;

    // Identity plugboard
    expected_plugboard_string = "";
    memcpy(&key.plugboard, &bomm_key_plugboard_identity, sizeof(bomm_key_plugboard_identity));
    bomm_key_plugboard_stringify(actual_plugboard_string, 39, &key);
    cr_assert_str_eq(actual_plugboard_string, expected_plugboard_string);

    // Plugboard sample
    expected_plugboard_string = "bc dx eq fl hy mo nz pr st vw";
    unsigned int example_plugboard[] = {
         0,  2,  1, 23, 16, 11,  6, 24,  8,  9,
        10,  5, 14, 25, 12, 17,  4, 15, 19,
        18, 20, 22, 21,  3,  7, 13
    };
    memcpy(&key.plugboard, &example_plugboard, sizeof(example_plugboard));
    bomm_key_plugboard_stringify(actual_plugboard_string, 39, &key);
    cr_assert_str_eq(actual_plugboard_string, expected_plugboard_string);
}

Test(key, bomm_key_mechanism_from_string) {
    bomm_mechanism_t actual_mechanism;
    actual_mechanism = bomm_key_mechanism_from_string("stepping");
    cr_assert_eq(actual_mechanism, BOMM_MECHANISM_STEPPING);
    actual_mechanism = bomm_key_mechanism_from_string("odometer");
    cr_assert_eq(actual_mechanism, BOMM_MECHANISM_ODOMETER);
    actual_mechanism = bomm_key_mechanism_from_string("none");
    cr_assert_eq(actual_mechanism, BOMM_MECHANISM_NONE);
}

Test(key, bomm_key_iterator_init_empty_wheel_order) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();
    bomm_key_iterator_t key_iterator;
    key_space->wheel_sets[1][1] = NULL;
    key_space->wheel_sets[2][1] = NULL;
    cr_assert_eq(bomm_key_iterator_init(&key_iterator, key_space), NULL);
    free(key_space);
}

Test(key, bomm_key_iterator_init_empty_wheel_set) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();
    bomm_key_iterator_t key_iterator;
    key_space->wheel_sets[1][0] = NULL;
    cr_assert_eq(bomm_key_iterator_init(&key_iterator, key_space), NULL);
    free(key_space);
}

Test(key, bomm_key_iterator_init_empty_position_mask) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();
    bomm_key_iterator_t key_iterator;
    key_space->position_masks[1] = BOMM_LETTERMASK_NONE;
    cr_assert_eq(bomm_key_iterator_init(&key_iterator, key_space), NULL);
    free(key_space);
}

Test(key, bomm_key_iterator_count) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();
    bomm_key_iterator_t key_iterator;
    bomm_key_iterator_t expected_key_iterator;
    bomm_key_iterator_init(&key_iterator, key_space);
    memcpy(&expected_key_iterator, &key_iterator, sizeof(key_iterator));
    cr_assert_eq(bomm_key_iterator_count(&key_iterator), 79092000);
    cr_assert_arr_eq(&key_iterator, &expected_key_iterator, sizeof(key_iterator));
    free(key_space);
}
