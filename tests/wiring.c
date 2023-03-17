//
//  wiring.c
//  Bomm
//
//  Created by Fränz Friederes on 15/03/2023.
//

#include <criterion/criterion.h>
#include "../src/wiring.h"

Test(wiring, bomm_load_wiring) {
    char* expected_wiring_string = "ekmflgdqvzntowyhxuspaibrcj";
    bomm_wiring_t* wiring = malloc(sizeof(bomm_wiring_t));
    bomm_wiring_extract(wiring, expected_wiring_string);
    char* actual_wiring_string = bomm_wiring_serialize(wiring);
    cr_assert_str_eq(actual_wiring_string, expected_wiring_string);
    free(actual_wiring_string);
    free(wiring);
}
