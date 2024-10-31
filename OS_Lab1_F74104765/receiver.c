#include "receiver.h"

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr) {
    clock_t start_time, end_time;

    if (mailbox_ptr->flag == 1) {
        // Message Passing 使用 msgrcv() 接收訊息
        start_time = clock();  // 開始計時
        if (msgrcv(mailbox_ptr->storage.msqid, message_ptr, sizeof(message_ptr->msg_text), 0, 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }
        end_time = clock();  // 結束計時
        total_time += (double)(end_time - start_time) / CLOCKS_PER_SEC;
    } else if (mailbox_ptr->flag == 2) {
        // Shared Memory 從共享記憶體讀取訊息
        sem_wait(sem_sender);                                          // 等待 sender 寫入完成
        start_time = clock();                                          // 開始計時
        strcpy(message_ptr->msg_text, mailbox_ptr->storage.shm_addr);  // 從共享記憶體讀取訊息
        end_time = clock();                                            // 結束計時
        sem_post(sem_receiver);                                        // 通知 sender 可以繼續寫入
        total_time += (double)(end_time - start_time) / CLOCKS_PER_SEC;
    } else {
        fprintf(stderr, "Unknown communication method\n");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <method>\n", argv[0]);
        exit(1);
    }

    int method = atoi(argv[1]);
    mailbox_t mailbox;
    mailbox.flag = method;

    sem_sender = sem_open("/sem_sender", O_CREAT, 0644, 0);      // Sender 初始化為 0
    sem_receiver = sem_open("/sem_receiver", O_CREAT, 0644, 1);  // Receiver 初始化為 1

    // 根據選擇的方式初始化 mail box（例如取得 msg queue 或共享記憶體）
    if (method == 1) {
        key_t key = ftok("progfile", 65);
        mailbox.storage.msqid = msgget(key, 0666);
        if (mailbox.storage.msqid == -1) {
            perror("msgget failed");
            exit(1);
        }
    } else if (method == 2) {
        key_t key = ftok("progfile", 65);
        int shmid = shmget(key, 1024, 0666);
        mailbox.storage.shm_addr = (char*)shmat(shmid, (void*)0, 0);
        if (mailbox.storage.shm_addr == (char*)(-1)) {
            perror("shmat failed");
            exit(1);
        }
    }

    message_t message;
    // clock_t start_time = clock();  // 開始計時

    printf("Message Passing\n");

    while (1) {
        receive(&message, &mailbox);
        if (strcmp(message.msg_text, "EOF") == 0) break;
        // 顯示接收的訊息內容
        printf("Receiving message: %s", message.msg_text);
    }

    // clock_t end_time = clock();  // 結束計時
    // double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\nSender exit!\n");
    printf("Total time taken in receiving msg: %f s\n", total_time);

    return 0;
}
