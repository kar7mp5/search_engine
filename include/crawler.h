#ifndef CRAWLER_H
#define CRAWLER_H

#include <curl/curl.h>
#include <gumbo.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

// Define constants
#define MAX_URL_LEN 1024
#define MAX_QUEUE_SIZE 10000
#define NUM_THREADS 4  // Number of worker threads
#define MAX_DEPTH 3    // Limit crawl depth to prevent infinite crawling
#define HASH_TABLE_SIZE 10007  // Prime number for hashing

// Structure for queue nodes (URLs with depth)
typedef struct QueueNode {
    char url[MAX_URL_LEN];
    int depth;
    struct QueueNode *next;
} QueueNode;

// Structure for the BFS queue
typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t cond_not_empty;
    pthread_cond_t cond_not_full;
} Queue;

// Structure for visited set (simple hash table for URLs)
typedef struct VisitedNode {
    char url[MAX_URL_LEN];
    struct VisitedNode *next;
} VisitedNode;

typedef struct {
    VisitedNode *buckets[HASH_TABLE_SIZE];
    pthread_mutex_t mutex;
} VisitedSet;

// Callback for curl to write response data
typedef struct {
    char *data;
    size_t size;
} MemoryBuffer;

// Function prototypes
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
unsigned int hash_url(const char *url);
bool is_visited(const char *url);
void add_visited(const char *url);
void queue_init(Queue *q);
void enqueue(Queue *q, const char *url, int depth);
bool dequeue(Queue *q, char *url, int *depth);
void get_root_url(const char *url, char *root);
void normalize_url(const char *base, const char *relative, char *full_url);
bool is_same_domain(const char *url);
void extract_links(GumboNode *node, const char *base_url, int current_depth);
void *crawler_thread(void *arg);

// Global variables (extern if needed in other files)
extern Queue queue;
extern VisitedSet visited;
extern bool crawling_done;
extern const char *target_domain;

#endif // CRAWLER_H