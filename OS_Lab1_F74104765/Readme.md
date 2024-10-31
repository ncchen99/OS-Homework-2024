# Sender & Receiver Communication Program

## 目錄
- [Sender \& Receiver Communication Program](#sender--receiver-communication-program)
  - [目錄](#目錄)
  - [專案簡介](#專案簡介)
  - [程式架構](#程式架構)
    - [相關文件：](#相關文件)
  - [通訊方式](#通訊方式)
    - [Message Passing](#message-passing)
    - [Shared Memory](#shared-memory)
  - [程式實作](#程式實作)
    - [Sender.c](#senderc)
      - [實現細節：](#實現細節)
    - [Receiver.c](#receiverc)
      - [實現細節：](#實現細節-1)
  - [如何執行](#如何執行)
    - [編譯與運行指令](#編譯與運行指令)
      - [運行方式](#運行方式)
  - [輸出範例](#輸出範例)
    - [Sender](#sender)
    - [Receiver](#receiver)
  - [注意事項](#注意事項)

---

## 專案簡介

這個專案實作了一個簡單的訊息傳遞機制，讓 `sender` 和 `receiver` 兩個進程可以透過兩種不同的方式進行通訊：
1. **訊息隊列 (Message Passing)**：利用 System V 的 `msgsnd()` 和 `msgrcv()` API 進行進程間的訊息交換。
2. **共享記憶體 (Shared Memory)**：使用共享記憶體和信號燈進行訊息交換。

## 程式架構

本專案包含兩個主要程式文件：
1. **Sender**：負責讀取訊息並將其發送給 Receiver。
2. **Receiver**：負責接收來自 Sender 的訊息。

### 相關文件：
- `sender.h`：定義了通用的數據結構和函數聲明。
- `receiver.h`：定義了通用的數據結構和函數聲明。

## 通訊方式

本專案提供兩種通訊方式：

### Message Passing

透過 System V 的訊息隊列實現，`sender` 將訊息加入訊息隊列，`receiver` 從訊息隊列中提取訊息。

**步驟**：
1. `sender` 呼叫 `msgsnd()` 將訊息發送到訊息隊列。
2. `receiver` 呼叫 `msgrcv()` 從訊息隊列中接收訊息。

### Shared Memory

透過共享記憶體和信號燈實現，`sender` 將訊息寫入共享記憶體，`receiver` 從共享記憶體中讀取訊息，並使用信號燈進行同步。

**步驟**：
1. `sender` 將訊息寫入共享記憶體後通知 `receiver`。
2. `receiver` 讀取共享記憶體中的訊息並回應 `sender`。

## 程式實作

### Sender.c

`Sender.c` 負責根據選擇的通訊方式，將讀取自文件的訊息發送給 `receiver`。在程式中，你可以選擇兩種不同的通訊機制：**Message Passing** 或 **Shared Memory**。

#### 實現細節：
1. 使用 `msgsnd()` 發送訊息到訊息隊列 (Message Passing)。
2. 使用共享記憶體寫入訊息並透過信號燈同步 (Shared Memory)。

### Receiver.c

`Receiver.c` 負責從訊息隊列或共享記憶體中接收訊息，並在接收到特殊的結束訊息（EOF）後退出。

#### 實現細節：
1. 使用 `msgrcv()` 從訊息隊列中讀取訊息 (Message Passing)。
2. 使用共享記憶體讀取訊息並透過信號燈同步 (Shared Memory)。

## 如何執行

### 編譯與運行指令

首先，你需要編譯程式：
```bash
make clean && make all
```

#### 運行方式
- 以 **Message Passing** 方式運行：
  - Sender:
    ```bash
    ./sender 1 input.txt
    ```
  - Receiver:
    ```bash
    ./receiver 1
    ```
  
- 以 **Shared Memory** 方式運行：
  - Sender:
    ```bash
    ./sender 2 input.txt
    ```
  - Receiver:
    ```bash
    ./receiver 2
    ```

## 輸出範例

以下是程式運行時的輸出格式範例：

### Sender

```bash
Message Passing
Sending message: first message
Sending message: second message
Sending message: third message
...
End of input file! exit!
Total time taken in sending msg: 0.000053 s
```

### Receiver

```bash
Message Passing
Receiving message: first message
Receiving message: second message
Receiving message: third message
...
Sender exit!
Total time taken in receiving msg: 0.000041 s
```

## 注意事項

- 確保在運行程序之前，已正確建立信號燈和共享記憶體，並且 `ftok()` 用於生成唯一的鍵值。
- 本專案的訊息傳遞過程會根據所選擇的通訊方式（`Message Passing` 或 `Shared Memory`）自動選擇對應的傳輸方式。
- 程式中的時間計算不包括進程之間的等待時間，僅記錄訊息傳遞的時間。

---

這樣你就完成了一個簡單的通訊機制，並能夠根據需要選擇通訊方式。