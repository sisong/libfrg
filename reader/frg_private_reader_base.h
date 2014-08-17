//  frg_private_reader_base.h
/*
 This is the frg copyright.
 
 Copyright (c) 2012-2013 HouSisong All Rights Reserved.
 (The MIT License)
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef __LIBFRG_frg_private_reader_base_h
#define __LIBFRG_frg_private_reader_base_h

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char   TByte;
typedef unsigned int    TUInt32;


/*// frg image file struct:
  head    : sizeof(TFrgFileHead) byte
  --if (!isSingleAlpha){
    --alpha code
      alpha_code_size       : 4byte
    --if (!isAlphaDataUseBytesZip){
        alpha rle code      : alpha_code_size byte
    --}else{
        alpha_rle_code_size : 4byte
        alpha BytesZip code : alpha_code_size-4 byte
    --}
  --}
  --if (!isSingleBGR){
    --bgr code
        bgr_code_size       : 4byte
    --if (!isRGBDataUseBytesZip){
        bgr data code       : bgr_code_size byte
    --}else{
        bgr_data_size       : 4byte
        bgr BytesZip code   : bgr_code_size-4 byte
    --}
  --}
*/

struct TFrgHeadInfo{
    TUInt32         headInfoCodeSize; //==size(TFrgHeadInfo)-4
    TUInt32         imageWidth;
    TUInt32         imageHeight;
    TUInt32         decoder_tempMemoryByteSize;
    TByte           encodingFormat;     //TEncodingFormat
    TByte           savedColorFormat;   //TSavedColorFormat
    TByte           colorInfo;          //TFrgHeadColorInfo
    TByte           _reserved;          //set 0
    TByte           singleColor_b;
    TByte           singleColor_g;
    TByte           singleColor_r;
    TByte           singleColor_a;
};

struct TFrgFileHead{ //frg文件头数据布局(小端格式读写).
    char                frgTagAndVersion[4];
    TUInt32             imageFileSize;
    struct TFrgHeadInfo headInfo;
};
static const int kFrgFileHeadSize=sizeof(struct TFrgFileHead);

#define  kFrgTagAndVersionSize 4
static const char kFrgTagAndVersion[kFrgTagAndVersionSize]={'F','R','G',14};

//占用8bit 存储颜色的相关信息.
typedef enum TFrgHeadColorInfo{
    kColorInfo_isSingleAlpha             = 1<<0,
    kColorInfo_isSingleBGR               = 1<<1,
    kColorInfo_isAlphaDataUseBytesZip    = 1<<2,
    kColorInfo_isRGBDataUseBytesZip      = 1<<3,
    kColorInfo_isAlphaDataNotUseBytesRLE = 1<<4,
} TFrgHeadColorInfo;

//占用8bit 编码数据储存方式.
typedef enum TEncodingFormat{
    kEncodingFormat_stream =1  //字节流.
    //todo:支持允许直接映射到内存或显存的储存格式 kEncodingFormat_memoryMapping? .
} TEncodingFormat;

//占用8bit 储存的颜色的格式和位数.
typedef enum TSavedColorFormat{
    kSavedColorFormat_A8R8G8B8 =32
    //todo:支持 kSavedColorFormat_ xxx ? .
} TSavedColorFormat;

    
//------


//分块大小.
static const int kFrg_ClipWidth=8;
static const int kFrg_ClipHeight=8;

//颜色块类型.  占用高4bit
typedef enum frg_TClipType{
    kFrg_ClipType_index_single_a_w8         = 0, //调色板(有序号数据 单Alpha,8像素宽) +4bit(局部调色板长度-1)
    kFrg_ClipType_index                     = 1, //调色板(有序号数据) +4bit(局部调色板长度-1)
    kFrg_ClipType_single_bgra_w8            = 2, //单色BGRA(8像素宽) +4bit向前匹配的位置处颜色相同(长度0表示自己储存了一个颜色到调色板)
    kFrg_ClipType_single_bgr                = 3, //单色BGR +4bit向前匹配的位置处颜色相同(长度0表示自己储存了一个颜色到调色板)
    kFrg_ClipType_match_table_single_a_w8   = 4, //调色板匹配(单Alpha,8像素宽) +2bit未用+2bit(局部调色板bit数-1) (并储存了一个向前匹配位置(变长1-5byte))
    kFrg_ClipType_match_table               = 5, //调色板匹配 +2bit未用+2bit(局部调色板bit数-1) (并储存了一个向前匹配位置(变长1-5byte))
    kFrg_ClipType_match_image               = 6, //帧间预测 +1bit未用+3bit匹配类型 (并储存了一个向前匹配xy坐标位置(2+2byte))
    kFrg_ClipType_directColor               = 7, //无损压缩(将分块内的所有颜色按顺序放置在调色板中) +3bit未用 +1bit(是否是单Alpha并且8像素宽)
} frg_TClipType;

#define kFrg_MaxSubTableSize (1<<4)  //最大局部调色板大小.
static const TByte kFrg_SubTableSize_to_indexBit[kFrg_MaxSubTableSize+1]={ 0,0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };//局部调色板大小查表得到序号需要的bit数.
static const int kFrg_MaxForwardLength=(1<<4)-1; //利用剩余的4bit空间能储存的最大向前匹配位置数值.

//kFrg_ClipType_match_image属性块匹配类型. 占用3bit
typedef enum frg_TMatchType{
    kFrg_MatchType_move_bgra_w8         = 0,    //平移BGRA(8像素宽)
    kFrg_MatchType_move_bgr             = 1,    //平移BGR
    kFrg_MatchType_left_right_bgra_w8   = 2,    //左右镜像BGRA(8像素宽)
    kFrg_MatchType_left_right_bgr       = 3,    //左右镜像BGR
    kFrg_MatchType_up_down_bgra_w8      = 4,    //上下镜像BGRA(8像素宽)
    kFrg_MatchType_up_down_bgr          = 5,    //上下镜像BGR
} frg_TMatchType;

//---
    
//数据用rle压缩后的包类型2bit
enum TByteRleType{
    kByteRleType_rle0  = 0,    //00表示后面存的压缩0    (包中不需要字节数据)
    kByteRleType_rle255= 1,    //01表示后面存的压缩255  (包中不需要字节数据)
    kByteRleType_rle   = 2,    //10表示后面存的压缩数据  (包中只需储存一个字节数据)
    kByteRleType_unrle = 3     //11表示后面存的未压缩数据 (包中连续储存多个字节数据)
};
static const int kByteRleType_bit=2;
    
#ifdef __cplusplus
}
#endif

#endif //__LIBFRG_frg_private_reader_base_h
