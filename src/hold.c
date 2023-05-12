//
//  hold.c
//  Bomm
//
//  Created by Fränz Friederes on 05/04/2023.
//

#include "hold.h"
#include <math.h>

bomm_hold_t* bomm_hold_init(
    bomm_hold_t* hold,
    size_t element_size,
    unsigned int hold_size
) {
    size_t element_mem_size = sizeof(bomm_hold_element_t) + element_size;
    size_t hold_mem_size = sizeof(bomm_hold_t) + element_mem_size * hold_size;

    if (hold_size == 0) {
        return NULL;
    }

    // Allocate hold
    if (hold == NULL) {
        if ((hold = malloc(hold_mem_size)) == NULL) {
            return NULL;
        }
    }

    // Initialize hold
    hold->element_size = element_size;
    hold->size = hold_size;
    hold->count = 0;
    pthread_mutex_init(&hold->mutex, NULL);

    return hold;
}

void bomm_hold_destroy(bomm_hold_t* hold) {
    pthread_mutex_destroy(&hold->mutex);
    free(hold);
}

float bomm_hold_add(bomm_hold_t* hold, float score, void* data, char* preview) {
    size_t element_mem_size = sizeof(bomm_hold_element_t) + hold->element_size;

    // Lock hold for atomic mutation
    pthread_mutex_lock(&hold->mutex);

    int index = hold->count - 1;
    char* element_ptr = &hold->elements[0] + element_mem_size * index;

    // Find the index that beats the given score (-1 if no such score exists)
    while (index >= 0 && ((bomm_hold_element_t*) element_ptr)->score < score) {
        index--;
        element_ptr -= element_mem_size;
    }

    // Check if the element beating the score is a duplicate
    bool duplicate =
        index != -1 &&
        (unsigned int) index < hold->count &&
        ((bomm_hold_element_t*) element_ptr)->score == score &&
        memcmp(
           ((bomm_hold_element_t*) element_ptr)->data,
           data,
           hold->element_size
        ) == 0;

    // Move below this element
    index++;
    element_ptr += element_mem_size;

    // Decide whether to accept the new element
    if ((unsigned int) index < hold->size && !duplicate) {
        // If hold is not full, yet, append one row
        if (hold->count < hold->size) {
            hold->count++;
        }

        // Move elements down one row to make space for the new element
        if ((unsigned int) index < hold->count - 1) {
            memmove(
                element_ptr + element_mem_size,
                element_ptr,
                (hold->count - 1 - index) * element_mem_size
            );
        }

        // Insert element
        bomm_hold_element_t* element = (bomm_hold_element_t*) element_ptr;
        element->score = score;
        memcpy(element->data, data, hold->element_size);
        if (preview != NULL) {
            memcpy(element->preview, preview, BOMM_HOLD_PREVIEW_SIZE);
        } else {
            element->preview[0] = '\0';
        }
    }

    // Return new score boundary necessary to enter the hold
    float new_score_boundary =
        hold->count == hold->size
            ? bomm_hold_at(hold, -1)->score
            : -INFINITY;

    // Unlock hold
    pthread_mutex_unlock(&hold->mutex);
    return new_score_boundary;
}
