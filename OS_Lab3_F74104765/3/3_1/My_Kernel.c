#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define PROC_NAME "mythread_info"
#define BUFFER_SIZE 128

static struct proc_dir_entry *entry;

// proc_read 函數，用於讀取 proc 檔案的內容
static ssize_t proc_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    char buffer[BUFFER_SIZE];
    int len = 0;

    // 確保每次讀取只返回一次內容
    if (*offset > 0) {
        return 0;
    }

    // 獲取當前線程的資訊
    pid_t pid = current->pid;
    pid_t tgid = current->tgid;
    int priority = current->prio;
    char state[20];

    // 轉換線程狀態為可讀格式
    switch (current->__state) {  // 改為 __state
    case TASK_RUNNING:
        strcpy(state, "RUNNING");
        break;
    case TASK_INTERRUPTIBLE:
        strcpy(state, "INTERRUPTIBLE");
        break;
    case TASK_UNINTERRUPTIBLE:
        strcpy(state, "UNINTERRUPTIBLE");
        break;
    default:
        strcpy(state, "UNKNOWN");
        break;
    }

    // 將資訊寫入 buffer
    len = snprintf(buffer, BUFFER_SIZE, "Process ID (PID): %d\nThread ID (TID): %d\nPriority: %d\nState: %s\n",
                   tgid, pid, priority, state);

    // 將資料複製到 userspace
    if (copy_to_user(user_buffer, buffer, len)) {
        return -EFAULT;
    }

    *offset += len;
    return len;
}

// 定義 proc 檔案的文件操作結構
static const struct proc_ops proc_fops = {
    // 使用 proc_ops
    .proc_read = proc_read,
};

// 模組初始化函數，建立 proc 檔案
static int __init my_module_init(void) {
    entry = proc_create(PROC_NAME, 0444, NULL, &proc_fops);  // 使用 proc_fops
    if (entry == NULL) {
        printk(KERN_ALERT "Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

// 模組出口函數，移除 proc 檔案
static void __exit my_module_exit(void) {
    proc_remove(entry);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Linux kernel module to display thread info via /proc/mythread_info");
MODULE_VERSION("1.0");

module_init(my_module_init);
module_exit(my_module_exit);
