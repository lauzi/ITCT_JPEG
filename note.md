// SEA-N 版

jpeg (header) 內的數值都是 big-endian，bmp 內的數值是 little-endian

作業的四個 sample jpg 都沒有 DRI, RSTm marker，而且我測一堆圖都沒有
不做說不定沒差(?)

解 Huffman code 時如果讀到 0xff (對齊的 byte 才算) 要再讀下一個 byte
看是不是 0x00，如果是的話這個 0xff 就是 codeword 的一部份，然後忽略
掉 0x00；不然就是 Marker (雖然 sample jpg 裡沒有)

AC 係數如果最後一個不是 0 的話，就不會有 EOB。所以除了讀到 EOB 要 break，讀滿 64 格時也要

Huffman decode 最好查表，8bit 2-level 或直接 16bit 之類的，一個一個 bit 解可能會有點慢(?)

有碰過最後一個 DataUnit 不是用 EOB 結束，是直接 EOI 結束的 (sample jpg 沒有)


又是一個沒講過的卡關點一枚 ...


... 我終於理解謎之鋸齒感從哪來的了

當 component 大小不一樣，比方說 4:2:2 時，

要做的事情不是直接放大兩倍就好，而是要內插

... 雖然這樣很合理啦


JPEG標準裡不含這些，像是 YUV, RGB 的轉換

看起來應該是在 JFIF 下，包括內插補點的計算方式等

http://www.w3.org/Graphics/JPEG/
http://www.w3.org/Graphics/JPEG/jfif3.pdf

--
※ 發信站: 批踢踢兔(ptt2.cc)
◆ From: 140.112.250.122
→ seanwu:不兩張圖仔細比的話根本看不出來就是了                  推 05/24 03:21
→ seanwu:發現投影片的圖是wiki來的.... 不過wiki底下多了一句     推 05/24 03:28
→ seanwu:Also note that the diagram does not indicate any      推 05/24 03:28
→ seanwu:chroma filtering, which should be applied to avoid    推 05/24 03:29
→ seanwu:aliasing ....                                         推 05/24 03:29
→ seanwu:改好了XD 我覺得我太龜毛不然助教九成九不會注意到這個   推 05/24 03:35
