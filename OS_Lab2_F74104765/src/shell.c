#include <bits/local_lim.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/builtin.h"
#include "../include/command.h"

// ======================= requirement 2.3 =======================
/**
 * @brief
 * Redirect command's stdin and stdout to the specified file descriptor
 * If you want to implement ( < , > ), use "in_file" and "out_file" included the cmd_node structure
 * If you want to implement ( | ), use "in" and "out" included the cmd_node structure.
 *
 * @param p cmd_node structure
 *
 */

void redirection(struct cmd_node *p) {
    if (p->in_file) {
        int fd_in = open(p->in_file, O_RDONLY);
        if (fd_in == -1) {
            perror("open for input");
            return;
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
    if (p->out_file) {
        int fd_out = open(p->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out == -1) {
            perror("open for output");
            return;
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
}
// ===============================================================

// ======================= requirement 2.2 =======================
/**
 * @brief
 * Execute external command
 * The external command is mainly divided into the following two steps:
 * 1. Call "fork()" to create child process
 * 2. Call "execvp()" to execute the corresponding executable file
 * @param p cmd_node structure
 * @return int
 * Return execution status
 */
int spawn_proc(struct cmd_node *p) {
    pid_t pid = fork();
    if (pid == 0) {
        // 子進程執行 redirection
        redirection(p);

        // 執行外部命令
        if (execvp(p->args[0], p->args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        // fork 失敗
        perror("fork");
    } else {
        // 父進程等待子進程結束
        int status;
        waitpid(pid, &status, 0);
    }
    return 1;
}

// ===============================================================

// ======================= requirement 2.4 =======================
/**
 * @brief
 * Use "pipe()" to create a communication bridge between processes
 * Call "spawn_proc()" in order according to the number of cmd_node
 * @param cmd Command structure
 * @return int
 * Return execution status
 */
// shell.c

int fork_cmd_node(struct cmd *cmd) {
    int num_pipes = cmd->pipe_num - 1;
    int pipe_fd[2 * num_pipes];
    pid_t pid;
    int status;

    // 創建所需的管道
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipe_fd + i * 2) == -1) {
            perror("pipe");
            return -1;
        }
    }

    struct cmd_node *current = cmd->head;
    for (int i = 0; current != NULL; i++, current = current->next) {
        pid = fork();
        if (pid == 0) {
            // 子進程處理輸入和輸出
            if (i > 0) {
                // 不是第一個指令，將標準輸入重定向為前一個管道的讀端
                dup2(pipe_fd[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < num_pipes) {
                // 不是最後一個指令，將標準輸出重定向為當前管道的寫端
                dup2(pipe_fd[i * 2 + 1], STDOUT_FILENO);
            } else {
                // 如果是最後一個命令節點，並且存在 out_file，則進行文件重定向
                if (current->out_file) {
                    redirection(current);
                }
            }

            // 關閉所有無用的文件描述符
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipe_fd[j]);
            }

            // 執行命令
            if (execvp(current->args[0], current->args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            // fork 失敗
            perror("fork");
            return -1;
        }
    }

    // 父進程關閉所有的管道
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipe_fd[i]);
    }

    // 父進程等待所有子進程完成
    for (int i = 0; i <= num_pipes; i++) {
        wait(&status);
    }

    return 1;
}

// ===============================================================

void shell() {
    char cwd[PATH_MAX];               // 用來儲存當前路徑
    char *username = getenv("USER");  // 取得使用者名稱
    char hostname[HOST_NAME_MAX];     // 儲存裝置名稱

    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname");
        strcpy(hostname, "unknown");
    }

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            strcpy(cwd, "unknown");
        }
        // 顯示提示符
        printf("%s@%s:%s$ ", username ? username : "user", hostname, cwd);

        char *buffer = read_line();
        if (buffer == NULL)
            continue;

        struct cmd *cmd = split_line(buffer);
        int status = -1;
        struct cmd_node *temp = cmd->head;

        if (temp->next == NULL) {
            status = searchBuiltInCommand(temp);
            if (status != -1) {
                int in = dup(STDIN_FILENO), out = dup(STDOUT_FILENO);
                if ((in == -1) | (out == -1))
                    perror("dup");
                redirection(temp);
                status = execBuiltInCommand(status, temp);

                // 恢復 shell 的標準輸入和輸出
                if (temp->in_file) dup2(in, 0);
                if (temp->out_file) dup2(out, 1);
                close(in);
                close(out);
            } else {
                // 外部命令
                status = spawn_proc(cmd->head);
            }
        } else {
            // 多個命令 (管道)
            status = fork_cmd_node(cmd);
        }

        // 釋放記憶體
        while (cmd->head) {
            struct cmd_node *temp = cmd->head;
            cmd->head = cmd->head->next;
            free(temp->args);
            free(temp);
        }
        free(cmd);
        free(buffer);

        if (status == 0)
            break;
    }
}
