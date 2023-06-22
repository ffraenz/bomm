//
//  utility.c
//  Bomm
//
//  Created by Fränz Friederes on 15/05/2023.
//

#include <criterion/criterion.h>
#include "shared/helpers.h"
#include "../src/utility.h"

Test(utility, bomm_str_unique) {
    cr_assert_eq(bomm_str_unique(""), true);
    cr_assert_eq(bomm_str_unique("a"), true);
    cr_assert_eq(bomm_str_unique("ab"), true);
    cr_assert_eq(bomm_str_unique("abcdefghijklmnopqrstuvwxyz"), true);
    cr_assert_eq(bomm_str_unique("0123456789"), true);
    cr_assert_eq(bomm_str_unique("aa"), false);
    cr_assert_eq(bomm_str_unique("abcdefghijkimnopqrstuvwxyz"), false);
    cr_assert_eq(bomm_str_unique("01234567890"), false);
}

Test(utility, bomm_hardware_concurrency) {
    unsigned int num_threads = bomm_hardware_concurrency();
    cr_assert_geq(num_threads, 1);
    cr_assert_lt(num_threads, INT_MAX);
}
