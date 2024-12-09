# OS Lab 3: 多線程矩陣乘法與使用 Proc 文件系統的核心模組

## 目錄
- [簡介](#簡介)
- [目標](#目標)
- [環境要求](#環境要求)
- [目錄結構](#目錄結構)
- [實作部分](#實作部分)
  - [2.2 雙線程矩陣乘法](#22-雙線程矩陣乘法)
  - [3.1 創建 `/proc/mythread_info`](#31-創建-proc-mythread_info)
  - [3.2 實作 Proc 檔案的讀取和寫入](#32-實作-proc-檔案的讀取與寫入)
- [編譯與執行](#編譯與執行)
  - [編譯核心模組與用戶空間程式](#編譯核心模組與用戶空間程式)
  - [載入與卸載核心模組](#載入與卸載核心模組)
  - [執行用戶程式](#執行用戶程式)
- [常見問題](#常見問題)
- [參考資料](#參考資料)

---

## 簡介

本實驗旨在透過多線程進行矩陣乘法運算，並學習如何使用 Linux 核心模組與 Proc 文件系統進行用戶空間與核心空間的通訊。實作內容包括雙線程矩陣乘法、創建並操作 `/proc/mythread_info` 檔案，以及實作該 Proc 檔案的讀取與寫入操作。

## 目標

1. **雙線程矩陣乘法**：
   - 使用雙線程完成矩陣乘法，處理競爭條件（race condition）。

2. **Linux 核心模組**：
   - 創建並操作 `/proc/mythread_info` 檔案。
   - 實作 Proc 檔案的讀取（`proc_read`）與寫入（`proc_write`）操作。
   - 支援單線程和雙線程對 Proc 檔案的寫入，並展示相關程序與線程資訊。

## 環境要求

- **操作系統**：建議使用實體 Linux 系統或虛擬機（如 VirtualBox、VMware）運行 Ubuntu、Fedora 等發行版。
- **開發工具**：
  - GCC 編譯器
  - Make 工具
  - Linux 內核標頭文件（與當前內核版本匹配）
- **權限**：需要管理員（sudo）權限進行模組的載入與卸載。

> **注意**：在 WSL2 環境下，默認情況下缺少內核標頭並且無法載入自訂核心模組，建議使用虛擬機或實體 Linux 系統進行實驗。

## 目錄結構

bash
OS_Lab3/
├── 2/
│ ├── 2_2.c
│ └── Makefile
├── 3/
│ ├── 3_1/
│ │ ├── 3_1.c
│ │ └── Makefile
│ ├── 3_2/
│ │ ├── 3_2.c
│ │ ├── My_Kernel.c
│ │ ├── 3_2_Config.h
│ │ └── Makefile
└── README.md


## 實作部分

### 2.2 雙線程矩陣乘法

- **檔案**：`2/2_2.c`
- **說明**：使用雙線程完成矩陣乘法，將矩陣 `A` 和 `B` 沿 K 方向切分為 `A1`、`A2` 和 `B1`、`B2`，分別由兩個線程計算，最終合併結果。

### 3.1 創建 `/proc/mythread_info`

- **檔案**：`3/3_1/My_Kernel.c`
- **說明**：創建一個 `/proc` 檔案 `mythread_info`，用於顯示當前程序和線程的資訊，包括 PID、TID、優先級和狀態。

### 3.2 實作 Proc 檔案的讀取和寫入

- **檔案**：
  - `3/3_2/My_Kernel.c`
  - `3/3_2/3_2.c`
  - `3/3_2/Makefile`
  - `3/3_2/3_2_Config.h`

- **說明**：
  - 在 `My_Kernel.c` 中實作 `proc_write` 和 `proc_read` 函數，支援單線程和雙線程的寫入操作，並展示程序 ID、線程 ID 和執行時間（毫秒）。
  - 在 `3_2.c` 中實作用戶空間的程式，進行單線程和雙線程的寫入操作。

## 編譯與執行

### 編譯核心模組與用戶空間程式

1. **進入 3.2 目錄**：
    ```bash
    cd 3/3_2/
    ```

2. **編譯核心模組與用戶程式**：
    ```bash
    make all
    ```
    - **說明**：此命令會編譯核心模組 `My_Kernel.ko` 及用戶程式 `3_2.out`。

### 載入與卸載核心模組

1. **載入核心模組**：
    ```bash
    make load
    ```
    - **說明**：使用 `insmod` 載入 `My_Kernel.ko` 模組，創建 `/proc/mythread_info` 檔案。

2. **卸載核心模組**：
    ```bash
    make unload
    ```
    - **說明**：使用 `rmmod` 卸載 `My_Kernel.ko` 模組，移除 `/proc/mythread_info` 檔案。

### 執行用戶程式

1. **單線程寫入**：
    ```bash
    make Prog_1thread
    ```
    - **說明**：
      - 將 `THREAD_NUMBER` 設定為 1，編譯並執行 `3_2.out` 程式，以單線程模式寫入字串到 `/proc/mythread_info`。
      - 執行完成後，清除相關檔案。

2. **雙線程寫入**：
    ```bash
    make Prog_2thread
    ```
    - **說明**：
      - 將 `THREAD_NUMBER` 設定為 2，編譯並執行 `3_2.out` 程式，以雙線程模式分別寫入字串到 `/proc/mythread_info`。
      - 執行完成後，清除相關檔案。

### 查看 `/proc/mythread_info` 的內容

bash
cat /proc/mythread_info


- **說明**：查看內核模組所寫入的相關資訊，包括每次寫入的字串、PID、TID、優先級，以及執行時間（毫秒）。

---

## 常見問題

### 無法找到 `/lib/modules/$(uname -r)/build` 目錄

- **問題原因**：缺少內核標頭文件。
- **解決方案**：
  - 確保已安裝與當前內核版本匹配的內核標頭。
  - 在 Ubuntu 上，可以使用以下命令安裝：
    ```bash
    sudo apt update
    sudo apt install build-essential linux-headers-$(uname -r)
    ```
  - 如果使用的是自訂內核，請確保內核標頭文件已正確安裝。

### 在 WSL2 環境下無法載入核心模組

- **原因**：WSL2 默認不支援載入自訂核心模組，且缺少內核標頭。
- **解決方案**：建議使用實體 Linux 系統或虛擬機來進行實驗，因為這些環境支持自訂核心模組的編譯與載入。

### 權限問題

- **問題**：載入和卸載核心模組需要管理員權限。
- **解決方案**：確保使用 `sudo` 執行相關命令，如 `make load`、`make unload`、`make Prog_1thread`、`make Prog_2thread`。

### 程式執行出現段錯誤（Segmentation Fault）

- **可能原因**：
  - 記憶體分配錯誤。
  - 超出陣列邊界。
  - 多線程同步問題。
- **解決方案**：
  - 使用除錯工具如 `gdb` 或 `valgrind` 追蹤錯誤來源。
  - 確認所有記憶體的正確分配與釋放。
  - 檢查並確保多線程操作中的同步機制正確。

---

## 參考資料

- [Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/)
- [The Linux Documentation Project](https://www.kernel.org/doc/html/latest/)
- [Proc Filesystem](https://www.kernel.org/doc/html/latest/filesystems/proc.html)
- [pthread Programming in C](https://www.gnu.org/software/libc/manual/html_node/Thread-Creation.html)

---

## 作者

- **Your Name**
- **聯絡方式**：your.email@example.com

---

## 授權

本專案依照 [MIT 授權條款](LICENSE) 進行授權。