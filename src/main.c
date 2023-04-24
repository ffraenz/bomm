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
#include "attack.h"
#include "message.h"
#include "measure.h"
#include "query.h"
#include "progress.h"

void* bomm_report_thread_run(void* arg) {
    bomm_query_t* query = (bomm_query_t*) arg;

    bomm_progress_t* attack_progress[query->attack_count];
    for (unsigned int i = 0; i < query->attack_count; i++) {
        attack_progress[i] = &query->attacks[i].progress;
    }

    bomm_progress_t joint_progress;
    joint_progress.batch_unit_size = 26;

    sleep(1);

    char progress_string[128];
    while (true) {
        // Lock progress updates
        for (unsigned int i = 0; i < query->attack_count; i++) {
            pthread_mutex_lock(&query->attacks[i].progress_mutex);
        }

        // Gather data
        bomm_progress_parallel(&joint_progress, attack_progress, query->attack_count);

        // Unlock progress updates
        for (unsigned int i = 0; i < query->attack_count; i++) {
            pthread_mutex_unlock(&query->attacks[i].progress_mutex);
        }

        // Joint progress
        bomm_progress_stringify(progress_string, 128, &joint_progress);
        printf("\n%s\n", progress_string);
        bomm_key_hold_print(query->hold);

        sleep(6);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    // Initialize query from input
    bomm_query_t* query = bomm_query_init(argc, argv);
    if (query == NULL) {
        return 1;
    }

    // Execute attacks in parallel
    for (unsigned int i = 0; i < query->attack_count; i++) {
        bomm_attack_t* attack = &query->attacks[i];
        int result = pthread_create(&attack->thread, NULL, bomm_attack_execute, attack);
        assert(!result);
        printf("Attack %d is started.\n", attack->id);
    }

    // Start report thread
    pthread_t report_thread;
    int result = pthread_create(&report_thread, NULL, bomm_report_thread_run, query);
    assert(!result);
    printf("Report thread is started.\n");

    // Wait for all threads to complete
    for (unsigned int i = 0; i < query->attack_count; i++) {
        bomm_attack_t* attack = &query->attacks[i];
        pthread_join(attack->thread, NULL);
        attack->thread = NULL;
        printf("Attack %d is completed.\n", attack->id);
    }

    // Kill report thread
    pthread_kill(report_thread, 0);

    // Print final results
    printf("Final hold:\n");
    bomm_key_hold_print(query->hold);

    // Clean up
    bomm_query_destroy(query);
    return 0;
}
