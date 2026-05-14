#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ROWS_A 18
#define COLS_A 16
#define ROWS_B 16
#define COLS_B 18

int A[ROWS_A][COLS_A];
int B[ROWS_B][COLS_B];
int C[ROWS_A][COLS_B];

typedef struct {
    int row;
    int col;
} thread_data;

void *compute_C_ij(void *arg) {
    thread_data *data = (thread_data *)arg;
    int i = data->row;
    int j = data->col;
    int sum = 0;

    for (int k = 0; k < COLS_A; k++) {
        sum += A[i][k] * B[k][j];
    }
    C[i][j] = sum;
    
    free(data);  
    pthread_exit(NULL);
}

int main(void) {

    for (int i = 0; i < ROWS_A; i++) {
        for (int j = 0; j < COLS_A; j++) {
            A[i][j] = (i + 1) + (j + 1);
        }
    }
    
    for (int i = 0; i < ROWS_B; i++) {
        for (int j = 0; j < COLS_B; j++) {
            B[i][j] = (i + 1) + 2 * (j + 1);
        }
    }

    pthread_t threads[ROWS_A * COLS_B];
    int thread_count = 0;
    
    for (int i = 0; i < ROWS_A; i++) {
        for (int j = 0; j < COLS_B; j++) {
            thread_data *data = malloc(sizeof(thread_data));
            if (data == NULL) {
                perror("Failed to allocate memory");
                exit(EXIT_FAILURE);
            }
            data->row = i;
            data->col = j;
            
            int rc = pthread_create(&threads[thread_count], NULL, compute_C_ij, data);
            if (rc) {
                fprintf(stderr, "Error: pthread_create, rc: %d\n", rc);
                exit(EXIT_FAILURE);
            }
            thread_count++;
        }
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Diagonal elements of matrix C:\n");
    for (int i = 0; i < ROWS_A && i < COLS_B; i++) {
        printf("C[%d][%d] = %d\n", i + 1, i + 1, C[i][i]);
    }
    
    return 0;
}
