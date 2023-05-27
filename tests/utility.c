//
//  utility.c
//  Bomm
//
//  Created by Fränz Friederes on 15/05/2023.
//

#include <criterion/criterion.h>
#include "../src/utility.h"

Test(wiring, bomm_hardware_concurrency) {
    unsigned int num_threads = bomm_hardware_concurrency();
    cr_assert_geq(num_threads, 1);
    cr_assert_lt(num_threads, INT_MAX);
}
