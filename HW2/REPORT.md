# 資料工程 HW1

404410030 資工四 鄭光宇

# 系統需求

要執行這支程式，系統必須具備：

 - 支援 ANSI C 的 gcc
 - make 工具程式

# 編譯

```
make
```

# 執行

```
./rsort filename [-d delimeter | -k field | -m memory_limit | -f output_filename (o.w. stdout) | -n (set numeric comparison) | -r (set reverse sort) | -c  (set case insensitive) | -s (set size_sort) ]
```

延續上次作業要求，另外新增了 `-s` 照資料大小排序。

`-d` 是分隔符號，`-k` 是要作為 key 的 pattern

而 `-c` 是不區分大小寫、`-n` 使用數值排序、`-r` 倒序（降序）排序。

最重要的，`-m` 限制大約使用多少 MB 的記憶體做內部排序。

# 資料

先將助教提供的檔案，合成一個大的 `full_data.rec` 檔

檔案大小 17GB 左右

## 觀察

![](https://i.imgur.com/mL6bMDa.png)

資料似乎是以 `_\n@` 的方式分隔（`_`為空格）

所以之後都使用這個分隔方式處理資料

## 實作

下面列出一些主要的程式碼

### 讀參數

跟上一次內部排序用一樣的程式碼

![](https://i.imgur.com/oCsJLJF.png)

### 資料間的比較函式

![](https://i.imgur.com/qv6GUxs.png)

針對各種參數有不同的比較方式

### 檔案寫出

![](https://i.imgur.com/o354Hyu.png)

### 讀檔/排序/分割檔/寫出

逐漸讀入檔案，每到了指定的記憶體容量限制，就寫出一個 chunk 到硬碟中、清除記憶體、記錄增加的 chunck 數並記錄他們寫入的路徑。這個函數可以指定要讀多少筆資料，可以協助後面 external sort 的 merge 階段。

![](https://i.imgur.com/wK5EYWq.png)

![](https://i.imgur.com/j0XUSmU.png)

![](https://i.imgur.com/K0rO8Uv.png)

![](https://i.imgur.com/tFvF0Ph.png)

### K-way merge

每個 queue 為上個階段已排序的 chunk，我們利用 Winner-Tree 維護所有 queue 中最小的值。並且在每次查詢 $O(1)$ ，每次更新 $O(K)$ 的複雜度下，不斷從 Winner-Tree 頂端 pop 出最小值，這些 pop 出來的資料會循序寫入到檔案，這樣就可以得到已經排序的文件。

Winner-Tree 的初始化方式（使用 array 實作）：

1. 初始化所有點為無限大
2. 將所有 queue 的第一個元素放在 Winner-Tree 的 leaf 上（array 尾端）
3. 由於 Winner-Tree 是 (nearly) Complete Binary Tree，從 array 最尾端往開頭走訪，過程中每次檢查如果自己的數值比 parent 小，就將 parent 更新。

Winner-True 的更新方式簡單來說：

1. 從頂端 pop 出 key 最小值
2. 從最小值對應回的那個 queue 再讀一筆資料。若資料為空，讀入資料的節點大小為無限大。
3. 走訪到自己的 parent，從 parent 看兩個 child 哪個小，哪個提上去到 parent 的位置。重複步驟直到走到 root ，一次更新的操作就完成了。

不斷重複 pop 最小值，直到所有 queue 為空（此時 Winner-Tree 頂端也為空）。完成外部排序。

![](https://i.imgur.com/O48WOuB.png)

![](https://i.imgur.com/rR0Clvb.png)

![](https://i.imgur.com/wLRVwSA.png)

### 主函式部分

![](https://i.imgur.com/qe78C4b.png)

# 實驗

## 照資料大小排序（`-s` 模式）


![](https://i.imgur.com/BiydUrO.png)


![](https://i.imgur.com/nm1JueC.png)

![](https://i.imgur.com/pJt7t6K.png)

可以看出最大的一筆資料因為沒有恰當的 `_\n@` 分隔

排序大約花費 7 分鐘左右

## 字典序（預設模式）

字典序排出來看起來很亂，這裡指附上執行時間供參考

![](https://i.imgur.com/m7aSVLG.png)

## 時間序（照發佈時間的字串排序）

![](https://i.imgur.com/DdyCg8i.png)

![](https://i.imgur.com/s97j8mB.png)

![](https://i.imgur.com/PfI6qwV.png)

可以看出成功照日期排序，`2017年8月9日`排在一起了

排序大約 7min

## 驗證程式碼正確性

生成 `1000000` 個隨機整數（約 5.7MB）

```
for ((i=0; i<1000000; ++i)); do echo $RANDOM >> random_numbers; done 
```

使用 `sort` 排序，作為標準答案

```
sort random_numbers -n > gt
```

使用 `rsort` 進行排序（用很小的 buffer size，為了驗證 k-way merge 能正常工作）
```
./rsort random_numbers -n -m 1 -f pred
```

比較兩者結果，沒報錯答案就是一樣

```
diff gt pred -q
```

![](https://i.imgur.com/mDEealr.png)


### GitHub

程式碼：[rsort.c](https://github.com/peter0749/Data-Engineering/blob/master/HW2/rsort.c)