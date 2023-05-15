//
//  utility.c
//  Bomm
//
//  Created by Fränz Friederes on 15/05/2023.
//

#include <criterion/criterion.h>
#include "../src/utility.h"

Test(wiring, bomm_hardware_concurrency) {
    unsigned int thread_count = bomm_hardware_concurrency();
    cr_assert_geq(thread_count, 1);
    cr_assert_lt(thread_count, INT_MAX);
}
