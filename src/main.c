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
 * Global query instance
 */
bomm_query_t* bomm_query_main;

/**
 * Handle signals SIGINT and SIGTERM
 */
void bomm_signal_handler(int signal) {
    printf("\nSignal received. Terminating.\n");
    bomm_query_print(bomm_query_main, bomm_query_main->hold->size);
    exit(signal);
}

/**
 * View thread updating the view
 */
void* bomm_view_thread(void* omitted) {
    (void) omitted;
    bool initial_draw = true;
    while (true) {
        if (initial_draw) {
            initial_draw = false;
        } else {
            printf("\x1b[%dA", 24);
        }
        bomm_query_print(bomm_query_main, 6);
        sleep(3);
    }
    return NULL;
}

/**
 * Command line program entry point
 */
int main(int argc, char *argv[]) {
    bool error = false;
    bomm_attack_t* attack;

    // Initialize query from input
    bomm_query_main = bomm_query_init(argc, argv);
    if (bomm_query_main == NULL) {
        return 1;
    }

    // Install signal handler
    signal(SIGINT, bomm_signal_handler);
    signal(SIGTERM, bomm_signal_handler);

    // Create attack threads that work in parallel
    for (unsigned int i = 0; i < bomm_query_main->attack_count; i++) {
        attack = &bomm_query_main->attacks[i];
        if (pthread_create(&attack->thread, NULL, bomm_attack_thread, attack)) {
            error = true;
        }
    }

    // Start view thread
    pthread_t view_thread = 0;
    if (pthread_create(&view_thread, NULL, bomm_view_thread, NULL)) {
        error = true;
    }

    if (error) {
        fprintf(stderr, "Error while creating threads\n");
        for (unsigned int i = 0; i < bomm_query_main->attack_count; i++) {
            attack = &bomm_query_main->attacks[i];
            if (attack->thread != 0) {
                pthread_kill(attack->thread, SIGINT);
            }
        }
        if (view_thread != 0) {
            pthread_kill(view_thread, SIGINT);
        }
        bomm_query_destroy(bomm_query_main);
        return 1;
    }

    // Wait for attack threads to complete
    for (unsigned int i = 0; i < bomm_query_main->attack_count; i++) {
        attack = &bomm_query_main->attacks[i];
        pthread_join(attack->thread, NULL);
        attack->thread = 0;
    }

    // Print final view
    printf("\x1b[%dA", 24);
    bomm_query_print(bomm_query_main, bomm_query_main->hold->size);

    // Kill view thread
    pthread_kill(view_thread, SIGINT);
    view_thread = 0;

    // Clean up
    bomm_query_destroy(bomm_query_main);
    return 0;
}
