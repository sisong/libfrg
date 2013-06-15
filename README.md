libfrg
========
Version  1.0.1 
libfrg: C\C++ codec of FRG  

---
FRG: "Fast Read Graphics" or "Fast Resource Graphics"  

FRG is an image format(like PNG or JPG etc.),for saving time loading from disk and decoding to display; It supports 32-bit ARGB color image.  

_FRG是一种优化从磁盘加载和解码到显示需要的时间的图像文件格式,它支持32-bit ARGB颜色的图像。_  

  
---
```
version 1.0 performance test:
  macosx10.8.3 ,  CPU:i7 2.3G (single thread), memory: 8G DDR3 1600MHz  ,  7zip use LZMA2 , load image file from Disk 236MB/s;
  1160 bmp files: ARGB32bit color 863,990,576 bytes and 215,980,447 pixels;
  note: .jpg no alpha color

=====================================================================================================================================
                                       .jpg 100 .jpg 95  .jpg 90 .frg(100 quality)     .frg 90             .frg 80           .frg 75
                         .bmp    .png   quality quality  quality (0 size  50 size) (0 size 50 size) (0 size 25 size 50 size) (50 size)
image file size         100.00%  15.13%  15.32%   6.81%    4.72%   21.54%  30.76%  11.62%   13.19%    8.59%  9.27%   9.67%     6.97%
Compressed  zip          20.15%  15.09%  14.75%   6.43%    4.41%   18.13%  17.51%  10.30%   10.11%    7.71%  7.57%   7.56%     5.23%
Compressed  7z           11.11%  14.37%  13.85%   5.99%    4.07%   13.26%  11.28%   7.84%    7.43%    5.86%  5.59%   5.56%     3.85%
memory decode
     pixels(M/s)       2,074.3    36.1    44.7    59.3     65.2    250.2   621.5   573.4    830.9    625.5  798.0   837.0     881.9
memory decode time(ms)    99.3  5713.3  4613.2  3472.4   3160.1    823.4   331.4   359.2    247.9    329.3  258.1   246.1     233.6
load image file from Disk 
236MB/s,load time(ms)  3,491.4   528.1   534.8   237.7    164.8    752.1 1,074.0   405.6    460.5    300.0  323.8   337.7     243.4
-------------------------------------------------------------------------------------------------------------------------------------
load+decode time (ms)  3,590.7 6,241.4 5,148.0 3,710.1  3,324.9  1,575.5 1,405.4   764.8    708.4    629.3  581.9   583.8     477.0
                        100.00% 173.82% 143.37% 103.33%   92.60%   43.88%  39.14%  21.30%   19.73%   17.53% 16.21%  16.26%    13.28%
=====================================================================================================================================
```
  
---
by HouSisong@Gmail.com

