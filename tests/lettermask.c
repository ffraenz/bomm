//
//  lettermask.c
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#include <criterion/criterion.h>
#include "../src/lettermask.h"

Test(message, bomm_lettermask_t) {
    cr_assert_geq(sizeof(bomm_lettermask_t) * 8, BOMM_ALPHABET_SIZE,
        "The alphabet is expected to fit into a lettermask");
}

Test(lettermask, bomm_lettermask_extract) {
    bomm_lettermask_t lettermask;

    bomm_lettermask_extract(&lettermask, "");
    cr_assert_eq(lettermask, BOMM_LETTERMASK_NONE);

    bomm_lettermask_extract(&lettermask, "swzfhmq");
    cr_assert_eq(lettermask, 0x024510a0);

    bomm_lettermask_extract(&lettermask, "abcdefghijklmnopqrstuvwxyz");
    cr_assert_eq(lettermask, BOMM_LETTERMASK_ALL);
}

Test(lettermask, bomm_lettermask_serialize) {
    bomm_lettermask_t lettermask;
    char actual_string[BOMM_ALPHABET_SIZE + 1];

    lettermask = BOMM_LETTERMASK_NONE;
    bomm_lettermask_serialize(actual_string, sizeof(actual_string), &lettermask);
    cr_assert_str_eq(actual_string, "");

    lettermask = 0x024510a0;
    bomm_lettermask_serialize(actual_string, sizeof(actual_string), &lettermask);
    cr_assert_str_eq(actual_string, "fhmqswz");

    lettermask = BOMM_LETTERMASK_ALL;
    bomm_lettermask_serialize(actual_string, sizeof(actual_string), &lettermask);
    cr_assert_str_eq(actual_string, "abcdefghijklmnopqrstuvwxyz");
}

Test(lettermask, bomm_lettermask_set) {
    bomm_lettermask_t lettermask = BOMM_LETTERMASK_NONE;

    bomm_lettermask_set(&lettermask, 0);
    cr_assert_eq(lettermask, 1);

    bomm_lettermask_set(&lettermask, 11);
    cr_assert_eq(lettermask, 0x00000801);

    bomm_lettermask_set(&lettermask, 25);
    cr_assert_eq(lettermask, 0x02000801);
}

Test(lettermask, bomm_lettermask_clear) {
    bomm_lettermask_t lettermask = BOMM_LETTERMASK_ALL;

    bomm_lettermask_clear(&lettermask, 0);
    cr_assert_eq(lettermask, 0x03fffffe);

    bomm_lettermask_clear(&lettermask, 11);
    cr_assert_eq(lettermask, 0x03fff7fe);

    bomm_lettermask_clear(&lettermask, 25);
    cr_assert_eq(lettermask, 0x01fff7fe);
}
