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
 * Print the report screen.
 */
void bomm_report_print(bomm_query_t* query, unsigned int count) {
    bomm_progress_t* attack_progress[query->attack_count];
    for (unsigned int i = 0; i < query->attack_count; i++) {
        attack_progress[i] = &query->attacks[i].progress;
    }

    bomm_progress_t joint_progress;
    joint_progress.batch_unit_size = 26;

    char space_string[80] =
        "                                        " \
        "                                        ";
    char line_string[240] =
        "────────────────────────────────────────" \
        "────────────────────────────────────────";

    char duration_string[16];
    char time_remaining_string[16];
    char key_string[80];
    char score_string[10];

    // Lock progress updates
    for (unsigned int i = 0; i < query->attack_count; i++) {
        pthread_mutex_lock(&query->attacks[i].progress_mutex);
    }

    // Calculate joint progress
    bomm_progress_parallel(&joint_progress, attack_progress, query->attack_count);

    // Unlock progress updates
    for (unsigned int i = 0; i < query->attack_count; i++) {
        pthread_mutex_unlock(&query->attacks[i].progress_mutex);
    }

    double percentage = bomm_progress_percentage(&joint_progress);
    bomm_duration_stringify(duration_string, 16, joint_progress.duration_sec);
    double time_remaining_sec = bomm_progress_time_remaining_sec(&joint_progress);
    bomm_duration_stringify(time_remaining_string, 16, time_remaining_sec);

    // Lock hold mutex while printing
    pthread_mutex_lock(&query->hold->mutex);

    // Print header
    printf("┌──────┬─%1$-.207s─┐\n", line_string);
    printf(
        "│ Bomm │ Progress \x1b[32m%10.3f %%\x1b[37m │ " \
        "Elapsed \x1b[32m%13.13s\x1b[37m │ " \
        "Remaining \x1b[32m%11.11s\x1b[37m │\n",
        percentage,
        duration_string,
        time_remaining_string
    );
    printf("├──────┴─%1$-.171s─┬─%1$-.27s─┤\n", line_string);

    // Print hold entries
    for (unsigned int i = 0; i < count; i++) {
        if (i < query->hold->count) {
            bomm_hold_element_t* element = bomm_hold_at(query->hold, i);
            bomm_key_stringify(key_string, sizeof(key_string), (bomm_key_t*) element->data);
            snprintf(score_string, sizeof(score_string), "%+10.10f", element->score);
            printf(
                "│ \x1b[32m%-64.64s\x1b[37m   %9.9s │\n",
                element->preview,
                score_string
            );
            printf("│ %-76.76s │\n", key_string);
        } else {
            printf("│ N/A %-.72s │\n", space_string);
            printf("│ %-.76s │\n", space_string);
        }
        if (i < count - 1) {
            printf("├─%1$-.192s─┬─%1$-.27s─┤\n", line_string);
        }
    }
    printf("└─%1$-.228s─┘\n", line_string);

    // Unlock hold mutex
    pthread_mutex_unlock(&query->hold->mutex);
}

/**
 * Handle signals SIGINT and SIGTERM.
 */
void bomm_signal_handler(int signal) {
    printf("\nSignal received. Terminating.\n");
    bomm_report_print(bomm_query_main, bomm_query_main->hold->size);
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
        bomm_report_print(query, 7);
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
    bomm_report_print(bomm_query_main, bomm_query_main->hold->size);

    // Kill report thread
    pthread_kill(report_thread, 0);

    // Clean up
    bomm_query_destroy(bomm_query_main);
    return 0;
}
