libfrg
========
Version  1.3.1   
libfrg: C\C++ codec of FRG image format.  

---
FRG: "Fast Read Graphics" or "Fast Resource Graphics"  
FRG is an image format(like PNG or JPG etc.),for optimize load image time(loading from disk and decoding). It supports 32bit ARGB color image. 
   
_FRG是一种为了优化加载和解码需要的时间而开发的图像文件格式,它支持32bit ARGB颜色的图像。_  
   
---
```
version 1.0.2 performance test:
  macosx10.8.3 ,  CPU:i7 2.3G (single thread), memory: 8G DDR3 1600MHz  ,  7zip use LZMA2 , load image file from Disk 236MB/s;
  1160 bmp files: ARGB32bit color 863,990,576 bytes and 215,980,447 pixels;
  note: .jpg no alpha color

=====================================================================================================================================
                                       .jpg 100 .jpg 95  .jpg 90 .frg 100 quality      .frg 90              .frg 80          .frg 75
                         .bmp    .png   quality quality  quality (0 size  50 size) (0 size 50 size) (0 size 25 size 50 size) (50 size)
image file size         100.00%  15.13%  15.32%   6.81%    4.72%   21.04%  30.79%  11.52%   13.22%    8.52%  9.31%   9.70%     7.00%
Compressed  zip          20.15%  15.09%  14.75%   6.43%    4.41%   17.96%  17.50%  10.29%   10.11%    7.70%  7.56%   7.55%     5.22%
Compressed  7z           11.11%  14.37%  13.85%   5.99%    4.07%   13.24%  11.27%   7.85%    7.41%    5.87%  5.56%   5.53%     3.83%
memory decode
     pixels(M/s)       2,074.3    36.1    44.7    59.3     65.2    255.9   625.1   576.3    830.6    650.2  802.7   870.3     900.2
memory decode time(ms)    99.3  5713.3  4613.2  3472.4   3160.1    804.8   329.5   357.4    248.0    316.8  256.6   236.7     228.8
load image file from Disk 
236MB/s,load time(ms)  3,491.4   528.1   534.8   237.7    164.8    734.6 1,075.0   402.1    461.4    297.5  325.1   338.6     244.3
-------------------------------------------------------------------------------------------------------------------------------------
load+decode time (ms)  3,590.7 6,241.4 5,148.0 3,710.1  3,324.9  1,539.4 1,404.5   759.5    709.4    614.3  581.7   575.2     473.2
                        57.53% 100.00%  82.48%  59.44%   53.27%   24.66%  22.50%  12.17%   11.37%    9.84%  9.32%   9.22%     7.58%
=====================================================================================================================================
```
   
test v1.3.0 DEMO png<->frg : https://github.com/sisong/png2frg_app   
   
---
by HouSisong@Gmail.com

