//
//  key.c
//  Bomm
//
//  Created by Fränz Friederes on 23/03/2023.
//

#include <criterion/criterion.h>
#include "../src/key.h"

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
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);
    bomm_key_iterator_t key_iterator;
    key_space.wheel_sets[1][1].name[0] = '\0';
    key_space.wheel_sets[2][1].name[0] = '\0';
    cr_assert_eq(bomm_key_iterator_init(&key_iterator, &key_space), NULL);
}

Test(key, bomm_key_iterator_init_empty_wheel_set) {
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);
    bomm_key_iterator_t key_iterator;
    key_space.wheel_sets[1][0].name[0] = '\0';
    cr_assert_eq(bomm_key_iterator_init(&key_iterator, &key_space), NULL);
}

Test(key, bomm_key_iterator_init_empty_position_mask) {
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);
    bomm_key_iterator_t key_iterator;
    key_space.position_masks[1] = BOMM_LETTERMASK_NONE;
    cr_assert_eq(bomm_key_iterator_init(&key_iterator, &key_space), NULL);
}

Test(key, bomm_key_iterator_next) {
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);
    key_space.plug_mask = BOMM_LETTERMASK_NONE;
    bomm_key_iterator_t key_iterator;
    bomm_key_iterator_init(&key_iterator, &key_space);

    bomm_key_iterator_t expected_key_iterator;
    memcpy(&expected_key_iterator, &key_iterator, sizeof(key_iterator));

    unsigned int num_keys = 1;
    while (!bomm_key_iterator_next(&key_iterator)) {
        num_keys++;
    }

    cr_assert_eq(num_keys, 26364000);
    cr_assert_arr_eq(&key_iterator, &expected_key_iterator, sizeof(key_iterator));
}

Test(key, bomm_key_iterator_plugboard_next_r_stecker) {
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);
    bomm_key_iterator_t iterator, initial_iterator;
    char string[8];

    // Configure the key space with no plugboard exhaustion
    key_space.plug_mask = BOMM_LETTERMASK_NONE;

    // Such a plugboard iterator must yield the empty plugboard only
    cr_assert_eq(bomm_key_iterator_init(&iterator, &key_space), &iterator);
    memcpy(&initial_iterator, &iterator, sizeof(iterator));
    bomm_wiring_plugboard_stringify(string, sizeof(string), iterator.key.plugboard);
    cr_assert_str_eq(string, "");
    cr_assert_eq(bomm_key_iterator_plugboard_next(&iterator), true);
    cr_assert_arr_eq(&iterator, &initial_iterator, sizeof(iterator));

    // Configure a key space with R-Stecker plugboard exhaustion
    key_space.plug_mask = 0x22010;

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

    cr_assert_eq(bomm_key_iterator_init(&iterator, &key_space), &iterator);
    memcpy(&initial_iterator, &iterator, sizeof(iterator));

    bomm_wiring_plugboard_stringify(string, sizeof(string), iterator.key.plugboard);
    cr_assert_str_eq(string, "");

    for (unsigned int i = 0; i < 72; i++) {
        cr_assert_eq(bomm_key_iterator_plugboard_next(&iterator), false);
        bomm_wiring_plugboard_stringify(string, sizeof(string), iterator.key.plugboard);
        cr_assert_str_eq(string, expected_strings[i]);
    }

    cr_assert_eq(bomm_key_iterator_plugboard_next(&iterator), true);
    cr_assert_arr_eq(&iterator, &initial_iterator, sizeof(iterator));
}

Test(key, bomm_key_space_plugboard_count) {
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);

    // No plugboard exhaustion (identity only)
    key_space.plug_mask = BOMM_LETTERMASK_NONE;
    cr_assert_eq(bomm_key_space_plugboard_count(&key_space), 1);

    // E-Stecker exhaustion
    key_space.plug_mask = 0x10;
    cr_assert_eq(bomm_key_space_plugboard_count(&key_space), 26);

    // R-Stecker exhaustion
    key_space.plug_mask = 0x22010;
    cr_assert_eq(bomm_key_space_plugboard_count(&key_space), 73);

    // I-Stecker exhaustion
    key_space.plug_mask = 0x862110;
    cr_assert_eq(bomm_key_space_plugboard_count(&key_space), 136);

    // Solo exhaustion
    key_space.plug_mask = BOMM_LETTERMASK_ALL;
    cr_assert_eq(bomm_key_space_plugboard_count(&key_space), 326);
}

Test(key, bomm_key_space_count) {
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);

    key_space.plug_mask = BOMM_LETTERMASK_NONE;
    cr_assert_eq(bomm_key_space_count(&key_space), 26364000);

    key_space.plug_mask = 0x862110;
    cr_assert_eq(bomm_key_space_count(&key_space), 3585504000);

    key_space.plug_mask = BOMM_LETTERMASK_ALL;
    cr_assert_eq(bomm_key_space_count(&key_space), 8594664000);
}

Test(key, bomm_key_space_slice) {
    bomm_key_space_t key_space;
    bomm_key_space_init_enigma_i(&key_space);

    key_space.plug_mask = 0x10;
    key_space.position_masks[1] = BOMM_LETTERMASK_FIRST;
    key_space.position_masks[2] = BOMM_LETTERMASK_FIRST;

    // Expected count: 26 plugboards * 60 wheel orders * 26 rings * 26 positions
    unsigned long num_keys_expected = 1054560;

    bomm_key_iterator_t key_space_iterator;
    bomm_key_iterator_init(&key_space_iterator, &key_space);

    // Test a single split, 2 splits (even splits), and 3 splits (odd splits)
    for (unsigned int num_slices = 1; num_slices < 4; num_slices++) {
        bomm_key_space_t key_space_slices[num_slices];
        bomm_key_space_slice(&key_space, num_slices, key_space_slices);

        bool key_space_end = false;
        bool slice_end = false;
        unsigned long num_keys = 0;

        for (unsigned int i = 0; i < num_slices; i++) {
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
                    num_keys
                );
                num_keys++;
                key_space_end = bomm_key_iterator_next(&key_space_iterator);
                slice_end = bomm_key_iterator_next(&slice_iterator);
            } while (!slice_end && !key_space_end);
        }

        // Iterating slice is expected to be complete
        cr_assert_eq(key_space_end, true);
        cr_assert_eq(slice_end, true);
        cr_assert_eq(num_keys, num_keys_expected);
    }
}
