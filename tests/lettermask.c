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

Test(lettermask, bomm_lettermask_from_string) {
    bomm_lettermask_t lettermask;

    bomm_lettermask_from_string(&lettermask, "");
    cr_assert_eq(lettermask, BOMM_LETTERMASK_NONE);

    bomm_lettermask_from_string(&lettermask, "e");
    cr_assert_eq(lettermask, 0x10);

    bomm_lettermask_from_string(&lettermask, "enr");
    cr_assert_eq(lettermask, 0x22010);

    bomm_lettermask_from_string(&lettermask, "enrxsi");
    cr_assert_eq(lettermask, 0x862110);

    bomm_lettermask_from_string(&lettermask, "swzfhmq");
    cr_assert_eq(lettermask, 0x024510a0);

    bomm_lettermask_from_string(&lettermask, "bcdefghijklmnopqrstuvwxyz");
    cr_assert_eq(lettermask, 0x3fffffe);

    bomm_lettermask_from_string(&lettermask, "abcdefghijklmnopqrstuvwxyz");
    cr_assert_eq(lettermask, BOMM_LETTERMASK_ALL);
}

Test(lettermask, bomm_lettermask_stringify) {
    bomm_lettermask_t lettermask;
    char actual_string[BOMM_ALPHABET_SIZE + 1];

    lettermask = BOMM_LETTERMASK_NONE;
    bomm_lettermask_stringify(actual_string, sizeof(actual_string), &lettermask);
    cr_assert_str_eq(actual_string, "");

    lettermask = 0x024510a0;
    bomm_lettermask_stringify(actual_string, sizeof(actual_string), &lettermask);
    cr_assert_str_eq(actual_string, "fhmqswz");

    lettermask = BOMM_LETTERMASK_ALL;
    bomm_lettermask_stringify(actual_string, sizeof(actual_string), &lettermask);
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

Test(lettermask, bomm_lettermask_count) {
    bomm_lettermask_t lettermask;

    lettermask = BOMM_LETTERMASK_NONE;
    cr_assert_eq(bomm_lettermask_count(&lettermask), 0);

    lettermask = 0x10;
    cr_assert_eq(bomm_lettermask_count(&lettermask), 1);

    lettermask = 0x22010;
    cr_assert_eq(bomm_lettermask_count(&lettermask), 3);

    lettermask = 0x862110;
    cr_assert_eq(bomm_lettermask_count(&lettermask), 6);

    lettermask = 0x3fffffe;
    cr_assert_eq(bomm_lettermask_count(&lettermask), 25);

    lettermask = BOMM_LETTERMASK_ALL;
    cr_assert_eq(bomm_lettermask_count(&lettermask), BOMM_ALPHABET_SIZE);
}
