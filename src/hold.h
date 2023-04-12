//
//  hold.h
//  Bomm
//
//  Created by Fränz Friederes on 05/04/2023.
//

#ifndef hold_h
#define hold_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#define BOMM_HOLD_PREVIEW_SIZE 64

/**
 * Variable-size struct representing a single element in a hold.
 */
typedef struct _bomm_hold_element {
    /**
     * Score positioning this element in a hold
     */
    float score;
    
    /**
     * Preview string visualizing the hold element
     */
    char preview[BOMM_HOLD_PREVIEW_SIZE];
    
    /**
     * Arbitrary element data (e.g. key)
     */
    char data[];
} bomm_hold_element_t;

/**
 * Variable-size struct holding a list of arbitrary elements ordered by the
 * fitness of an arbitrary measure.
 *
 * The name "hold" was coined by Heidi Williams in her 2000 paper "Applying
 * statistical language recognition techniques in the ciphertext-only
 * cryptanalysis of enigma".
 */
typedef struct _bomm_hold {
    /**
     * Size of an element's data in memory.
     */
    size_t element_size;
    
    /**
     * Maximum number of elements to be hold
     */
    unsigned int size;
    
    /**
     * Current number of elements
     */
    unsigned int count;
    
    /**
     * Mutex for access control across threads
     */
    pthread_mutex_t mutex;
    
    /**
     * Hold elements
     */
    char elements[];
} bomm_hold_t;

/**
 * Allocate and initialize a hold for the given element size and hold size
 * (i.e. the maximum number of elements it should hold).
 * @return Pointer to initialized hold or NULL, if allocation failed.
 */
bomm_hold_t* bomm_hold_init(size_t element_size, unsigned int hold_size);

/**
 * Destroy a hold and free its memory.
 */
void bomm_hold_destroy(bomm_hold_t* hold);

/**
 * Atomic operation to add the given element to the hold.
 * @param hold Pointer to the hold struct
 * @param data Pointer to the element data
 * @param preview Pointer to the preview string or a NULL-pointer to omit
 * @return New score boundary to enter the hold
 */
float bomm_hold_add(bomm_hold_t* hold, float score, void* data, char* preview);

/**
 * Return a pointer to the element at the given index.
 * Negative indices target elements from the end of the hold.
 */
static inline bomm_hold_element_t* bomm_hold_at(bomm_hold_t* hold, int index) {
    if (index < 0) {
        index = hold->size + index;
    }
    
    // TODO: Assert `index < hold->size`
    
    size_t element_mem_size = sizeof(bomm_hold_element_t) + hold->element_size;
    return (bomm_hold_element_t*) (&hold->elements[0] + element_mem_size * index);
}

#endif /* hold_h */
