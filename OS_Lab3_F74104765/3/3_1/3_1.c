#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define matrix_row_x 1234
#define matrix_col_x 250

#define matrix_row_y 250
#define matrix_col_y 4

FILE *fptr1;
FILE *fptr2;
FILE *fptr3;
int **x;
int **y;
int **z;

// Put file data into x and y arrays
void data_processing(void) {
    int tmp;
    fscanf(fptr1, "%d", &tmp);
    fscanf(fptr1, "%d", &tmp);
    for (int i = 0; i < matrix_row_x; i++) {
        for (int j = 0; j < matrix_col_x; j++) {
            if (fscanf(fptr1, "%d", &x[i][j]) != 1) {
                printf("Error reading from file\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    fscanf(fptr2, "%d", &tmp);
    fscanf(fptr2, "%d", &tmp);
    for (int i = 0; i < matrix_row_y; i++) {
        for (int j = 0; j < matrix_col_y; j++) {
            if (fscanf(fptr2, "%d", &y[i][j]) != 1) {
                printf("Error reading from file\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void *thread1(void *arg) {
    for (int i = 0; i < matrix_row_x / 2; i++) {
        for (int j = 0; j < matrix_col_y; j++) {
            for (int k = 0; k < matrix_row_y; k++) {
                z[i][j] += x[i][k] * y[k][j];
            }
        }
    }
    return NULL;
}

void *thread2(void *arg) {
    for (int i = matrix_row_x / 2; i < matrix_row_x; i++) {
        for (int j = 0; j < matrix_col_y; j++) {
            for (int k = 0; k < matrix_row_y; k++) {
                z[i][j] += x[i][k] * y[k][j];
            }
        }
    }
    return NULL;
}

int main() {
    // Allocate memory for matrices
    x = malloc(sizeof(int *) * matrix_row_x);
    if (x == NULL) {
        perror("Memory allocation failed for x");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < matrix_row_x; i++) {
        x[i] = malloc(sizeof(int) * matrix_col_x);
        if (x[i] == NULL) {
            perror("Memory allocation failed for x[i]");
            exit(EXIT_FAILURE);
        }
    }

    y = malloc(sizeof(int *) * matrix_row_y);
    if (y == NULL) {
        perror("Memory allocation failed for y");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < matrix_row_y; i++) {
        y[i] = malloc(sizeof(int) * matrix_col_y);
        if (y[i] == NULL) {
            perror("Memory allocation failed for y[i]");
            exit(EXIT_FAILURE);
        }
    }

    z = malloc(sizeof(int *) * matrix_row_x);
    if (z == NULL) {
        perror("Memory allocation failed for z");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < matrix_row_x; i++) {
        z[i] = malloc(sizeof(int) * matrix_col_y);
        if (z[i] == NULL) {
            perror("Memory allocation failed for z[i]");
            exit(EXIT_FAILURE);
        }
        memset(z[i], 0, sizeof(int) * matrix_col_y);
    }

    // Open files
    fptr1 = fopen("m1.txt", "r");
    if (fptr1 == NULL) {
        perror("Error opening m1.txt");
        exit(EXIT_FAILURE);
    }

    fptr2 = fopen("m2.txt", "r");
    if (fptr2 == NULL) {
        perror("Error opening m2.txt");
        exit(EXIT_FAILURE);
    }

    fptr3 = fopen("3_1.txt", "a");
    if (fptr3 == NULL) {
        perror("Error opening 3_1.txt");
        exit(EXIT_FAILURE);
    }

    // Process data
    printf("Reading from file\n");
    data_processing();

    // Create threads
    printf("Creating threads\n");
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    // Wait for threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Write results to file
    printf("Writing to file\n");
    fprintf(fptr3, "%d %d\n", matrix_row_x, matrix_col_y);
    for (int i = 0; i < matrix_row_x; i++) {
        for (int j = 0; j < matrix_col_y; j++) {
            fprintf(fptr3, "%d ", z[i][j]);
        }
        fprintf(fptr3, "\n");
    }

    // Cleanup
    fclose(fptr1);
    fclose(fptr2);
    fclose(fptr3);

    for (int i = 0; i < matrix_row_x; i++) {
        free(x[i]);
        free(z[i]);
    }
    free(x);
    free(z);

    for (int i = 0; i < matrix_row_y; i++) {
        free(y[i]);
    }
    free(y);

    return 0;
}
