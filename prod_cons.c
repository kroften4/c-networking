#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

#define QUEUE_MAX_SIZE 5
#define PRODUCERS_AMOUNT 10
#define CONSUMERS_AMOUNT 10

int queue[QUEUE_MAX_SIZE];
size_t queue_size = 0;
pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t queue_not_empty_cond = PTHREAD_COND_INITIALIZER;

pthread_cond_t queue_not_full_cond = PTHREAD_COND_INITIALIZER;

void queue_print(int *queue, size_t size);
void consume();
void produce();

int main() {
    pthread_t producers[PRODUCERS_AMOUNT];
    pthread_t consumers[CONSUMERS_AMOUNT];

    for (size_t i = 0; i < PRODUCERS_AMOUNT; i++) {
        pthread_create(&producers[i], NULL, (void *)&produce, NULL);
    }
    for (size_t i = 0; i < CONSUMERS_AMOUNT; i++) {
        pthread_create(&consumers[i], NULL, (void *)&consume, NULL);
    }
    puts("created all threads");

    for (size_t i = 0; i < PRODUCERS_AMOUNT; i++) {
        pthread_join(producers[i], NULL);
    }
    for (size_t i = 0; i < CONSUMERS_AMOUNT; i++) {
        pthread_join(consumers[i], NULL);
    }
    puts("All done");

    return 0;
}

void queue_print(int *queue, size_t size) {
    pthread_mutex_lock(&queue_mtx);
    printf("[ ");
    for (int i = 0; i < size; i++) {
        printf("%d ", queue[i]);
    }
    printf("]\n");
    pthread_mutex_unlock(&queue_mtx);
}

void consume() {
    while (1) {
        pthread_mutex_lock(&queue_mtx);
        while (queue_size == 0) {
            pthread_cond_wait(&queue_not_empty_cond, &queue_mtx);
        }

        queue_size -= 1;
        printf("removing %d\n", queue[queue_size]);
        queue[queue_size] = -1;

        if (queue_size != QUEUE_MAX_SIZE) {
            pthread_cond_signal(&queue_not_full_cond);
        }
        pthread_mutex_unlock(&queue_mtx);

        queue_print(queue, QUEUE_MAX_SIZE);
    }
}

void produce() {
    while (1) {
        pthread_mutex_lock(&queue_mtx);
        while (queue_size == QUEUE_MAX_SIZE) {
            pthread_cond_wait(&queue_not_full_cond, &queue_mtx);
        }

        printf("placing %zu on %d\n", queue_size, queue[queue_size]);
        queue[queue_size] = queue_size;
        queue_size += 1;

        if (queue_size != 0) {
            pthread_cond_signal(&queue_not_empty_cond);
        }
        pthread_mutex_unlock(&queue_mtx);

        queue_print(queue, QUEUE_MAX_SIZE);
    }
}

