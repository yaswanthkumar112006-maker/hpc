#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define N 5

pthread_mutex_t chopsticks[N];

void* philosopher(void* num)
{
    int id = *(int*)num;

    int left = id;
    int right = (id + 1) % N;

    int first = left < right ? left : right;
    int second = left < right ? right : left;

    printf("Philosopher %d is thinking\n", id);
    sleep(1);

    printf("Philosopher %d is hungry\n", id);

    pthread_mutex_lock(&chopsticks[first]);
    printf("Philosopher %d picked chopstick %d\n", id, first);

    pthread_mutex_lock(&chopsticks[second]);
    printf("Philosopher %d picked chopstick %d\n", id, second);

    printf("Philosopher %d is eating\n", id);
    sleep(1);

    pthread_mutex_unlock(&chopsticks[second]);
    pthread_mutex_unlock(&chopsticks[first]);

    printf("Philosopher %d finished eating\n", id);

    return NULL;
}

int main()
{
    pthread_t philo[N];
    int ids[N];

    for (int i = 0; i < N; i++)
        pthread_mutex_init(&chopsticks[i], NULL);

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&philo[i], NULL, philosopher, &ids[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(philo[i], NULL);

    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&chopsticks[i]);

    return 0;
}