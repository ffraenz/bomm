//
//  wheel.c
//  Bomm
//
//  Created by Fränz Friederes on 12/04/2023.
//

#include <criterion/criterion.h>
#include <jansson.h>
#include "shared/helpers.h"
#include "../src/wheel.h"

Test(wiring, bomm_wheel_init_with_name) {
    bomm_test_skip_if_non_latin_alphabet;

    bomm_wheel_t wheel;
    char string[BOMM_ALPHABET_SIZE + 1];

    cr_assert_eq(bomm_wheel_init_with_name(&wheel, "ETW-ABC"), &wheel);
    cr_assert_str_eq(wheel.name, "ETW-ABC");
    bomm_wiring_stringify(string, sizeof(string), &wheel.wiring);
    cr_assert_str_eq(string, "abcdefghijklmnopqrstuvwxyz");
    bomm_lettermask_stringify(string, sizeof(string), &wheel.turnovers);
    cr_assert_str_eq(string, "");

    cr_assert_eq(bomm_wheel_init_with_name(&wheel, "I"), &wheel);
    cr_assert_str_eq(wheel.name, "I");
    bomm_wiring_stringify(string, sizeof(string), &wheel.wiring);
    cr_assert_str_eq(string, "ekmflgdqvzntowyhxuspaibrcj");
    bomm_lettermask_stringify(string, sizeof(string), &wheel.turnovers);
    cr_assert_str_eq(string, "q");

    cr_assert_eq(bomm_wheel_init_with_name(&wheel, "VIII"), &wheel);
    cr_assert_str_eq(wheel.name, "VIII");
    bomm_wiring_stringify(string, sizeof(string), &wheel.wiring);
    cr_assert_str_eq(string, "fkqhtlxocbjspdzramewniuygv");
    bomm_lettermask_stringify(string, sizeof(string), &wheel.turnovers);
    cr_assert_str_eq(string, "mz");

    cr_assert_eq(bomm_wheel_init_with_name(&wheel, "UKW-B"), &wheel);
    cr_assert_str_eq(wheel.name, "UKW-B");
    bomm_wiring_stringify(string, sizeof(string), &wheel.wiring);
    cr_assert_str_eq(string, "yruhqsldpxngokmiebfzcwvjat");
    bomm_lettermask_stringify(string, sizeof(string), &wheel.turnovers);
    cr_assert_str_eq(string, "");

    cr_assert_eq(bomm_wheel_init_with_name(&wheel, "unknown"), NULL);
}

Test(wiring, bomm_wheel_init_with_json) {
    bomm_test_skip_if_non_latin_alphabet;
    const char* wheel_json_string =
        "{ \"name\": \"I\", \"wiring\": \"ekmflgdqvzntowyhxuspaibrcj\", " \
        "\"turnovers\": \"q\" }";

    json_error_t error;
    json_t* wheel_json = json_loads(wheel_json_string, 0, &error);

    bomm_wheel_t wheel;
    bomm_wheel_t* result = bomm_wheel_init_with_json(&wheel, wheel_json);
    json_decref(wheel_json);

    char actual_string[BOMM_ALPHABET_SIZE + 1];
    cr_assert_eq(result, &wheel);
    cr_assert_str_eq(wheel.name, "I");
    bomm_wiring_stringify(actual_string, sizeof(actual_string), &wheel.wiring);
    cr_assert_str_eq(actual_string, "ekmflgdqvzntowyhxuspaibrcj");
    bomm_lettermask_stringify(actual_string, sizeof(actual_string), &wheel.turnovers);
    cr_assert_str_eq(actual_string, "q");
}

Test(wiring, bomm_wheel_set_init_with_json) {
    bomm_test_skip_if_non_latin_alphabet;
    unsigned int num_wheels = 3;
    bomm_wheel_t wheels[num_wheels];
    bomm_wheel_init(&wheels[0], "I",   "ekmflgdqvzntowyhxuspaibrcj", "q");
    bomm_wheel_init(&wheels[1], "II",  "ajdksiruxblhwtmcqgznpyfvoe", "e");
    bomm_wheel_init(&wheels[2], "III", "bdfhjlcprtxvznyeiwgakmusqo", "v");

    const char* wheel_set_json_string = "[\"II\", \"I\", \"III\"]";

    json_error_t error;
    json_t* wheel_set_json = json_loads(wheel_set_json_string, 0, &error);

    unsigned int wheel_set_size = 3;
    bomm_wheel_t wheel_set[wheel_set_size];
    bool success = bomm_wheel_set_init_with_json(
        wheel_set,
        wheel_set_size,
        wheel_set_json,
        wheels,
        num_wheels
    );

    cr_assert_eq(success, true);
    cr_assert_arr_eq(&wheel_set[0], &wheels[1], sizeof(bomm_wheel_t));
    cr_assert_arr_eq(&wheel_set[1], &wheels[0], sizeof(bomm_wheel_t));
    cr_assert_arr_eq(&wheel_set[2], &wheels[2], sizeof(bomm_wheel_t));
}
