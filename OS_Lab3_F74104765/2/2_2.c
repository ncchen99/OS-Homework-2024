#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MATRIX_ROW_X 1234
#define MATRIX_COL_X 250

#define MATRIX_ROW_Y 250
#define MATRIX_COL_Y 4

pthread_spinlock_t lock;
FILE *fptr1;
FILE *fptr2;
FILE *fptr3;
int **x;
int **y;
int **z;

// 結果矩陣初始化
void initialize_result_matrix() {
    z = malloc(sizeof(int *) * MATRIX_ROW_X);
    if (z == NULL) {
        perror("Failed to allocate memory for matrix z");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MATRIX_ROW_X; i++) {
        z[i] = malloc(sizeof(int) * MATRIX_COL_Y);
        if (z[i] == NULL) {
            perror("Failed to allocate memory for matrix z row");
            exit(EXIT_FAILURE);
        }
        // 初始化為 0
        memset(z[i], 0, sizeof(int) * MATRIX_COL_Y);
    }
}

// 讀取矩陣資料
void read_matrices() {
    int tmp;
    // 讀取矩陣 X
    fscanf(fptr1, "%d", &tmp);
    fscanf(fptr1, "%d", &tmp);
    for (int i = 0; i < MATRIX_ROW_X; i++) {
        for (int j = 0; j < MATRIX_COL_X; j++) {
            if (fscanf(fptr1, "%d", &x[i][j]) != 1) {
                printf("Error reading from m1.txt at row %d, col %d\n", i, j);
                exit(EXIT_FAILURE);
            }
        }
    }

    // 讀取矩陣 Y
    fscanf(fptr2, "%d", &tmp);
    fscanf(fptr2, "%d", &tmp);
    for (int i = 0; i < MATRIX_ROW_Y; i++) {
        for (int j = 0; j < MATRIX_COL_Y; j++) {
            if (fscanf(fptr2, "%d", &y[i][j]) != 1) {
                printf("Error reading from m2.txt at row %d, col %d\n", i, j);
                exit(EXIT_FAILURE);
            }
        }
    }
}

// 線程參數結構
typedef struct {
    int start_row;
    int end_row;
} thread_param;

// 矩陣乘法線程函數
void *matrix_multiplication_thread(void *arg) {
    thread_param *param = (thread_param *)arg;
    for (int i = param->start_row; i < param->end_row; i++) {
        for (int j = 0; j < MATRIX_COL_Y; j++) {
            int res = 0;
            for (int k = 0; k < MATRIX_ROW_Y; k++) {
                res += x[i][k] * y[k][j];
            }
            // 鎖定以保護寫入結果矩陣
            pthread_spin_lock(&lock);
            z[i][j] = res;
            pthread_spin_unlock(&lock);
        }
    }
    pthread_exit(NULL);
}

int main() {
    // 初始化鎖
    if (pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE) != 0) {
        perror("Failed to initialize spinlock");
        exit(EXIT_FAILURE);
    }

    // 分配矩陣 X
    x = malloc(sizeof(int *) * MATRIX_ROW_X);
    if (x == NULL) {
        perror("Failed to allocate memory for matrix x");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MATRIX_ROW_X; i++) {
        x[i] = malloc(sizeof(int) * MATRIX_COL_X);
        if (x[i] == NULL) {
            perror("Failed to allocate memory for matrix x row");
            exit(EXIT_FAILURE);
        }
    }

    // 分配矩陣 Y
    y = malloc(sizeof(int *) * MATRIX_ROW_Y);
    if (y == NULL) {
        perror("Failed to allocate memory for matrix y");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MATRIX_ROW_Y; i++) {
        y[i] = malloc(sizeof(int) * MATRIX_COL_Y);
        if (y[i] == NULL) {
            perror("Failed to allocate memory for matrix y row");
            exit(EXIT_FAILURE);
        }
    }

    printf("Memory allocation successful\n");

    // 打開文件
    fptr1 = fopen("m1.txt", "r");
    fptr2 = fopen("m2.txt", "r");
    fptr3 = fopen("2.txt", "a");

    if (fptr1 == NULL || fptr2 == NULL || fptr3 == NULL) {
        printf("Error opening files.\n");
        return 1;
    }
    printf("File opening successful\n");

    // 讀取矩陣資料
    read_matrices();
    printf("Matrix reading successful\n");

    // 初始化結果矩陣
    initialize_result_matrix();

    // 設定線程參數
    pthread_t thread1, thread2;
    thread_param param1, param2;

    // 將矩陣 A 沿 K 方向切分
    int mid = MATRIX_ROW_X / 2;
    param1.start_row = 0;
    param1.end_row = mid;
    param2.start_row = mid;
    param2.end_row = MATRIX_ROW_X;

    // 創建線程
    if (pthread_create(&thread1, NULL, matrix_multiplication_thread, (void *)&param1) != 0) {
        perror("Failed to create thread 1");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&thread2, NULL, matrix_multiplication_thread, (void *)&param2) != 0) {
        perror("Failed to create thread 2");
        exit(EXIT_FAILURE);
    }

    // 等待線程完成
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("Matrix multiplication successful\n");

    // 將結果寫入文件
    fprintf(fptr3, "%d %d\n", MATRIX_ROW_X, MATRIX_COL_Y);
    for (int i = 0; i < MATRIX_ROW_X; i++) {
        for (int j = 0; j < MATRIX_COL_Y; j++) {
            fprintf(fptr3, "%d ", z[i][j]);
        }
        fprintf(fptr3, "\n");
    }
    printf("Result writing successful\n");

    // 釋放資源
    fclose(fptr1);
    fclose(fptr2);
    fclose(fptr3);
    printf("File closing successful\n");

    for (int i = 0; i < MATRIX_ROW_X; i++) {
        free(x[i]);
    }
    free(x);

    for (int i = 0; i < MATRIX_ROW_Y; i++) {
        free(y[i]);
    }
    free(y);

    for (int i = 0; i < MATRIX_ROW_X; i++) {
        free(z[i]);
    }
    free(z);

    // 銷毀鎖
    pthread_spin_destroy(&lock);

    return 0;
}
