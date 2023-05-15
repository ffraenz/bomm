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

Test(key, bomm_key_iterator_next) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();
    key_space->plug_mask = BOMM_LETTERMASK_NONE;
    bomm_key_iterator_t key_iterator;
    bomm_key_iterator_init(&key_iterator, key_space);

    bomm_key_iterator_t expected_key_iterator;
    memcpy(&expected_key_iterator, &key_iterator, sizeof(key_iterator));

    unsigned int count = 1;
    while (!bomm_key_iterator_next(&key_iterator)) {
        count++;
    }

    cr_assert_eq(count, 26364000);
    cr_assert_arr_eq(&key_iterator, &expected_key_iterator, sizeof(key_iterator));
    free(key_space);
}

Test(key, bomm_key_iterator_plugboard_next_r_stecker) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();
    bomm_key_iterator_t iterator, initial_iterator;
    char string[8];

    // Configure the key space with no plugboard exhaustion
    key_space->plug_mask = BOMM_LETTERMASK_NONE;

    // Such a plugboard iterator must yield the empty plugboard only
    cr_assert_eq(bomm_key_iterator_init(&iterator, key_space), &iterator);
    memcpy(&initial_iterator, &iterator, sizeof(iterator));
    bomm_key_plugboard_stringify(string, sizeof(string), &iterator.key);
    cr_assert_str_eq(string, "");
    cr_assert_eq(bomm_key_iterator_plugboard_next(&iterator), true);
    cr_assert_arr_eq(&iterator, &initial_iterator, sizeof(iterator));

    // Configure a key space with R-Stecker plugboard exhaustion
    key_space->plug_mask = 0x22010;

    // For such a key space the following plugboard configurations are expected
    const char expected_strings[72][128] = {
        "ae", "an", "ar", "be", "bn", "br", "ce", "cn", "cr",
        "de", "dn", "dr", "ef", "eg", "eh", "ei", "ej", "ek",
        "el", "em", "en", "eo", "ep", "eq", "er", "es", "et",
        "eu", "ev", "ew", "ex", "ey", "ez", "fn", "fr", "gn",
        "gr", "hn", "hr", "in", "ir", "jn", "jr", "kn", "kr",
        "ln", "lr", "mn", "mr", "no", "np", "nq", "nr", "ns",
        "nt", "nu", "nv", "nw", "nx", "ny", "nz", "or", "pr",
        "qr", "rs", "rt", "ru", "rv", "rw", "rx", "ry", "rz"
    };

    cr_assert_eq(bomm_key_iterator_init(&iterator, key_space), &iterator);
    memcpy(&initial_iterator, &iterator, sizeof(iterator));

    bomm_key_plugboard_stringify(string, sizeof(string), &iterator.key);
    cr_assert_str_eq(string, "");

    for (unsigned int i = 0; i < 72; i++) {
        cr_assert_eq(bomm_key_iterator_plugboard_next(&iterator), false);
        bomm_key_plugboard_stringify(string, sizeof(string), &iterator.key);
        cr_assert_str_eq(string, expected_strings[i]);
    }

    cr_assert_eq(bomm_key_iterator_plugboard_next(&iterator), true);
    cr_assert_arr_eq(&iterator, &initial_iterator, sizeof(iterator));

    free(key_space);
}

Test(key, bomm_key_space_plugboard_count) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();

    // No plugboard exhaustion (identity only)
    key_space->plug_mask = BOMM_LETTERMASK_NONE;
    cr_assert_eq(bomm_key_space_plugboard_count(key_space), 1);

    // E-Stecker exhaustion
    key_space->plug_mask = 0x10;
    cr_assert_eq(bomm_key_space_plugboard_count(key_space), 26);

    // R-Stecker exhaustion
    key_space->plug_mask = 0x22010;
    cr_assert_eq(bomm_key_space_plugboard_count(key_space), 73);

    // I-Stecker exhaustion
    key_space->plug_mask = 0x862110;
    cr_assert_eq(bomm_key_space_plugboard_count(key_space), 136);

    // Solo exhaustion
    key_space->plug_mask = BOMM_LETTERMASK_ALL;
    cr_assert_eq(bomm_key_space_plugboard_count(key_space), 326);

    free(key_space);
}

Test(key, bomm_key_space_count) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();

    key_space->plug_mask = BOMM_LETTERMASK_NONE;
    cr_assert_eq(bomm_key_space_count(key_space), 26364000);

    key_space->plug_mask = 0x862110;
    cr_assert_eq(bomm_key_space_count(key_space), 3585504000);

    key_space->plug_mask = BOMM_LETTERMASK_ALL;
    cr_assert_eq(bomm_key_space_count(key_space), 8594664000);

    free(key_space);
}

Test(key, bomm_key_space_slice) {
    bomm_key_space_t* key_space = bomm_key_space_init_enigma_i();
    key_space->plug_mask = 0x10;
    key_space->position_masks[1] = BOMM_LETTERMASK_FIRST;
    key_space->position_masks[2] = BOMM_LETTERMASK_FIRST;

    // Expected count: 26 plugboards * 60 wheel orders * 26 rings * 26 positions
    unsigned long expected_count = 1054560;

    bomm_key_iterator_t key_space_iterator;
    bomm_key_iterator_init(&key_space_iterator, key_space);

    // Test a single split, 2 splits (even splits), and 3 splits (odd splits)
    for (unsigned int split_count = 1; split_count < 4; split_count++) {
        bomm_key_space_t key_space_slices[split_count];
        bomm_key_space_slice(key_space, split_count, key_space_slices);

        bool key_space_end = false;
        bool slice_end = false;
        unsigned long count = 0;

        for (unsigned int i = 0; i < split_count; i++) {
            bomm_key_iterator_t slice_iterator;
            bomm_key_iterator_init(&slice_iterator, &key_space_slices[i]);

            do {
                // Iterating the elements of each slice is expected to be
                // equivalent to iterating the elements of the entire key space
                cr_assert_eq(
                    memcmp(
                        &key_space_iterator.key,
                        &slice_iterator.key,
                        sizeof(bomm_key_t)
                    ),
                    0,
                    "Key at index %lu is not equivalent to key from slice",
                    count
                );
                count++;
                key_space_end = bomm_key_iterator_next(&key_space_iterator);
                slice_end = bomm_key_iterator_next(&slice_iterator);
            } while (!slice_end && !key_space_end);
        }

        // Iterating slice is expected to be complete
        cr_assert_eq(key_space_end, true);
        cr_assert_eq(slice_end, true);
        cr_assert_eq(count, expected_count);
    }

    free(key_space);
}
