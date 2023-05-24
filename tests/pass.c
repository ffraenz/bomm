//
//  pass.c
//  Bomm
//
//  Created by Fränz Friederes on 24/05/2023.
//

#include <criterion/criterion.h>
#include <jansson.h>
#include "../src/pass.h"

Test(wiring, bomm_pass_init_json_hill_climb_1) {
    const char* pass_json_string =
        "{ " \
        "\"type\": \"hillClimb\", " \
        "\"measure\": \"ic\", " \
        "\"finalMeasure\": \"trigram\", " \
        "\"finalMeasureMinNumPlugs\": 4, " \
        "\"backtrackingMinNumPlugs\": 6 " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);

    bomm_pass_t pass;
    cr_assert_eq(bomm_pass_init_json(&pass, pass_json), &pass);

    cr_assert_eq(pass.type, BOMM_PASS_HILL_CLIMB);
    cr_assert_eq(pass.config.hill_climb.measure, BOMM_MEASURE_IC);
    cr_assert_eq(pass.config.hill_climb.final_measure, BOMM_MEASURE_SINKOV_TRIGRAM);
    cr_assert_eq(pass.config.hill_climb.final_measure_min_num_plugs, 4);
    cr_assert_eq(pass.config.hill_climb.backtracking_min_num_plugs, 6);
}

Test(wiring, bomm_pass_init_json_hill_climb_2) {
    const char* pass_json_string =
        "{ " \
        "\"type\": \"hillClimb\", " \
        "\"measure\": \"trigram\", " \
        "\"backtrackingMinNumPlugs\": 3 " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);

    bomm_pass_t pass;
    cr_assert_eq(bomm_pass_init_json(&pass, pass_json), &pass);

    cr_assert_eq(pass.type, BOMM_PASS_HILL_CLIMB);
    cr_assert_eq(pass.config.hill_climb.measure, BOMM_MEASURE_SINKOV_TRIGRAM);
    cr_assert_eq(pass.config.hill_climb.final_measure, BOMM_MEASURE_SINKOV_TRIGRAM);
    cr_assert_eq(pass.config.hill_climb.backtracking_min_num_plugs, 3);
}

Test(wiring, bomm_pass_init_json_reswapping) {
    const char* pass_json_string =
        "{ " \
        "\"type\": \"reswapping\", " \
        "\"measure\": \"entropy\" " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);

    bomm_pass_t pass;
    cr_assert_eq(bomm_pass_init_json(&pass, pass_json), &pass);

    cr_assert_eq(pass.type, BOMM_PASS_RESWAPPING);
    cr_assert_eq(pass.config.reswapping.measure, BOMM_MEASURE_ENTROPY);
}

Test(wiring, bomm_pass_init_json_unknown) {
    const char* pass_json_string =
        "{ " \
        "\"type\": \"unknown\", " \
        "\"foo\": \"bar\" " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);
    cr_assert_eq(bomm_pass_init_json(NULL, pass_json), NULL);
}
