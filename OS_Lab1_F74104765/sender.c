#include "sender.h"

void send(message_t message, mailbox_t* mailbox_ptr) {
    clock_t start_time, end_time;

    if (mailbox_ptr->flag == 1) {
        // Message Passing 使用 msgsnd() 發送訊息
        start_time = clock();  // 開始計時
        if (msgsnd(mailbox_ptr->storage.msqid, &message, sizeof(message.msg_text), 0) == -1) {
            perror("msgsnd failed");
            exit(1);
        }
        end_time = clock();  // 結束計時
        total_time += (double)(end_time - start_time) / CLOCKS_PER_SEC;
    } else if (mailbox_ptr->flag == 2) {
        // Shared Memory 寫入訊息到共享記憶體
        sem_wait(sem_receiver);                                   // 等待 receiver 準備好
        start_time = clock();                                     // 開始計時
        strcpy(mailbox_ptr->storage.shm_addr, message.msg_text);  // 寫入共享記憶體
        end_time = clock();                                       // 結束計時
        sem_post(sem_sender);                                     // 通知 receiver 可以讀取
        total_time += (double)(end_time - start_time) / CLOCKS_PER_SEC;
    } else {
        fprintf(stderr, "Unknown communication method\n");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <method> <input_file>\n", argv[0]);
        exit(1);
    }

    int method = atoi(argv[1]);  // 1 for Message Passing, 2 for Shared Memory
    char* input_file = argv[2];

    mailbox_t mailbox;
    mailbox.flag = method;

    key_t key;
    int shmid;

    sem_sender = sem_open("/sem_sender", O_CREAT, 0644, 0);      // Sender 初始化為 0
    sem_receiver = sem_open("/sem_receiver", O_CREAT, 0644, 1);  // Receiver 初始化為 1

    // 根據選擇的方式初始化 mail box（例如創建 msg queue 或共享記憶體）
    if (method == 1) {
        // 初始化訊息隊列
        key = ftok("progfile", 65);
        mailbox.storage.msqid = msgget(key, 0666 | IPC_CREAT);
        if (mailbox.storage.msqid == -1) {
            perror("msgget failed");
            exit(1);
        }
    } else if (method == 2) {
        // 初始化共享記憶體
        key = ftok("progfile", 65);
        shmid = shmget(key, 1024, 0666 | IPC_CREAT);
        mailbox.storage.shm_addr = (char*)shmat(shmid, (void*)0, 0);
        if (mailbox.storage.shm_addr == (char*)(-1)) {
            perror("shmat failed");
            exit(1);
        }
    }

    // 打開文件讀取訊息
    FILE* file = fopen(input_file, "r");
    if (!file) {
        perror("fopen failed");
        exit(1);
    }

    message_t message;
    // clock_t start_time = clock();  // 開始計時

    printf("Message Passing\n");

    while (fgets(message.msg_text, sizeof(message.msg_text), file) != NULL) {
        send(message, &mailbox);
        // 顯示發送的訊息內容
        printf("Sending message: %s", message.msg_text);
    }

    // 發送退出訊息
    strcpy(message.msg_text, "EOF");
    send(message, &mailbox);

    // unlink the share memory
    if (method == 2) {
        shmdt(mailbox.storage.shm_addr);
        shmctl(shmid, IPC_RMID, NULL);
    }  // unlink the message queue
    else {
        msgctl(mailbox.storage.msqid, IPC_RMID, NULL);
    }

    printf("\nEnd of input file! exit!\n");

    // clock_t end_time = clock();  // 結束計時
    // double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    // 打印總的發送時間
    printf("Total sending time: %f seconds\n", total_time);

    fclose(file);
    return 0;
}
