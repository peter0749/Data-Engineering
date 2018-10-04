### 資料工程 HW

404410030 資工四 鄭光宇

### 系統需求

要執行這支程式，系統必須具備：

- 支援 ANSI C 的 gcc
- make 指令

### 如何編譯

在專案目錄下輸入指令：

```
make
```

### 如何執行

同作業要求

```
rsort 檔名 [參數]
```

![](https://i.imgur.com/r9LtuSh.png)

`-d` 是分隔符號，`-k` 是要作為 key 的 pattern

而 `-c` 是不區分大小寫、`-n` 使用數值排序、`-r` 倒序（降序）排序

### 實作部分

#### 前置處理

先合併檔

![](https://i.imgur.com/rrFMutd.png)

#### 主函數部份

讀取執行參數、讀檔、斷行、排序、輸出

![](https://i.imgur.com/cRPY2mn.png)

讀檔部份跟前一次作業類似，就不放上來了

#### 讀取參數部份

![](https://i.imgur.com/taeJSQo.png)

簡單解析從 main function 傳入的 argv 裏面的參數

`-d`, `-k` 後方需要接一個字串，其他3個參數則不需要輸入內容

#### 排序用的比較函數

![](https://i.imgur.com/ERKiCIu.png)

同作業要求，實作了依照 `-k` 為key值排序、依照 `-n` 決定是否為數值排序、依照 `-c` 決定是否區分大小寫、依照 `-r` 決定是否降序排序。

### 實驗

使用上次作業的資料集的合併檔

#### 照文章標題排序

![](https://i.imgur.com/s9aM8V4.png)
![](https://i.imgur.com/0rnqdKB.png)

花費時間約為18秒

#### 照URL中的編號排序

升序

![](https://i.imgur.com/2Bp8TNm.png)

降序

![](https://i.imgur.com/1CXPECJ.png)

#### 其他實驗

數字排序

![](https://i.imgur.com/KaNyhQL.png)

大小寫字母排序

![](https://i.imgur.com/A1Pcq0X.png)

### 總結

程式行為符合預期，如果是針對數字就必須使用數值排序。

不然預設就是字典序，這點可以從 `3333 44 444 55` 和 `7 44 55 111 ... 3333` 兩種排序看出來。 `-n` 這個選項是方便的功能。


### GitHub

[https://github.com/peter0749/Data-Engineering/tree/master/HW1](https://github.com/peter0749/Data-Engineering/tree/master/HW1)

