#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/timekeeping.h>
#include <linux/uaccess.h>

#define PROC_NAME "mythread_info"
#define BUFFER_SIZE 4096

static struct proc_dir_entry *entry;

// 緩衝區，用於儲存讀取和寫入的資訊
static char proc_buffer[BUFFER_SIZE];
static size_t proc_buffer_size = 0;

// 自旋鎖，用於保護緩衝區
static spinlock_t proc_lock;

// proc_read 函數，用於讀取 proc 檔案的內容
static ssize_t proc_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    ssize_t ret = 0;

    if (*offset > 0) {
        return 0;
    }

    spin_lock(&proc_lock);
    if (proc_buffer_size > size) {
        spin_unlock(&proc_lock);
        return -EINVAL;
    }

    if (copy_to_user(user_buffer, proc_buffer, proc_buffer_size)) {
        spin_unlock(&proc_lock);
        return -EFAULT;
    }

    *offset += proc_buffer_size;
    ret = proc_buffer_size;
    spin_unlock(&proc_lock);

    return ret;
}

// proc_write 函數，用於寫入 proc 檔案的內容
static ssize_t proc_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *offset) {
    char input_buffer[BUFFER_SIZE];
    struct timespec64 start_time, end_time;
    long elapsed_ms;
    pid_t pid, tgid;
    int priority;
    char state[20];
    char info_buffer[512];
    int info_len;

    if (len > BUFFER_SIZE - 1) {
        printk(KERN_WARNING "mythread_info: Input too large\n");
        return -EINVAL;
    }

    if (copy_from_user(input_buffer, user_buffer, len)) {
        return -EFAULT;
    }
    input_buffer[len] = '\0';

    // 獲取寫入開始時間
    ktime_get_real_ts64(&start_time);

    // 取得當前線程的資訊
    pid = current->pid;
    tgid = current->tgid;
    priority = current->prio;

    switch (current->__state) {
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

    // 獲取寫入結束時間
    ktime_get_real_ts64(&end_time);

    // 計算執行時間（毫秒）
    elapsed_ms = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                 (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

    // 格式化寫入的資訊
    info_len = snprintf(info_buffer, sizeof(info_buffer),
                        "Thread Write: %s\nProcess ID (PID): %d\nThread ID (TID): %d\nPriority: %d\nElapsed Time: %ld ms\n\n",
                        input_buffer, tgid, pid, priority, elapsed_ms);

    // 將資訊追加到 proc_buffer 中
    spin_lock(&proc_lock);
    if (proc_buffer_size + info_len >= BUFFER_SIZE) {
        printk(KERN_WARNING "mythread_info: Buffer overflow on write\n");
        spin_unlock(&proc_lock);
        return -ENOMEM;
    }

    memcpy(proc_buffer + proc_buffer_size, info_buffer, info_len);
    proc_buffer_size += info_len;
    spin_unlock(&proc_lock);

    return len;
}

// 定義 proc 檔案的操作結構
static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

// 模組初始化函數，建立 proc 檔案
static int __init my_module_init(void) {
    spin_lock_init(&proc_lock);
    memset(proc_buffer, 0, BUFFER_SIZE);
    proc_buffer_size = 0;

    entry = proc_create(PROC_NAME, 0666, NULL, &proc_ops);
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
MODULE_DESCRIPTION("A Linux kernel module to display and write thread info via /proc/mythread_info");
MODULE_VERSION("1.0");

module_init(my_module_init);
module_exit(my_module_exit);
