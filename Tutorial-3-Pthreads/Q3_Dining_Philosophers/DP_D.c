#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 5

pthread_mutex_t chopstick[N];

void* philosopher(void* arg)
{
    int id = *(int*)arg;

    printf("Philosopher %d is thinking\n", id);
    sleep(1);

    printf("Philosopher %d is hungry\n", id);

    pthread_mutex_lock(&chopstick[id]);
    printf("Philosopher %d picked left chopstick\n", id);

    pthread_mutex_lock(&chopstick[(id + 1) % N]);
    printf("Philosopher %d picked right chopstick\n", id);

    printf("Philosopher %d is eating\n", id);
    sleep(1);

    pthread_mutex_unlock(&chopstick[(id + 1) % N]);
    pthread_mutex_unlock(&chopstick[id]);

    printf("Philosopher %d finished eating\n", id);

    return NULL;
}

int main()
{
    pthread_t threads[N];
    int id[N];

    for (int i = 0; i < N; i++)
        pthread_mutex_init(&chopstick[i], NULL);

    for (int i = 0; i < N; i++) {
        id[i] = i;
        pthread_create(&threads[i], NULL, philosopher, &id[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&chopstick[i]);

    return 0;
}