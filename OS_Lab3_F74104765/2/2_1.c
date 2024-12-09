#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
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

// 單線程完成矩陣乘法
void matrix_multiplication(void) {
    int res;
    for (int i = 0; i < matrix_row_x; i++) {
        for (int j = 0; j < matrix_col_y; j++) {
            res = 0;
            for (int k = 0; k < matrix_row_y; k++) {
                res += x[i][k] * y[k][j];
            }
            fprintf(fptr3, "%d ", res);
            if (j == matrix_col_y - 1) fprintf(fptr3, "\n");
        }
    }
}

int main() {
    x = malloc(sizeof(int *) * matrix_row_x);
    if (x == NULL) {
        perror("Failed to allocate memory for matrix x");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < matrix_row_x; i++) {
        x[i] = malloc(sizeof(int) * matrix_col_x);
        if (x[i] == NULL) {
            perror("Failed to allocate memory for matrix x row");
            exit(EXIT_FAILURE);
        }
    }

    y = malloc(sizeof(int *) * matrix_row_y);
    if (y == NULL) {
        perror("Failed to allocate memory for matrix y");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < matrix_row_y; i++) {
        y[i] = malloc(sizeof(int) * matrix_col_y);
        if (y[i] == NULL) {
            perror("Failed to allocate memory for matrix y row");
            exit(EXIT_FAILURE);
        }
    }
    printf("Memory allocation successful\n");
    fptr1 = fopen("m1.txt", "r");
    fptr2 = fopen("m2.txt", "r");
    fptr3 = fopen("2.txt", "a");

    if (fptr1 == NULL || fptr2 == NULL || fptr3 == NULL) {
        printf("Error opening files.\n");
        return 1;
    }
    printf("File opening successful\n");
    // 讀取資料
    int tmp;
    fscanf(fptr1, "%d", &tmp);
    fscanf(fptr1, "%d", &tmp);
    for (int i = 0; i < matrix_row_x; i++) {
        for (int j = 0; j < matrix_col_x; j++) {
            if (fscanf(fptr1, "%d", &x[i][j]) != 1) {
                printf("Error reading from m1.txt\n");
                return 1;
            }
        }
    }
    printf("Matrix x reading successful\n");

    fscanf(fptr2, "%d", &tmp);
    fscanf(fptr2, "%d", &tmp);
    for (int i = 0; i < matrix_row_y; i++) {
        for (int j = 0; j < matrix_col_y; j++) {
            if (fscanf(fptr2, "%d", &y[i][j]) != 1) {
                printf("Error reading from m2.txt\n");
                return 1;
            }
        }
    }
    printf("Matrix y reading successful\n");
    fprintf(fptr3, "%d %d\n", matrix_row_x, matrix_col_y);

    // 執行單線程矩陣乘法
    matrix_multiplication();
    printf("Matrix multiplication successful\n");
    // 釋放資源
    fclose(fptr1);
    fclose(fptr2);
    fclose(fptr3);
    printf("File closing successful\n");
    for (int i = 0; i < matrix_row_x; i++) {
        free(x[i]);
    }
    free(x);

    for (int i = 0; i < matrix_row_y; i++) {
        free(y[i]);
    }
    free(y);

    return 0;
}