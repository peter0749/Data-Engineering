### 資料工程 HW0

404410030 資工四 鄭光宇

### 系統需求

要執行這支程式，系統必須具備：

- Apache2.0、PHP5.0 以上
- 支援 C11 的 gcc
- make 指令

### 如何編譯

在專案目錄下輸入指令：

```
make
```

### 如何執行

1. 在專案同目錄下新增 `data` 目錄
2. 將 ettoday0.rec ~ ettoday5.rec 放在專案同目錄下的 `data` 目錄
3. 執行 `make` 編譯程式，編譯出 `parser` 程式
4. 執行 `./parser` 程式，程式輸出斷句、排序後的結果到 `dataset.txt`
5. 將 `dataset.txt` 和專題目錄下的 `index.php` 放到你的個人網頁資料夾 or `/var/www/html`
6. 在瀏覽器網址輸入 `http://localhost/你的個人網站/index.php` or `http://localhost/index.php` 進入網頁
7. 搜尋句子

### 實作部分

#### 前置處理

依照作業規定，使用「。？！」等字元進行斷句。僅採用中文字開頭的句子，並且句子中的中文字必須有五個中文字以上（不包含五）。中文字的部分 Uncode 編碼我採用 `0x4E00` ~ `0x9FFF` 這個範圍。之後定義一個 `struct`，方便後面處理資料。

![](https://i.imgur.com/pTgf3jK.png)

定義判斷中文字範圍的工具函式。

![](https://i.imgur.com/Y5JitJu.png)

斷詞部分。實作一個 dynamic array 去儲存斷好的句子。

![](https://i.imgur.com/UoRKsh0.png)

![](https://i.imgur.com/NBjfs5g.png)

工具函式，做一些空格、tab 等特殊字元的處理。

![](https://i.imgur.com/vPFacTc.png)

parser 部分。先找到 `@GAISRec` 行，之後連續讀取四行，得到標題、URL、內文等資料。對每個文章內文做完斷句後，再將該句子、來源標題與 URL，以 tab 分隔模式寫在同一行。

![](https://i.imgur.com/UFIG4Gp.png)

![](https://i.imgur.com/CjtAU9G.png)

取得所有斷句後，照句子字典序對句子排序。最後寫入 `dataset.txt` 檔案。

![](https://i.imgur.com/AcrJeuy.png)

![](https://i.imgur.com/ASA6a3v.png)

使用 `wcscmp` 比較句子 key 值大小。

![](https://i.imgur.com/KweFFhD.png)


主函式部分。

![](https://i.imgur.com/aVlqbfN.png)

#### 網頁介面部分

使用表單 + PHP，讓使用者查詢指定開頭的句子。後台呼叫 `grep` 找出 `dataset.txt` 中，符合的句子。並將結果寫入暫存檔。若使用者在同一個 session 並且查詢的內容是相同的，就直接讀取暫存檔。

![](https://i.imgur.com/MYc1gY5.png)

![](https://i.imgur.com/43UZSA0.png)

![](https://i.imgur.com/Zmlm0vm.png)


根據使用者所在 page，印出相應的 50 行結果。每一行都附帶原標題與連結在左側，而相關內文在右側。

![](https://i.imgur.com/LkYW1vJ.png)

最後在網頁尾端印出到每個 page 的按鈕，提供使用者選擇。

![](https://i.imgur.com/wC2WXo0.png)

### 計算前置處理花費時間

計算前置處理花費時間：

![](https://i.imgur.com/01VvPFS.png)

從讀擋到排序輸出大約 18 秒左右。

### GitHub

[https://github.com/peter0749/Data-Engineering/tree/master/HW0](https://github.com/peter0749/Data-Engineering/tree/master/HW0)

### Demo (Youtube)

[https://www.youtube.com/watch?v=\_be3uvhgNlw](https://www.youtube.com/watch?v=_be3uvhgNlw)
