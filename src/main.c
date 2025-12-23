#include <curl/curl.h>
#include <gumbo.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For usleep if needed

#include <ctype.h>   // For tolower
#include <stdbool.h> // For bool

// Define constants
#define MAX_URL_LEN 1024
#define MAX_QUEUE_SIZE 10000
#define NUM_THREADS 4 // Number of worker threads
#define MAX_DEPTH 3   // Limit crawl depth to prevent infinite crawling

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
#define HASH_TABLE_SIZE 10007 // Prime number for hashing
typedef struct VisitedNode {
    char url[MAX_URL_LEN];
    struct VisitedNode *next;
} VisitedNode;

typedef struct {
    VisitedNode *buckets[HASH_TABLE_SIZE];
    pthread_mutex_t mutex;
} VisitedSet;

// Global queue and visited set
Queue queue;
VisitedSet visited;
bool crawling_done = false; // Flag to signal threads to stop

// Domain to restrict crawling (e.g., "www.scrapethissite.com")
const char *target_domain = "www.scrapethissite.com";

// Callback for curl to write response data
typedef struct {
    char *data;
    size_t size;
} MemoryBuffer;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryBuffer *mem = (MemoryBuffer *)userp;
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL)
        return 0; // Out of memory
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

// Hash function for URLs
unsigned int hash_url(const char *url) {
    unsigned int hash = 0;
    while (*url) {
        hash = hash * 31 + tolower(*url++);
    }
    return hash % HASH_TABLE_SIZE;
}

// Check if URL is visited
bool is_visited(const char *url) {
    unsigned int index = hash_url(url);
    pthread_mutex_lock(&visited.mutex);
    VisitedNode *node = visited.buckets[index];
    while (node) {
        if (strcmp(node->url, url) == 0) {
            pthread_mutex_unlock(&visited.mutex);
            return true;
        }
        node = node->next;
    }
    pthread_mutex_unlock(&visited.mutex);
    return false;
}

// Add URL to visited
void add_visited(const char *url) {
    unsigned int index = hash_url(url);
    VisitedNode *new_node = malloc(sizeof(VisitedNode));
    strcpy(new_node->url, url);
    new_node->next = NULL;

    pthread_mutex_lock(&visited.mutex);
    new_node->next = visited.buckets[index];
    visited.buckets[index] = new_node;
    pthread_mutex_unlock(&visited.mutex);
}

// Initialize queue
void queue_init(Queue *q) {
    q->front = q->rear = NULL;
    q->size = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond_not_empty, NULL);
    pthread_cond_init(&q->cond_not_full, NULL);
}

// Enqueue URL with depth
void enqueue(Queue *q, const char *url, int depth) {
    pthread_mutex_lock(&q->mutex);
    while (q->size >= MAX_QUEUE_SIZE) {
        pthread_cond_wait(&q->cond_not_full, &q->mutex);
    }

    QueueNode *new_node = malloc(sizeof(QueueNode));
    strcpy(new_node->url, url);
    new_node->depth = depth;
    new_node->next = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = new_node;
    } else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->size++;
    pthread_cond_signal(&q->cond_not_empty);
    pthread_mutex_unlock(&q->mutex);
}

// Dequeue URL and depth
bool dequeue(Queue *q, char *url, int *depth) {
    pthread_mutex_lock(&q->mutex);
    while (q->size == 0 && !crawling_done) {
        pthread_cond_wait(&q->cond_not_empty, &q->mutex);
    }
    if (q->size == 0 && crawling_done) {
        pthread_mutex_unlock(&q->mutex);
        return false; // No more items, crawling done
    }

    QueueNode *temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;
    strcpy(url, temp->url);
    *depth = temp->depth;
    free(temp);
    q->size--;
    pthread_cond_signal(&q->cond_not_full);
    pthread_mutex_unlock(&q->mutex);
    return true;
}

// Get the root URL (scheme://authority)
void get_root_url(const char *url, char *root) {
    const char *start = strstr(url, "://");
    if (!start) {
        strcpy(root, url);
        return;
    }
    start += 3;
    const char *end = strchr(start, '/');
    if (!end)
        end = url + strlen(url);
    size_t len = end - url;
    strncpy(root, url, len);
    root[len] = '\0';
}

// Normalize URL (handle absolute, root-relative, and relative properly)
void normalize_url(const char *base, const char *relative, char *full_url) {
    if (strncmp(relative, "http://", 7) == 0 || strncmp(relative, "https://", 8) == 0) {
        strcpy(full_url, relative);
    } else if (relative[0] == '/') {
        // Root-relative: scheme://authority + relative
        char root[MAX_URL_LEN];
        get_root_url(base, root);
        snprintf(full_url, MAX_URL_LEN, "%s%s", root, relative);
    } else {
        // Relative: append to base's directory
        strcpy(full_url, base);
        size_t len = strlen(full_url);
        if (len > 0 && full_url[len - 1] != '/') {
            strcat(full_url, "/");
        }
        strcat(full_url, relative);
    }

    // Remove fragment if any
    char *hash = strchr(full_url, '#');
    if (hash)
        *hash = '\0';
}

// Check if URL belongs to target domain
bool is_same_domain(const char *url) {
    const char *domain_start = strstr(url, "://");
    if (!domain_start)
        return false;
    domain_start += 3;
    size_t domain_len = strlen(target_domain);
    return strncmp(domain_start, target_domain, domain_len) == 0 &&
           (domain_start[domain_len] == '/' || domain_start[domain_len] == '\0');
}

// Extract links from HTML using Gumbo
void extract_links(GumboNode *node, const char *base_url, int current_depth) {
    if (node->type != GUMBO_NODE_ELEMENT)
        return;

    if (node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute *href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href) {
            char full_url[MAX_URL_LEN];
            normalize_url(base_url, href->value, full_url);
            if (is_same_domain(full_url) && !is_visited(full_url)) {
                add_visited(full_url);
                if (current_depth + 1 <= MAX_DEPTH) {
                    enqueue(&queue, full_url, current_depth + 1);
                }
            }
        }
    }

    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        extract_links((GumboNode *)children->data[i], base_url, current_depth);
    }
}

// Worker thread function
void *crawler_thread(void *arg) {
    CURL *curl = curl_easy_init();
    if (!curl)
        return NULL;

    char url[MAX_URL_LEN];
    int depth;

    while (dequeue(&queue, url, &depth)) {
        if (depth > MAX_DEPTH)
            continue; // Skip if exceeds depth limit

        printf("Crawling: %s (depth %d)\n", url, depth);

        MemoryBuffer chunk = {.data = NULL, .size = 0};

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; Crawler/1.0)");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Timeout to prevent hangs

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            // Parse HTML with Gumbo
            GumboOutput *output = gumbo_parse(chunk.data);
            extract_links(output->root, url, depth);
            gumbo_destroy_output(&kGumboDefaultOptions, output);

            // TODO: Process the page content here (e.g., save to file, index, etc.)
        } else {
            fprintf(stderr, "Failed to fetch %s: %s\n", url, curl_easy_strerror(res));
        }

        free(chunk.data);
        usleep(100000); // Polite delay: 0.1s between requests
    }

    curl_easy_cleanup(curl);
    return NULL;
}

// Main function
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

    // Wait for queue to fill or some condition; here, simulate by sleeping or user input
    // For demo, run for a while then stop
    sleep(30); // Run for 30 seconds; adjust or use a better stop condition
    crawling_done = true;
    pthread_cond_broadcast(&queue.cond_not_empty); // Wake threads

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup (free visited nodes, etc.)
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