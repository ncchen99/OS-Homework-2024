# osfs - 簡易記憶體基礎檔案系統

## 介紹

**osfs**（Operating System File System）是一個簡單的記憶體基礎虛擬檔案系統（VFS）實作，旨在展示如何在 Linux 核心中建立自訂檔案系統。VFS 作為作業系統中的一個抽象層，允許多個檔案系統共存，使用者應用程式無需了解不同檔案系統的實作細節。

## 技術背景

### 虛擬檔案系統 (VFS)

VFS 是作業系統中的一個抽象層，其主要目的是允許多個檔案系統共存，同時使用者應用程式不需要了解不同檔案系統的實作細節。使用者空間的應用程式只需透過作業系統提供的通用介面（即 VFS）來執行操作，例如打開檔案或存取資料。這些請求會被傳遞給 VFS，VFS 會負責挑選合適的底層檔案系統來執行這些操作。

### osfs 的佈局

osfs 的佈局直接影響檔案系統的設計和功能。作業中會分配一塊記憶體來模擬硬碟，並在記憶體空間中劃分出以下結構：

- **Superblock**
- **Inode bitmap**
- **Data bitmap**
- **Inode table**
- **Data blocks**

其中，Inode bitmap 和 Data bitmap 使用位元向量的方式來進行可用空間的管理。

#### osfs_sb_info

- **Magic number**：用來確認檔案系統的類型。
- **Block size**：每個 data block 的大小。
- **Inode count 和 block count**：Inode 和 data block 的數量。
- **指向 Inode bitmap、Data bitmap、Inode table 和 Data blocks 的指標**。

#### osfs_inode

- **Inode number**
- **File size**
- **Blocks**：檔案佔用的 data block 數量。
- **檔案的擁有者**。
- **Time stamp**：`atime`、`ctime` 和 `mtime`，分別代表存取時間、修改時間和建立時間。
- **i_block**：data block pointer，指向實際存放資料的位置。

### 檔案系統中的存取路徑

假設要讀取 `/os/lab4` 中的資料，檔案系統內的流程如下：

1. 讀取根目錄的 inode，並找到其 data block 指標。
2. 讀取根目錄的 data block，並找到 `os` 的 entry。
3. 讀取 `os` 的 inode，並找到其 data block 指標。
4. 讀取 `os` 的 data block，並找到 `lab4` 的 entry。
5. 將 `lab4` 的 inode 讀入記憶體。
6. 讀取 `lab4` 的 inode，並找到其 data block 指標。
7. 讀取 `lab4` 的 data block。
8. 更新 `lab4` 的 inode（例如，最後存取時間）。

## 作業要求和評分方式

本作業要求完成兩個主要函式：

1. **osfs_create**：建立新檔案時所需的函式（7%）。
2. **osfs_write**：將資料寫入檔案的函式（3%）。

### osfs_create

#### 關鍵函式

```c:file.c
static int osfs_create(struct mnt_idmap *idmap, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
```

#### 步驟

1. 解析 VFS 傳入的父目錄，並取得父目錄和相關資訊。
2. 驗證檔案名稱長度是否符合規定。
3. 分配並初始化一個 VFS inode 和 osfs inode。
4. 在父目錄中新增一個目錄 entry。
5. 更新父目錄的 metadata。
6. 將 inode 與 VFS dentry 綁定。

### osfs_write

#### 關鍵函式

```c:file.c
static ssize_t osfs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
```

#### 步驟

1. 取得 inode 和檔案系統資訊。
2. 檢查 inode 的 data block 是否已分配，如果沒有，就分配一個 data block。
3. 檢查寫入資料的大小，並限制寫入長度以符合一個 data block 的大小。
4. 將使用者輸入的資料寫入 data block。
5. 更新 inode 和 osfs_inode 的屬性，例如大小和時間戳記。
6. 回傳寫入的位元組數。

#### 相關資料結構

- **osfs_inode**
  - `i_block`
  - `i_blocks`
  - `i_size`

- **osfs_inode** 中的相關函式：
  - `osfs_alloc_data_block`
  - `copy_from_user`

## 各檔案說明

### `file.c`

管理檔案的讀寫操作，包括 `osfs_read` 和 `osfs_write` 函式。

```c:file.c
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "osfs.h"

/* osfs_read 和 osfs_write 函式 */

const struct file_operations osfs_file_operations = {
    .open = generic_file_open,
    .read = osfs_read,
    .write = osfs_write,
    .llseek = default_llseek,
};

const struct inode_operations osfs_file_inode_operations = {
    /* 可以在此添加其他 inode 操作，如 getattr */
};
```

### `inode.c`

管理 inode 的分配和回收，包括 `osfs_iget` 和 `osfs_new_inode` 函式。

```c:inode.c
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "osfs.h"

/* osfs_iget、osfs_new_inode 等函式 */

const struct inode_operations osfs_dir_inode_operations = {
    .lookup = osfs_lookup,
    .create = osfs_create,
    /* 添加其他操作 */
};
```

### `osfs_init.c`

初始化和退出 osfs 模組，包括掛載和卸載檔案系統的函式。

```c:osfs_init.c
#include <linux/init.h>
#include <linux/module.h>
#include "osfs.h"

/* osfs_mount、osfs_kill_superblock 函式 */

struct file_system_type osfs_type = {
    .owner = THIS_MODULE,
    .name = "osfs",
    .mount = osfs_mount,
    .kill_sb = osfs_kill_superblock,
    .fs_flags = FS_USERNS_MOUNT,
};

static int __init osfs_init(void)
{
    /* 註冊檔案系統 */
}

static void __exit osfs_exit(void)
{
    /* 反註冊檔案系統 */
}

module_init(osfs_init);
module_exit(osfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OSLAB");
MODULE_DESCRIPTION("A simple memory-based file system kernel module");
```

### `super.c`

管理 superblock 的操作，包括填充 superblock 和銷毀 inode。

```c:super.c
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include "osfs.h"

/* osfs_super_ops、osfs_fill_super、osfs_destroy_inode 函式 */

const struct super_operations osfs_super_ops = {
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,
    .destroy_inode = osfs_destroy_inode,
};

int osfs_fill_super(struct super_block *sb, void *data, int silent)
{
    /* 填充 superblock 的詳細步驟 */
}
```

### `osfs.h`

定義 osfs 檔案系統所需的結構和常數。

```osfs.h
#ifndef _OSFS_H
#define _OSFS_H

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/bitmap.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/module.h>

/* 常數定義 */

#define OSFS_MAGIC 0x051AB520
#define INODE_COUNT 20
#define DATA_BLOCK_COUNT 20
#define MAX_FILENAME_LEN 255
#define MAX_DIR_ENTRIES (BLOCK_SIZE / sizeof(struct osfs_dir_entry))
#define BITMAP_SIZE(bits) (((bits) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define INODE_BITMAP_SIZE BITMAP_SIZE(INODE_COUNT)
#define BLOCK_BITMAP_SIZE BITMAP_SIZE(DATA_BLOCK_COUNT)
#define ROOT_INODE 1

/* 結構定義 */

struct osfs_sb_info {
    uint32_t magic;
    uint32_t block_size;
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t nr_free_inodes;
    uint32_t nr_free_blocks;
    unsigned long *inode_bitmap;
    unsigned long *block_bitmap;
    void *inode_table;
    void *data_blocks;
};

struct osfs_dir_entry {
    char filename[MAX_FILENAME_LEN];
    uint32_t inode_no;
};

struct osfs_inode {
    uint32_t i_ino;
    uint32_t i_size;
    uint32_t i_blocks;
    uint16_t i_mode;
    uint16_t i_links_count;
    uint32_t i_uid;
    uint32_t i_gid;
    struct timespec64 __i_atime;
    struct timespec64 __i_mtime;
    struct timespec64 __i_ctime;
    uint32_t i_block;
};

/* 函式聲明 */

struct inode *osfs_iget(struct super_block *sb, unsigned long ino);
struct osfs_inode *osfs_get_osfs_inode(struct super_block *sb, uint32_t ino);
int osfs_get_free_inode(struct osfs_sb_info *sb_info);
int osfs_alloc_data_block(struct osfs_sb_info *sb_info, uint32_t *block_no);
int osfs_fill_super(struct super_block *sb, void *data, int silent);
struct inode *osfs_new_inode(const struct inode *dir, umode_t mode);
void osfs_free_data_block(struct osfs_sb_info *sb_info, uint32_t block_no);
void osfs_destroy_inode(struct inode *inode);

/* 外部操作結構 */

extern const struct inode_operations osfs_file_inode_operations;
extern const struct file_operations osfs_file_operations;
extern const struct inode_operations osfs_dir_inode_operations;
extern const struct file_operations osfs_dir_operations;
extern const struct super_operations osfs_super_ops;

#endif /* _osfs_H */
```

### `Makefile`

用於編譯 osfs 核心模組。

```Makefile
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += osfs.o
osfs-objs := super.o inode.o file.o dir.o osfs_init.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
```

## 操作說明

### 編譯核心模組

在 osfs 專案目錄下，執行以下命令來編譯核心模組：

```bash
make
```

編譯完成後，應會生成 `osfs.ko` 模組檔案。

### 插入模組

使用 `insmod` 命令將 osfs 模組插入 Linux 核心：

```bash
sudo insmod osfs.ko
```

插入後，使用 `dmesg` 命令檢查模組是否已正確插入：

```bash
dmesg | tail
```

應顯示類似以下訊息：

```
osfs: Successfully registered
```

### 掛載資料夾

建立一個目錄作為掛載點，例如 `mnt`：

```bash
mkdir mnt
```

使用 `mount` 命令掛載 osfs 檔案系統：

```bash
sudo mount -t osfs none mnt/
```

再次使用 `dmesg` 檢查掛載操作是否成功：

```bash
dmesg | tail
```

應顯示類似以下訊息：

```
osfs: Superblock filled successfully
```

### 測試檔案系統

1. **建立檔案**

   使用 `touch` 命令來建立一個新的檔案：

   ```bash
   sudo touch mnt/test1.txt
   ```

2. **寫入資料**

   使用 `echo` 命令將資料寫入檔案：

   ```bash
   sudo bash -c "echo 'I LOVE OSLAB' > mnt/test1.txt"
   ```

3. **檢查檔案內容**

   使用 `cat` 命令檢查檔案內容：

   ```bash
   cat mnt/test1.txt
   ```

   應顯示：

   ```
   I LOVE OSLAB
   ```

### 卸載資料夾

若掛載點已經被使用，可能會無法直接卸載。以下是卸載掛載點的方法：

1. **基本卸載**

   使用 `umount` 命令指定掛載點的路徑：

   ```bash
   sudo umount mnt/
   ```

2. **檢查掛載點是否被使用**

   如果卸載失敗，使用 `lsof` 或 `fuser` 檢查哪些進程正在使用掛載點：

   ```bash
   sudo lsof +f -- mnt/
   ```
   
   或

   ```bash
   sudo fuser -m mnt/
   ```

3. **強制卸載**

   若確定掛載點不再被使用，可以使用強制卸載：

   ```bash
   sudo umount -f mnt/
   ```

   或懸掛卸載：

   ```bash
   sudo umount -l mnt/
   ```

### 退出模組

卸載模組前，確保所有掛載的檔案系統已被卸載。然後使用 `rmmod` 或 `modprobe` 來卸載模組：

```bash
sudo rmmod osfs
```

或

```bash
sudo modprobe -r osfs
```

確認模組已成功卸載，使用 `dmesg` 檢查訊息：

```bash
dmesg | tail
```

應顯示類似以下訊息：

```
osfs: Successfully unregistered
```

### 完整操作範例

```bash
# 編譯模組
make

# 插入模組
sudo insmod osfs.ko

# 檢查模組是否已插入
dmesg | tail

# 建立掛載點
mkdir mnt

# 掛載檔案系統
sudo mount -t osfs none mnt/

# 檢查掛載是否成功
dmesg | tail

# 建立檔案
sudo touch mnt/test1.txt

# 寫入資料
sudo bash -c "echo 'I LOVE OSLAB' > mnt/test1.txt"

# 檢查檔案內容
cat mnt/test1.txt

# 卸載檔案系統
sudo umount mnt/

# 卸載模組
sudo rmmod osfs
```

## 修改 `file.c` 和 `dir.c` 中的關鍵程式碼

### `file.c` 中的 `osfs_write` 函式

在 `file.c` 中，`osfs_write` 函式負責將資料寫入檔案。以下是關鍵程式碼的修改說明：

```c:file.c
static ssize_t osfs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
    // 步驟1：取得 inode 和檔案系統資訊
    struct inode *inode = file_inode(filp);
    struct osfs_inode *osfs_inode = inode->i_private;
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;
    void *data_block;
    ssize_t bytes_written = 0;
    int ret;

    // 步驟2：檢查 inode 的 data block 是否已分配，若未分配則分配一個
    if (osfs_inode->i_blocks == 0) {
        ret = osfs_alloc_data_block(sb_info, &osfs_inode->i_block);
        if (ret) {
            return ret;
        }
        osfs_inode->i_blocks = 1;
    }

    // 步驟3：限制寫入長度以符合一個 data block 的大小
    if (*ppos + len > sb_info->block_size) {
        len = sb_info->block_size - *ppos;
    }

    // 步驟4：將資料從使用者空間寫入 data block
    data_block = sb_info->data_blocks + osfs_inode->i_block * sb_info->block_size + *ppos;
    if (copy_from_user(data_block, buf, len)) {
        return -EFAULT;
    }

    // 步驟5：更新 inode 和 osfs_inode 的屬性
    *ppos += len;
    bytes_written = len;
    if (*ppos > osfs_inode->i_size) {
        osfs_inode->i_size = *ppos;
        inode->i_size = *ppos;
    }
    inode->__i_mtime = inode->__i_atime = current_time(inode);
    osfs_inode->__i_mtime = osfs_inode->__i_atime = osfs_inode->__i_ctime = current_time(inode);
    mark_inode_dirty(inode);

    // 步驟6：回傳寫入的位元組數
    return bytes_written;
}
```

### `dir.c` 中的 `osfs_create` 函式

在 `dir.c` 中，`osfs_create` 函式負責建立新檔案。以下是關鍵程式碼的修改說明：

```c:dir.c
static int osfs_create(struct mnt_idmap *idmap, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{   
    /* 步驟1：解析父目錄並取得相關資訊 */
    // struct osfs_sb_info *sb_info = dir->i_sb->s_fs_info;

    /* 步驟2：驗證檔案名稱長度 */
    if (dentry->d_name.len > MAX_FILENAME_LEN) {
        pr_err("osfs_create: Filename too long\n");
        return -ENAMETOOLONG;
    }

    /* 步驟3：分配並初始化 VFS inode 和 osfs inode */
    struct inode *inode = osfs_new_inode(dir, mode);
    if (IS_ERR(inode)) {
        return PTR_ERR(inode);
    }

    /* 步驟4：在父目錄中新增一個目錄 entry */
    int ret = osfs_add_dir_entry(dir, inode->i_ino, dentry->d_name.name, dentry->d_name.len);
    if (ret) {
        iput(inode);
        return ret;
    }

    /* 步驟5：更新父目錄的 metadata */
    dir->i_size += sizeof(struct osfs_dir_entry);
    mark_inode_dirty(dir);

    /* 步驟6：將 inode 與 VFS dentry 綁定 */
    d_instantiate(dentry, inode);
    if (!dentry->d_inode) {
        iput(inode);
        return -EIO;
    }

    pr_info("osfs_create: File '%.*s' created with inode %lu\n",
            (int)dentry->d_name.len, dentry->d_name.name, inode->i_ino);

    return 0;
}
```

## 測試

完成程式碼後，按照以下步驟測試檔案系統：

1. **建置核心模組**

   ```bash
   make
   ```

2. **檢查建置的檔案**

   使用 `ls` 確認 `osfs.ko` 模組是否已生成。

   ```bash
   ls
   ```

3. **插入模組**

   ```bash
   sudo insmod osfs.ko
   ```

4. **檢查模組插入狀況**

   ```bash
   sudo dmesg | tail
   ```

5. **建立掛載點**

   ```bash
   mkdir mnt
   ```

6. **掛載檔案系統**

   ```bash
   sudo mount -t osfs none mnt/
   ```

7. **檢查掛載操作**

   ```bash
   sudo dmesg | tail
   ```

8. **測試建立檔案**

   ```bash
   sudo touch mnt/test1.txt
   ```

9. **測試寫入資料**

   ```bash
   sudo bash -c "echo 'I LOVE OSLAB' > mnt/test1.txt"
   ```

10. **檢查檔案內容**

    ```bash
    cat mnt/test1.txt
    ```

    應顯示：

    ```
    I LOVE OSLAB
    ```

11. **卸載檔案系統**

    ```bash
    sudo umount mnt/
    ```

12. **退出模組**

    ```bash
    sudo rmmod osfs
    ```

13. **確認模組已卸載**

    使用 `dmesg` 檢查卸載訊息：

    ```bash
    sudo dmesg | tail
    ```

## 參考文獻

1. [Linux VFS Documentation](https://www.kernel.org/doc/html/latest/filesystems/vfs.html)
2. [Linux Kernel Module Programming Guide](https://www.tldp.org/LDP/lkmpg/2.6/html/index.html)
3. [Understanding the Linux Virtual File System](https://lwn.net/Articles/3656/)
4. [Kernel Programming Books and Resources](https://kernelnewbies.org/Books)
5. [Linux Device Drivers](https://lwn.net/Kernel/LDD3/)
6. [OSFS Design and Implementation](#)
7. [Filesystem Structure and Inodes](#)
8. [Assignment Requirements](#)
9. [Function Implementation Details](#)
10. [Data Structures and Function Modifications](#)
11. [Testing Procedures](#)
12. [Advanced Features](#)

*感謝您的閱讀！若有任何問題或需要進一步協助，歡迎隨時聯繫我們。*
