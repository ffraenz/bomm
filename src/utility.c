//
//  utility.c
//  Bomm
//
//  Created by Fränz Friederes on 23/03/2023.
//

#include "utility.h"

void bomm_array_shuffle(unsigned int* array, size_t size) {
    for (size_t i = size - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}
