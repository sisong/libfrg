//  frg_writer.h
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
#ifndef _LIBFRG_frg_writer_h
#define _LIBFRG_frg_writer_h

#include <vector>

namespace frg{
    
    struct TFrgParameter;
    struct TFrgPixels32Ref;

//save as frg 
void writeFrgImage(std::vector<unsigned char>& outFrgCode,const TFrgPixels32Ref& srcImage,const TFrgParameter& parameter);

    //质量控制.
    static const float kFrg_quality_max=100;    //无损压缩,不推荐.
    static const float kFrg_quality_default=80; //推荐压缩quality选择80左右(质量和大小适中); 另外也推荐81,略大质量更好(质量损失过大的块会无损保存).
    static const float kFrg_quality_min=0;      //不推荐小kFrg_quality_default太多的值,质量较差.
    //保持质量的情况下的压缩大小控制.
    static const float kFrg_size_maxUnZipSpeed=100;//最大化内存解压速度,但生成的文件较大(从磁盘加载可能较慢).
    static const float kFrg_size_default=50; //推荐: 在大多数设备上,硬盘加载时间+内存解压的时间最少.
    static const float kFrg_size_minSize=0; //最小化生成的文件大小,但内存解压较慢.
    
    struct TFrgBGRA32{
        unsigned char b;    //blue
        unsigned char g;    //green
        unsigned char r;    //red
        unsigned char a;    //alpha     (if no alpha,must set a=255)
    };
    inline TFrgBGRA32 getFrgBGRA32(unsigned char r8,unsigned char g8,unsigned char b8,unsigned char a8) {
        TFrgBGRA32 result; result.b=b8;  result.g=g8;  result.r=r8;  result.a=a8;
        return result;
    }
    inline TFrgBGRA32 getFrgColor(unsigned char r8,unsigned char g8,unsigned char b8) {
        return getFrgBGRA32(r8,g8,b8,255);
    }
    
    struct TFrgPixels32Ref{
        TFrgBGRA32* pColor;     //图片像素内存起始地址.
        int         width;      //图片宽[0..65535]
        int         height;     //图片高[0..65535]
        int         byte_width; //本行起始像素内存和下一行起始像素内存的间隔字节距离.
    };
    
    struct TFrgParameter{
        //frg支持的几个压缩调节参数. 
        float  quality;                     //质量设置,值域[0--100].
        float  compressRatio;               //=(压缩后大小/压缩前大小); 压缩时大于这个值就不使用数据压缩，而是直接存储. (最优值本质上=1-平台磁盘速度/平台解压速度).
        bool   isMustFitColorTable;         //对于颜色较多的块,是否也必须使用调色板(这样的话文件会更小但质量可能变差).
        bool   isDeleteEmptyColor;          //全透明的区域,颜色会被忽略,设置为true.
        bool   isAlphaUseRleAdvance;        //alpha数据压缩前是否用rle预压缩(这样数据会被压缩的更小一些).
        int    alphaRleAdvanceParameter;    //alpha数据rle预压缩参数,一般取6--16.
        float  alphaRleAdvanceCompressRatio;//alpha数据rle预压缩压缩了要求.
        
        //setCtrlParameter将frg编码的多个控制参数映射到两个正交的控制参数上(质量和压缩大小;该函数的实现是按作者的经验设定的系数值,你也可以手工定制你的控制参数).
        explicit TFrgParameter(float _quality=kFrg_quality_default,float _compressSizeParameter=kFrg_size_default){
            setParameter(_quality,_compressSizeParameter);
        }
        
        void setParameter(float _quality,float _compressSizeParameter);
    };
    
}//end namespace frg

#endif
