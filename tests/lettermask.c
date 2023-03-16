//
//  lettermask.c
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#include <criterion/criterion.h>
#include "../src/lettermask.h"

Test(lettermask, bomm_load_lettermask) {
    bomm_lettermask_t lettermask;
    
    bomm_load_lettermask(&lettermask, "");
    cr_assert_eq(lettermask, BOMM_LETTERMASK_NONE);
    
    bomm_load_lettermask(&lettermask, "swzfhmq");
    cr_assert_eq(lettermask, 0x024510a0);
    
    bomm_load_lettermask(&lettermask, "abcdefghijklmnopqrstuvwxyz");
    cr_assert_eq(lettermask, BOMM_LETTERMASK_ALL);
}

Test(lettermask, bomm_describe_lettermask) {
    bomm_lettermask_t lettermask;
    char* actual_describe_string;
    
    lettermask = BOMM_LETTERMASK_NONE;
    actual_describe_string = bomm_describe_lettermask(&lettermask);
    cr_assert_str_eq(actual_describe_string, "");
    free(actual_describe_string);
    
    lettermask = 0x024510a0;
    actual_describe_string = bomm_describe_lettermask(&lettermask);
    cr_assert_str_eq(actual_describe_string, "fhmqswz");
    free(actual_describe_string);
    
    lettermask = BOMM_LETTERMASK_ALL;
    actual_describe_string = bomm_describe_lettermask(&lettermask);
    cr_assert_str_eq(actual_describe_string, "abcdefghijklmnopqrstuvwxyz");
    free(actual_describe_string);
}
