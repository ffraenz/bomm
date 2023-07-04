//
//  progress.h
//  Bomm
//
//  Created by Fränz Friederes on 24/04/2023.
//

#ifndef progress_h
#define progress_h

#include <stdio.h>
#include "utility.h"

/**
 * Struct representing the progress of an arbitrary workload.
 */
typedef struct _bomm_progress {
    /**
     * Number of workload units in a batch.
     */
    unsigned int num_batch_units;

    /**
     * Estimated total number of workload units to be completed.
     */
    unsigned long num_units;

    /**
     * Number of workload units completed so far.
     */
    unsigned long num_units_completed;

    /**
     * Number of decrypts evaluated
     */
    unsigned long num_decrypts;

    /**
     * Number of seconds elapsed so far.
     */
    double duration_sec;

    /**
     * Duration of the last batch in seconds.
     */
    double batch_duration_sec;
} bomm_progress_t;

/**
 * Turn the progress of multiple child workloads worked on in parallel into a
 * joint progress.
 */
static inline void bomm_progress_parallel(
    bomm_progress_t* progress,
    bomm_progress_t* children[],
    unsigned int size
) {
    progress->num_units = 0;
    progress->num_units_completed = 0;
    progress->num_decrypts = 0;
    progress->duration_sec = 0;
    progress->batch_duration_sec = 0;

    for (unsigned int i = 0; i < size; i++) {
        bomm_progress_t* child = children[i];

        progress->num_units += child->num_units;
        progress->num_units_completed += child->num_units_completed;
        progress->num_decrypts += child->num_decrypts;

        if (child->duration_sec > progress->duration_sec) {
            progress->duration_sec = child->duration_sec;
        }

        double normalized_batch_duration =
            (((double) progress->num_batch_units / size) / child->num_batch_units) *
            child->batch_duration_sec;

        if (normalized_batch_duration > progress->batch_duration_sec) {
            progress->batch_duration_sec = normalized_batch_duration;
        }
    }
}

/**
 * Calculate the percentage done for the given progress.
 */
static inline double bomm_progress_percentage(bomm_progress_t* progress) {
    if (progress->num_units == 0) {
        return 0;
    }
    return (double) progress->num_units_completed / progress->num_units;
}

/**
 * Calculate the estimated time remaining in seconds for the given progress.
 */
static inline double bomm_progress_time_remaining_sec(bomm_progress_t* progress) {
    unsigned long num_units_remaining =
        progress->num_units - progress->num_units_completed;
    double batches_remaining =
        (double) num_units_remaining / progress->num_batch_units;
    return progress->batch_duration_sec * batches_remaining;
}

#endif /* progress_h */
