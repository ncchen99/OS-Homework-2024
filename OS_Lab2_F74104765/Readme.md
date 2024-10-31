## Shell 作業說明文件

### 目錄結構

```plaintext
.
├── include              # 標頭文件所在目錄
│   ├── builtin.h        # 定義內建命令函式
│   ├── command.h        # 定義指令解析函式
│   └── shell.h          # 定義 shell 主流程函式
├── src                  # 原始碼所在目錄
│   ├── builtin.c        # 內建命令的實作
│   ├── command.c        # 指令解析與資料結構管理
│   └── shell.c          # 主 shell 迴圈與流程控制
├── demo.txt             # 測試文件
├── my_shell.c           # 主程式入口
├── makefile             # 編譯和清理命令
└── README.md            # 說明文件
```

---

### Makefile 說明

此 Makefile 包含編譯和清理指令，提供 `all`、`clean` 和 `clean_obj` 三個主要目標：

- `all`：編譯所有原始碼並生成執行檔 `my_shell`。
- `clean`：移除編譯生成的執行檔和 `.o` 物件檔。
- `clean_obj`：僅移除 `.o` 物件檔。

**指令使用：**
```bash
make        # 編譯程式
make clean  # 清理執行檔和物件檔
make clean_obj # 只清理物件檔
```

---

### 程式架構

1. **主程式入口 (`my_shell.c`)**：負責初始化 shell，並調用主 shell 函式 `shell()`。
2. **`shell.c`**：核心執行流程。包含 `shell()` 函式，負責顯示提示符、解析指令、執行內建命令或外部指令，並處理管道與重定向等功能。
3. **`builtin.c`**：內建指令的實作，如 `cd`、`pwd`、`echo` 等命令，並定義 `searchBuiltInCommand` 函式來確認指令是否為內建。
4. **`command.c`**：包含指令解析邏輯，將輸入的字串解析成結構化的指令節點，並管理各節點的管道和重定向資訊。

---

### 功能說明

- **內建命令 (`builtin.c`)**：支援常見的內建指令，包含：
  - `cd [directory]`：變更當前工作目錄。
  - `pwd`：列出當前工作目錄。
  - `echo`：輸出內容。
  - `exit`：退出 shell。

- **指令解析 (`command.c`)**：解析使用者輸入並構建 `cmd` 結構，包含多個 `cmd_node` 節點，用於管理每個指令及其參數、重定向和管道資訊。

- **重定向與管道 (`shell.c`)**：
  - `redirection()`：根據 `cmd_node` 中的 `in_file` 和 `out_file` 欄位設置標準輸入輸出。
  - `fork_cmd_node()`：執行多個連接的指令，利用 `pipe()` 創建子進程間的通道，並管理每個進程的輸入輸出。

---

### 執行流程

1. **啟動 Shell**：執行 `./my_shell` 進入 shell 互動模式，會顯示提示符 `<username>@<hostname>:<current directory>$ `。
2. **輸入指令**：可以輸入內建指令或外部指令，並支援管道 (`|`)、輸入 (`<`) 和輸出 (`>`) 重定向。
3. **指令解析與執行**：
   - Shell 會呼叫 `split_line()` 將指令解析為 `cmd` 結構。
   - 根據指令數量執行對應的函式：單指令時調用 `spawn_proc()`，多指令則調用 `fork_cmd_node()`。
   - 進行重定向設定（如有）。
4. **結束執行**：若輸入 `exit`，則退出 shell。

---

### 使用範例

```shell
oslab@PC:~/my_shell$ pwd
/home/oslab/my_shell
oslab@PC:~/my_shell$ cd /path/to/directory
oslab@PC:/path/to/directory$ cat demo.txt | grep "example" > result.txt
oslab@PC:/path/to/directory$ exit
```

在這裡，shell 將輸入的指令解析後，執行了 `cd` 來更換目錄、`cat` 和 `grep` 指令進行管道操作，並將結果輸出至 `result.txt` 文件。最後通過 `exit` 指令退出了 shell。

---

### 注意事項

- **錯誤處理**：對於錯誤的指令或無效的文件路徑，shell 會輸出錯誤訊息，避免程式崩潰。
- **記憶體管理**：每次指令執行完後，釋放指令結構的記憶體以避免記憶體洩漏。
- **管道與重定向**：最後一個指令的輸出若需重定向至文件，將由 `fork_cmd_node()` 中的 `redirection()` 處理。

---

### 結語

這個 shell 提供了基礎的命令解析、管道與重定向處理，並模擬了 UNIX shell 的基本功能。該架構讓使用者可以擴展其他內建指令和功能，適合用於學習 shell 和作業系統的程式設計基礎。
