#include "crawler.h"

int main() {
    // Initialize visited set
    memset(visited.buckets, 0, sizeof(visited.buckets));
    pthread_mutex_init(&visited.mutex, NULL);

    // Initialize queue with seed URL
    queue_init(&queue);
    const char *seed_url = "https://www.scrapethissite.com/";
    add_visited(seed_url);
    enqueue(&queue, seed_url, 0);

    // Create threads
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, crawler_thread, NULL);
    }

    // For demo, run for 30 seconds
    sleep(30);
    crawling_done = true;
    pthread_cond_broadcast(&queue.cond_not_empty);

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        VisitedNode *node = visited.buckets[i];
        while (node) {
            VisitedNode *temp = node;
            node = node->next;
            free(temp);
        }
    }
    pthread_mutex_destroy(&visited.mutex);
    pthread_mutex_destroy(&queue.mutex);
    pthread_cond_destroy(&queue.cond_not_empty);
    pthread_cond_destroy(&queue.cond_not_full);

    return 0;
}