#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LENGTH_V1 1000
#define LENGTH_V2 10000
#define LENGTH_V3 100000
#define RUNS 3

typedef void (*sort_fn)(int list[], int n, unsigned long long *moves);

typedef struct
{
    double times[RUNS];
    unsigned long long moves[RUNS];
    double mean;
    double stddev;
} BenchmarkResult;

void swap(int *xp, int *yp, unsigned long long *moves);

void bubble_sort(int list[], int n, unsigned long long *moves);

void merge_sort(int list[], int n, unsigned long long *moves);
void merge_sort_ordena(int list[], int left, int right, unsigned long long *moves);
void merge(int list[], int left, int mid, int right, unsigned long long *moves);

void quick_sort(int list[], int n, unsigned long long *moves);
void quick_sort_handler(int list[], int low, int high, unsigned long long *moves);
int partition(int list[], int low, int high, unsigned long long *moves);

void fill_random_array(int list[], int n);
void copy_array(const int source[], int target[], int n);
double now_in_seconds(void);
BenchmarkResult run_benchmark(sort_fn algorithm, const int original[], int n);
void print_result(const char *name, int n, BenchmarkResult result, const char *move_label);

int main(void)
{
    const int sizes[] = {LENGTH_V1, LENGTH_V2, LENGTH_V3};
    const int total_sizes = (int) (sizeof(sizes) / sizeof(sizes[0]));

    setvbuf(stdout, NULL, _IONBF, 0);
    srand(42);

    for (int i = 0; i < total_sizes; i++)
    {
        int size = sizes[i];
        int *original = malloc(sizeof(int) * size);

        if (original == NULL)
        {
            fprintf(stderr, "Erro ao alocar o vetor original de tamanho %d.\n", size);
            return 1;
        }

        fill_random_array(original, size);

        printf("========================================\n");
        printf("Tamanho do vetor: %d\n", size);
        printf("========================================\n");

        print_result("Bubble Sort", size, run_benchmark(bubble_sort, original, size), "trocas");
        print_result("Quick Sort", size, run_benchmark(quick_sort, original, size), "trocas");
        print_result("Merge Sort", size, run_benchmark(merge_sort, original, size), "movimentacoes");

        free(original);
    }

    return 0;
}

void swap(int *xp, int *yp, unsigned long long *moves)
{
    if (xp == yp)
    {
        return;
    }

    int temp = *xp;
    *xp = *yp;
    *yp = temp;
    (*moves)++;
}

void bubble_sort(int list[], int n, unsigned long long *moves)
{
    for (int i = 0; i < n - 1; i++)
    {
        int swapped = 0;

        for (int j = 0; j < n - i - 1; j++)
        {
            if (list[j] > list[j + 1])
            {
                swap(&list[j], &list[j + 1], moves);
                swapped = 1;
            }
        }

        if (!swapped)
        {
            break;
        }
    }
}

void merge_sort(int list[], int n, unsigned long long *moves)
{
    if (n <= 1)
    {
        return;
    }

    merge_sort_ordena(list, 0, n - 1, moves);
}

void merge_sort_ordena(int list[], int left, int right, unsigned long long *moves)
{
    if (left == right)
    {
        return;
    }

    int mid = (left + right) / 2;
    merge_sort_ordena(list, left, mid, moves);
    merge_sort_ordena(list, mid + 1, right, moves);
    merge(list, left, mid, right, moves);
    return;
}

void merge(int list[], int left, int mid, int right, unsigned long long *moves)
{
    int i, j, k;
    int a_length = mid - left + 1;
    int b_length = right - mid;

    int *a = (int *) malloc(sizeof(int) * a_length);
    int *b = (int *) malloc(sizeof(int) * b_length);

    if (a == NULL || b == NULL)
    {
        fprintf(stderr, "Erro ao alocar vetores auxiliares do merge sort.\n");
        free(a);
        free(b);
        exit(1);
    }

    for (i = 0; i < a_length; i++) a[i] = list[i + left];
    for (i = 0; i < b_length; i++) b[i] = list[i + mid + 1];

    for (i = 0, j = 0, k = left; k <= right; k++)
    {
        if (i == a_length) list[k] = b[j++];
        else if (j == b_length) list[k] = a[i++];
        else if (a[i] < b[j]) list[k] = a[i++];
        else list[k] = b[j++];

        (*moves)++;
    }

    free(a);
    free(b);
}

void quick_sort(int list[], int n, unsigned long long *moves)
{
    if (n <= 1)
    {
        return;
    }

    quick_sort_handler(list, 0, n - 1, moves);
}

void quick_sort_handler(int list[], int low, int high, unsigned long long *moves)
{
    if (low < high)
    {
        int pi = partition(list, low, high, moves);

        quick_sort_handler(list, low, pi - 1, moves);
        quick_sort_handler(list, pi + 1, high, moves);
    }
}

int partition(int list[], int low, int high, unsigned long long *moves)
{
    int pivot = list[high];
    int i = low - 1;

    for (int j = low; j < high; j++)
    {
        if (list[j] <= pivot)
        {
            i++;
            swap(&list[i], &list[j], moves);
        }
    }

    swap(&list[i + 1], &list[high], moves);
    return i + 1;
}

void fill_random_array(int list[], int n)
{
    for (int i = 0; i < n; i++)
    {
        list[i] = rand();
    }
}

void copy_array(const int source[], int target[], int n)
{
    for (int i = 0; i < n; i++)
    {
        target[i] = source[i];
    }
}

double now_in_seconds(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (double) ts.tv_sec + (double) ts.tv_nsec / 1000000000.0;
}

BenchmarkResult run_benchmark(sort_fn algorithm, const int original[], int n)
{
    BenchmarkResult result = {0};
    int *working = malloc(sizeof(int) * n);

    if (working == NULL)
    {
        fprintf(stderr, "Erro ao alocar o vetor de trabalho de tamanho %d.\n", n);
        exit(1);
    }

    for (int run = 0; run < RUNS; run++)
    {
        unsigned long long moves = 0;
        copy_array(original, working, n);

        double start = now_in_seconds();
        algorithm(working, n, &moves);
        double end = now_in_seconds();

        result.times[run] = end - start;
        result.moves[run] = moves;
        result.mean += result.times[run];
    }

    result.mean /= RUNS;

    for (int run = 0; run < RUNS; run++)
    {
        double diff = result.times[run] - result.mean;
        result.stddev += diff * diff;
    }

    result.stddev = sqrt(result.stddev / RUNS);

    free(working);
    return result;
}

void print_result(const char *name, int n, BenchmarkResult result, const char *move_label)
{
    (void) n;

    printf("%s\n", name);
    printf("Execucao 1: %.6f s | %s: %llu\n", result.times[0], move_label, result.moves[0]);
    printf("Execucao 2: %.6f s | %s: %llu\n", result.times[1], move_label, result.moves[1]);
    printf("Execucao 3: %.6f s | %s: %llu\n", result.times[2], move_label, result.moves[2]);
    printf("Tempo medio: %.6f s\n", result.mean);
    printf("Desvio padrao: %.6f s\n", result.stddev);
    printf("\n");
}
