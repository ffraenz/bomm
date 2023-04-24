//
//  main.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include "attack.h"
#include "message.h"
#include "measure.h"
#include "query.h"
#include "progress.h"

bomm_query_t* bomm_query_main;

/**
 * Handle signals SIGINT and SIGTERM.
 */
void bomm_signal_handler(int signal) {
    printf("\nSignal received. Terminating.\n");
    bomm_query_report_print(bomm_query_main, bomm_query_main->hold->size);
    exit(signal);
}

void* bomm_report_thread(void* arg) {
    bomm_query_t* query = (bomm_query_t*) arg;
    bool initial_draw = true;
    while (true) {
        if (initial_draw) {
            initial_draw = false;
        } else {
            printf("\x1b[%dA", 24);
        }
        bomm_query_report_print(query, 6);
        sleep(3);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, bomm_signal_handler);
    signal(SIGTERM, bomm_signal_handler);

    // Initialize query from input
    bomm_query_main = bomm_query_init(argc, argv);
    if (bomm_query_main == NULL) {
        return 1;
    }

    // Execute attacks in parallel
    for (unsigned int i = 0; i < bomm_query_main->attack_count; i++) {
        bomm_attack_t* attack = &bomm_query_main->attacks[i];
        int result = pthread_create(&attack->thread, NULL, bomm_attack_thread, attack);
        assert(!result);
    }

    // Start report thread
    pthread_t report_thread;
    int result = pthread_create(&report_thread, NULL, bomm_report_thread, bomm_query_main);
    assert(!result);

    // Wait for all threads to complete
    for (unsigned int i = 0; i < bomm_query_main->attack_count; i++) {
        bomm_attack_t* attack = &bomm_query_main->attacks[i];
        pthread_join(attack->thread, NULL);
        attack->thread = NULL;
    }

    // Print final report
    printf("\x1b[%dA", 24);
    bomm_query_report_print(bomm_query_main, bomm_query_main->hold->size);

    // Kill report thread
    pthread_kill(report_thread, 0);

    // Clean up
    bomm_query_destroy(bomm_query_main);
    return 0;
}
