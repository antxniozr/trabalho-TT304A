#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <limits.h> // Inclui INT_MAX

#define MAX_THREADS 8

typedef struct 
{
    int thread_id;
    int *data;
    int data_size;
    double execution_time;
    char *filename; // Adiciona o campo filename
} ThreadData;

// Função para mesclar duas metades ordenadas de um array
void merge(int *data, int left, int middle, int right) 
{
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 = right - middle;

    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    for (i = 0; i < n1; i++)
        L[i] = data[left + i];
    for (j = 0; j < n2; j++)
        R[j] = data[middle + 1 + j];

    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) 
    {
        if (L[i] <= R[j]) 
        {
            data[k] = L[i];
            i++;
        } 
        else 
        {
            data[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) 
    {
        data[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) 
    {
        data[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

// Função principal do mergesort
void merge_sort(int *data, int left, int right) 
{
    if (left < right) 
    {
        int middle = left + (right - left) / 2;
        merge_sort(data, left, middle);
        merge_sort(data, middle + 1, right);
        merge(data, left, middle, right);
    }
}

void *process_file(void *arg) 
{
    ThreadData *t_data = (ThreadData *)arg;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    FILE *file = fopen(t_data->filename, "r");
    if (!file) 
    {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", t_data->filename);
        pthread_exit(NULL);
    }

    int value;
    int size = 0;
    while (fscanf(file, "%d", &value) != EOF) 
    {
        t_data->data[size++] = value;
    }
    fclose(file);

    merge_sort(t_data->data, 0, size - 1);

    clock_gettime(CLOCK_MONOTONIC, &end);
    t_data->execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    t_data->data_size = size; // Define o tamanho dos dados processados
    pthread_exit(NULL);
}

int* final_merge(ThreadData thread_data[], int num_threads, int total_size) 
{
    int *merged_data = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)calloc(num_threads, sizeof(int));

    for (int i = 0; i < total_size; i++) 
    {
        int min_val = INT_MAX;
        int min_idx = -1;

        for (int j = 0; j < num_threads; j++) 
        {
            if (indices[j] < thread_data[j].data_size && thread_data[j].data[indices[j]] < min_val) 
            {
                min_val = thread_data[j].data[indices[j]];
                min_idx = j;
            }
        }

        merged_data[i] = min_val;
        indices[min_idx]++;
    }

    free(indices);
    return merged_data;
}

int main(int argc, char *argv[]) 
{
    if (argc < 4) 
    {
        fprintf(stderr, "Uso: %s <num_threads> <arq1> <arq2> ... -o <saida>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[1]);
    if (num_threads > MAX_THREADS) 
    {
        num_threads = MAX_THREADS;
    }

    int num_files = argc - 4; // Exclui <programa> <num_threads> <arquivos> -o <saida>
    char *output_file = argv[argc - 1];

    ThreadData thread_data[num_threads];
    pthread_t threads[num_threads];

    struct timespec total_start, total_end;
    clock_gettime(CLOCK_MONOTONIC, &total_start);

    int total_size = 0;
    for (int i = 0; i < num_threads; i++) 
    {
        thread_data[i].thread_id = i;
        thread_data[i].filename = argv[i + 2]; // Associa o arquivo ao campo filename
        thread_data[i].data = (int *)malloc(10000 * sizeof(int)); // Separa 10000 números por arquivo

        pthread_create(&threads[i], NULL, process_file, (void *)&thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) 
    {
        pthread_join(threads[i], NULL);
        total_size += thread_data[i].data_size; // Soma o tamanho dos dados de cada thread
        printf("Tempo de execução do Thread %d: %f segundos.\n", i, thread_data[i].execution_time); //Calcula o tempo de execução de cada Thread
    }

    clock_gettime(CLOCK_MONOTONIC, &total_end);
    double total_time = (total_end.tv_sec - total_start.tv_sec) + (total_end.tv_nsec - total_start.tv_nsec) / 1e9;
    printf("Tempo total de execução: %f segundos.\n", total_time);

    int *merged_data = final_merge(thread_data, num_threads, total_size);

    FILE *output = fopen(output_file, "w");
    if (!output) 
    {
        perror("Erro ao abrir o arquivo de saída");
        free(merged_data);
        return 1;
    }

    for (int i = 0; i < total_size; i++) 
    {
       fprintf(output, "%d\n", merged_data[i]);
    }

    fclose(output);
    free(merged_data);

    for (int i = 0; i < num_threads; i++) 
    {
        free(thread_data[i].data);
    }

    return 0;
}
