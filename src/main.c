//
//  main.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "attack.h"
#include "query.h"

/**
 * Main query instance
 */
bomm_query_t* bomm_query_main;

/**
 * Handle SIGINT and SIGTERM signals to safely cancel query.
 */
void bomm_signal_handler(int signal) {
    (void) signal;
    printf("\nSignal received. Cancelling query.\n");
    bomm_query_cancel(bomm_query_main);
}

/**
 * Command line program entry point
 */
int main(int argc, char *argv[]) {
    // Seed PRNG
    srand((unsigned int) time(NULL));

    // Initialize query from command line arguments
    bomm_query_main = bomm_query_init(argc, argv);
    if (bomm_query_main == NULL) {
        return 1;
    }

    // Print out details
    printf("Hold size: %d\n", bomm_query_main->hold->size);
    printf("Concurrent attacks: %d\n", bomm_query_main->num_attacks);

    // Install signal handler
    signal(SIGINT, bomm_signal_handler);
    signal(SIGTERM, bomm_signal_handler);

    // Start query threads
    if (bomm_query_start(bomm_query_main)) {
        fprintf(stderr, "Error while creating query threads\n");
        bomm_query_destroy(bomm_query_main);
        return 1;
    }

    // Start view loop
    if (!bomm_query_main->quiet) {
        bool initial_view = true;
        do {
            if (!initial_view) {
                // Move up the print cursor 24 lines to redraw
                printf("\x1b[%dA", 24);
            }
            bomm_query_print(bomm_query_main, 6);

            if (initial_view) {
                initial_view = false;
            } else {
                sleep(5);
            }
        } while (bomm_query_is_pending(bomm_query_main));
    }

    // Wait for the threads to finish
    printf("Waiting for query threads to terminate.\n");
    bomm_query_join(bomm_query_main);

    // Print final view
    printf("Final query results:\n");
    bomm_query_print(bomm_query_main, bomm_query_main->hold->size);
    printf("Concurrent attacks: %d\n", bomm_query_main->num_attacks);
    printf("Number of units: %lu\n", bomm_query_main->joint_progress.num_units);
    printf("Number of decrypts: %lu\n", bomm_query_main->joint_progress.num_decrypts);

    // Clean up
    bomm_query_destroy(bomm_query_main);
    bomm_query_main = NULL;
    bomm_measure_config_destroy();

    return 0;
}
