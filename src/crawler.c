#include "crawler.h"

// Global variables definition
Queue queue;
VisitedSet visited;
bool crawling_done = false;
const char *target_domain = "www.scrapethissite.com";

// Write callback implementation
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryBuffer *mem = (MemoryBuffer *)userp;
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL) return 0;
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

// Hash function implementation
unsigned int hash_url(const char *url) {
    unsigned int hash = 0;
    while (*url) {
        hash = hash * 31 + tolower(*url++);
    }
    return hash % HASH_TABLE_SIZE;
}

// is_visited implementation
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

// add_visited implementation
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

// queue_init implementation
void queue_init(Queue *q) {
    q->front = q->rear = NULL;
    q->size = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond_not_empty, NULL);
    pthread_cond_init(&q->cond_not_full, NULL);
}

// enqueue implementation
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

// dequeue implementation
bool dequeue(Queue *q, char *url, int *depth) {
    pthread_mutex_lock(&q->mutex);
    while (q->size == 0 && !crawling_done) {
        pthread_cond_wait(&q->cond_not_empty, &q->mutex);
    }
    if (q->size == 0 && crawling_done) {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    QueueNode *temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    strcpy(url, temp->url);
    *depth = temp->depth;
    free(temp);
    q->size--;
    pthread_cond_signal(&q->cond_not_full);
    pthread_mutex_unlock(&q->mutex);
    return true;
}

// get_root_url implementation
void get_root_url(const char *url, char *root) {
    const char *start = strstr(url, "://");
    if (!start) {
        strcpy(root, url);
        return;
    }
    start += 3;
    const char *end = strchr(start, '/');
    if (!end) end = url + strlen(url);
    size_t len = end - url;
    strncpy(root, url, len);
    root[len] = '\0';
}

// normalize_url implementation
void normalize_url(const char *base, const char *relative, char *full_url) {
    if (strncmp(relative, "http://", 7) == 0 || strncmp(relative, "https://", 8) == 0) {
        strcpy(full_url, relative);
    } else if (relative[0] == '/') {
        char root[MAX_URL_LEN];
        get_root_url(base, root);
        snprintf(full_url, MAX_URL_LEN, "%s%s", root, relative);
    } else {
        strcpy(full_url, base);
        size_t len = strlen(full_url);
        if (len > 0 && full_url[len - 1] != '/') {
            strcat(full_url, "/");
        }
        strcat(full_url, relative);
    }

    char *hash = strchr(full_url, '#');
    if (hash) *hash = '\0';
}

// is_same_domain implementation
bool is_same_domain(const char *url) {
    const char *domain_start = strstr(url, "://");
    if (!domain_start) return false;
    domain_start += 3;
    size_t domain_len = strlen(target_domain);
    return strncmp(domain_start, target_domain, domain_len) == 0 &&
           (domain_start[domain_len] == '/' || domain_start[domain_len] == '\0');
}

// extract_links implementation
void extract_links(GumboNode *node, const char *base_url, int current_depth) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

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

// crawler_thread implementation
void *crawler_thread(void *arg) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    char url[MAX_URL_LEN];
    int depth;

    while (dequeue(&queue, url, &depth)) {
        if (depth > MAX_DEPTH) continue;

        printf("Crawling: %s (depth %d)\n", url, depth);

        MemoryBuffer chunk = {.data = NULL, .size = 0};

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; Crawler/1.0)");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            GumboOutput *output = gumbo_parse(chunk.data);
            extract_links(output->root, url, depth);
            gumbo_destroy_output(&kGumboDefaultOptions, output);
            // TODO: Process the page content here
        } else {
            fprintf(stderr, "Failed to fetch %s: %s\n", url, curl_easy_strerror(res));
        }

        free(chunk.data);
        usleep(100000);
    }

    curl_easy_cleanup(curl);
    return NULL;
}