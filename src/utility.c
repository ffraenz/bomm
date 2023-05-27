//
//  utility.c
//  Bomm
//
//  Created by Fränz Friederes on 23/03/2023.
//

#include "utility.h"
#include <unistd.h>

void bomm_array_shuffle(unsigned int* array, size_t size) {
    for (size_t i = size - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

unsigned int bomm_hardware_concurrency(void) {
    long num_cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpu_cores < 1) {
        return 1;
    }
    return (unsigned int) num_cpu_cores;
}
