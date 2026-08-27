#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#define SIZE 1000000  // 1 million elements
#define SIZE 18250
// Swap two elements
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}// Bubble Sort
void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
}

// Partition function for QuickSort
int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

// QuickSort recursive function
void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main() {
    int *original = (int *)malloc(SIZE * sizeof(int));
int *bubbleArr = (int *)malloc(SIZE * sizeof(int));
int *quickArr = (int *)malloc(SIZE * sizeof(int));

if (!original || !bubbleArr || !quickArr) {
    printf("Memory allocation failed\n");
    return 1;
}

    srand(time(NULL));

    // Fill array with random integers
    for (int i = 0; i < SIZE; i++) {
    original[i] = rand() % 100;
    bubbleArr[i] = original[i];
    quickArr[i] = original[i];
}
    // Measure execution time
    clock_t start, end;

/* Bubble Sort */
start = clock();
bubbleSort(bubbleArr, SIZE);
end = clock();

double bubbleTime = (double)(end - start) / CLOCKS_PER_SEC;

/* Quick Sort */
start = clock();
quickSort(quickArr, 0, SIZE - 1);
end = clock();

double quickTime = (double)(end - start) / CLOCKS_PER_SEC;

printf("Bubble Sort Time : %.6f seconds\n", bubbleTime);
printf("Quick Sort Time  : %.6f seconds\n", quickTime);
    free(original);
free(bubbleArr);
free(quickArr);
    return 0;
}
