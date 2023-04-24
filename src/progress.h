//
//  progress.h
//  Bomm
//
//  Created by Fränz Friederes on 24/04/2023.
//

#ifndef progress_h
#define progress_h

#include <stdio.h>

/**
 * Struct representing the progress of an arbitrary workload.
 */
typedef struct _bomm_progress {
    /**
     * Number of workload units in a batch.
     */
    unsigned int batch_unit_size;

    /**
     * Estimated total number of workload units to be completed.
     */
    unsigned long unit_count;

    /**
     * Number of workload units completed so far.
     */
    unsigned long completed_unit_count;

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
    progress->unit_count = 0;
    progress->completed_unit_count = 0;
    progress->duration_sec = 0;
    progress->batch_duration_sec = 0;

    for (unsigned int i = 0; i < size; i++) {
        bomm_progress_t* child = children[i];

        progress->unit_count += child->unit_count;
        progress->completed_unit_count += child->completed_unit_count;

        if (child->duration_sec > progress->duration_sec) {
            progress->duration_sec = child->duration_sec;
        }

        double normalized_batch_duration =
            (((double) progress->batch_unit_size / size) / child->batch_unit_size) *
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
    if (progress->unit_count == 0) {
        return 0;
    }
    return (double) progress->completed_unit_count / progress->unit_count;
}

/**
 * Calculate the estimated time remaining in seconds for the given progress.
 */
static inline double bomm_progress_time_remaining_sec(bomm_progress_t* progress) {
    unsigned long remaining_unit_count =
        progress->unit_count - progress->completed_unit_count;
    double batches_remaining =
        (double) remaining_unit_count / progress->batch_unit_size;
    return progress->batch_duration_sec * batches_remaining;
}

/**
 * Export the given progress to a string.
 */
static inline void bomm_progress_stringify(
    char* str,
    size_t size,
    bomm_progress_t* progress
) {
    double percentage = bomm_progress_percentage(progress);
    double time_remaining_sec = bomm_progress_time_remaining_sec(progress);
    snprintf(
        str,
        size,
        "Progress: %.2f%%, Time elapsed: %.1f min, Time remaining: %.0f min",
        percentage * 100.0,
        progress->duration_sec / 60.0,
        time_remaining_sec / 60.0
    );
}

#endif /* progress_h */
