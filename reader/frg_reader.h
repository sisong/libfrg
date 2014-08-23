//  frg_reader.h
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
#ifndef _LIBFRG_frg_reader_h
#define _LIBFRG_frg_reader_h

//用来启动内存访问越界检查,用以避免损坏的frg文件数据或构造特殊frg编码的攻击;速度影响较小(打开可能慢2%).
//#define FRG_READER_RUN_MEM_SAFE_CHECK

#if defined(__BCPLUSPLUS__) || defined( _BORLANDC_ )
  #define _BCC32_OBJ_FOR_DELPHI
  #define _IS_NEED_INLINE_FRG_DRAW_CODE
  //uses bcc32 compile out one ".obj" file can link to Delphi App.
  #pragma option push -V?-
  #define FRG_READER_EXPORT_API  __stdcall 
  #define FRG_READER_STATIC   static 
#else
  #define FRG_READER_EXPORT_API  
  #define FRG_READER_STATIC  
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
typedef enum frg_TColorType {
    kFrg_ColorType_32bit_A8R8G8B8  =1
    //todo: other ?  8bit_A8 24bit_R8G8B8 24bit_B8G8R8 32bit_X8B8G8R8 32bit_A8B8G8R8 32bit_X8R8G8B8 32bit_R8G8B8A8
} frg_TColorType;

//你可以修改kFrg_outColor_*这4个值以支持你需要的rgba32bit颜色输出格式(需要重新编译解码器源代码).
//outColor32 = (b8<<blue_shl) | (g8<<green_shl) | (r8<<red_shl) | (a8<<alpha_shl);
enum {
    kFrg_outColor_blue_shl    =0,
    kFrg_outColor_green_shl   =8,
    kFrg_outColor_red_shl     =16,
    kFrg_outColor_alpha_shl   =24
};
static const int kFrg_outColor_size =4;

struct frg_TPixelsRef{
    void*    pColor;        //图片像素内存起始地址.
    int      width;         //图片宽.
    int      height;        //图片高.
    int      byte_width;    //本行起始像素内存和下一行起始像素内存的间隔字节距离.
    enum frg_TColorType  colorType; //now must set kFrg_ColorType_32bit_A8R8G8B8
};

    
#define frg_BOOL    int
#define frg_FALSE   0
#define frg_TRUE    (!frg_FALSE)

struct frg_TFrgImageInfo{
    int imageFileSize;
    int imageWidth;
    int imageHeight;
    int decoder_tempMemoryByteSize;
};
    
//返回获得frg数据类型信息的大小. //在某些只需要知道文件头信息的时候就不用读取整个文件的数据了.
int FRG_READER_EXPORT_API getFrgHeadSize(void);

//获得frg图片基本信息. //assert(frgCode_end-frgCode_begin>=frgHeadSize) ;  out_frgImageInfo pointer can null;
frg_BOOL FRG_READER_EXPORT_API readFrgImageInfo(const unsigned char* frgCode_begin,const unsigned char* frgCode_end,struct frg_TFrgImageInfo* out_frgImageInfo);


//解码frg格式的图片数据.
frg_BOOL FRG_READER_EXPORT_API readFrgImage(const unsigned char* frgCode_begin,const unsigned char* frgCode_end,const struct frg_TPixelsRef* dst_image,unsigned char* tempMemory,unsigned char* tempMemory_end);

#ifdef __cplusplus
}
#endif

#endif //_LIBFRG_frg_reader_h

