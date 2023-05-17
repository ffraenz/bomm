//
//  hold.c
//  Bomm
//
//  Created by Fränz Friederes on 05/04/2023.
//

#include <criterion/criterion.h>
#include <math.h>
#include "../src/hold.h"

Test(key, bomm_hold_init) {
    size_t element_size = sizeof(unsigned int);
    bomm_hold_t* hold = bomm_hold_init(NULL, element_size, 5);
    cr_assert_eq(hold->data_size, element_size);
    cr_assert_eq(hold->size, 5);
    cr_assert_eq(hold->count, 0);
    bomm_hold_destroy(hold);
}

Test(key, bomm_hold_add) {
    size_t element_size = sizeof(unsigned int);
    unsigned int data;
    float score_boundary;
    bomm_hold_element_t* element;

    bomm_hold_t* hold = bomm_hold_init(NULL, element_size, 5);

    data = 1337;
    score_boundary = bomm_hold_add(hold, 3, &data, "L333t Third");
    cr_assert_eq(score_boundary, -INFINITY);
    cr_assert_eq(hold->count, 1);

    data = 8888;
    score_boundary = bomm_hold_add(hold, -10, &data, "8888 Out");
    cr_assert_eq(score_boundary, -INFINITY);
    cr_assert_eq(hold->count, 2);

    // Duplicate entry to be ignored
    data = 8888;
    score_boundary = bomm_hold_add(hold, -10, &data, "8888 Out");
    cr_assert_eq(score_boundary, -INFINITY);
    cr_assert_eq(hold->count, 2);

    data = 1234;
    score_boundary = bomm_hold_add(hold, 2, &data, "1234 Fifth");
    cr_assert_eq(score_boundary, -INFINITY);
    cr_assert_eq(hold->count, 3);

    data = 7777;
    score_boundary = bomm_hold_add(hold, 7, &data, "<7 First");
    cr_assert_eq(score_boundary, -INFINITY);
    cr_assert_eq(hold->count, 4);

    data = 4444;
    score_boundary = bomm_hold_add(hold, 4, &data, "<4 Second");
    cr_assert_eq(score_boundary, -10);
    cr_assert_eq(hold->count, 5);

    data = 133337;
    score_boundary = bomm_hold_add(hold, 3, &data, "L3333t Fourth");
    cr_assert_eq(score_boundary, 2);
    cr_assert_eq(hold->count, 5);

    // Duplicate entry to be ignored
    data = 7777;
    score_boundary = bomm_hold_add(hold, 7, &data, "<7 First");
    cr_assert_eq(score_boundary, 2);
    cr_assert_eq(hold->count, 5);

    element = bomm_hold_at(hold, 0);
    cr_assert_eq(element->score, 7);
    cr_assert_eq(*((unsigned int*) element->data), 7777);
    cr_assert_str_eq(element->preview, "<7 First");

    element = bomm_hold_at(hold, 1);
    cr_assert_eq(element->score, 4);
    cr_assert_eq(*((unsigned int*) element->data), 4444);
    cr_assert_str_eq(element->preview, "<4 Second");

    element = bomm_hold_at(hold, 2);
    cr_assert_eq(element->score, 3);
    cr_assert_eq(*((unsigned int*) element->data), 1337);
    cr_assert_str_eq(element->preview, "L333t Third");

    element = bomm_hold_at(hold, 3);
    cr_assert_eq(element->score, 3);
    cr_assert_eq(*((unsigned int*) element->data), 133337);
    cr_assert_str_eq(element->preview, "L3333t Fourth");

    element = bomm_hold_at(hold, 4);
    cr_assert_eq(element->score, 2);
    cr_assert_eq(*((unsigned int*) element->data), 1234);
    cr_assert_str_eq(element->preview, "1234 Fifth");

    bomm_hold_destroy(hold);
}
