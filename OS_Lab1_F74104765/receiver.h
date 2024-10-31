#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    int flag;  // 1 for message passing, 2 for shared memory
    union {
        int msqid;  // for system V api. You can replace it with struecture for POSIX api
        char* shm_addr;
    } storage;
} mailbox_t;

typedef struct {
    long msg_type;        // 訊息類型 (通常為 long 型別，用來標識訊息，特別是在訊息隊列中)
    char msg_text[1024];  // 訊息內容
} message_t;

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr);

// 使用 POSIX API 解決同步問題（信號量）
sem_t* sem_sender;
sem_t* sem_receiver;

double total_time = 0;