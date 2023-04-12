//
//  wheel.c
//  Bomm
//
//  Created by Fränz Friederes on 12/04/2023.
//

#include <criterion/criterion.h>
#include <jansson.h>
#include "../src/wheel.h"

Test(wiring, bomm_wheel_extract_json) {
    const char* wheel_json_string = "{ \"name\": \"I\", \"wiring\": \"ekmflgdqvzntowyhxuspaibrcj\", \"turnovers\": \"q\" }";
    json_error_t error;
    json_t* wheel_json = json_loads(wheel_json_string, 0, &error);
    
    bomm_wheel_t wheel;
    bomm_wheel_t* result = bomm_wheel_extract_json(&wheel, wheel_json);
    json_decref(wheel_json);
    
    char actual_string[BOMM_ALPHABET_SIZE + 1];
    cr_assert_eq(result, &wheel);
    cr_assert_str_eq(wheel.name, "I");
    bomm_wiring_serialize(actual_string, sizeof(actual_string), &wheel.wiring);
    cr_assert_str_eq(actual_string, "ekmflgdqvzntowyhxuspaibrcj");
    bomm_lettermask_serialize(actual_string, sizeof(actual_string), &wheel.turnovers);
    cr_assert_str_eq(actual_string, "q");
}
