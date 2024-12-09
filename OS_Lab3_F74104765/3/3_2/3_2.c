#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define PROC_PATH "/proc/mythread_info"

// 獲取當前時間（毫秒）
long get_time_in_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000L + tv.tv_usec / 1000L;
}

// 寫入 proc 檔案的函數
void write_to_proc(const char *message) {
    FILE *fp = fopen(PROC_PATH, "w");
    if (fp == NULL) {
        perror("Failed to open /proc/mythread_info for writing");
        return;
    }

    long start_time = get_time_in_ms();
    fwrite(message, sizeof(char), strlen(message), fp);
    fclose(fp);
    long end_time = get_time_in_ms();

    printf("PID: %d, TID: %ld, Execution Time: %ld ms\n", getpid(), pthread_self(), end_time - start_time);
}

// 單線程寫入
void *single_thread_write(void *arg) {
    const char *message = "Single Thread Writing to /proc/mythread_info\n";
    write_to_proc(message);
    return NULL;
}

// 雙線程寫入
void *multi_thread_write(void *arg) {
    const char *message = "Multi Thread Writing to /proc/mythread_info\n";
    write_to_proc(message);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [single|multi]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "single") == 0) {
        pthread_t thread;
        pthread_create(&thread, NULL, single_thread_write, NULL);
        pthread_join(thread, NULL);
    } else if (strcmp(argv[1], "multi") == 0) {
        pthread_t thread1, thread2;
        pthread_create(&thread1, NULL, multi_thread_write, NULL);
        pthread_create(&thread2, NULL, multi_thread_write, NULL);
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
    } else {
        printf("Invalid argument. Use 'single' or 'multi'.\n");
        return 1;
    }

    return 0;
}