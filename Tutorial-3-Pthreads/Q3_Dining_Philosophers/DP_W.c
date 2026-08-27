#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define N 5

pthread_mutex_t chopsticks[N];
pthread_mutex_t waiter;

void* philosopher(void* arg)
{
    int id = *(int*)arg;

    printf("Philosopher %d is thinking\n", id);
    sleep(1);

    printf("Philosopher %d is hungry\n", id);

    pthread_mutex_lock(&waiter);

    printf("Philosopher %d got permission from waiter\n", id);

    pthread_mutex_lock(&chopsticks[id]);
    printf("Philosopher %d picked left chopstick\n", id);

    pthread_mutex_lock(&chopsticks[(id + 1) % N]);
    printf("Philosopher %d picked right chopstick\n", id);

    pthread_mutex_unlock(&waiter);

    printf("Philosopher %d is eating\n", id);
    sleep(1);

    pthread_mutex_unlock(&chopsticks[(id + 1) % N]);
    pthread_mutex_unlock(&chopsticks[id]);

    printf("Philosopher %d finished eating\n", id);

    return NULL;
}

int main()
{
    pthread_t threads[N];
    int ids[N];

    pthread_mutex_init(&waiter, NULL);

    for (int i = 0; i < N; i++)
        pthread_mutex_init(&chopsticks[i], NULL);

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&chopsticks[i]);

    pthread_mutex_destroy(&waiter);

    return 0;
}