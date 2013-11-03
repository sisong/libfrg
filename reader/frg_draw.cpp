//  frg_draw.cpp
//  for frg_reader
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
#ifdef _IS_NEED_INLINE_FRG_DRAW_CODE
#include "frg_draw.h"
#include "frg_reader.h"

#ifdef FRG_READER_IS_TRY_USE_ARCH64
    enum { kIs_ARCH64=(sizeof(void*)==8)?1:0 };

    static const bool _kIs_BIG_ENDIAN= (*(unsigned short *)"\0\xff" < 0x100);
    static const int kFrg_pairColor0_shl=_kIs_BIG_ENDIAN? 32:0;
    static const int kFrg_pairColor1_shl=_kIs_BIG_ENDIAN? 0:32;

    #include <stddef.h>
    typedef ptrdiff_t TFastInt;
#else
    enum { kIs_ARCH64=0 };
    static const int kFrg_pairColor0_shl=0;
    static const int kFrg_pairColor1_shl=0;
    typedef int TFastInt;
#endif

#ifdef _MSC_VER
typedef unsigned __int64    TUInt64;
#else
typedef unsigned long long  TUInt64;
#endif

#ifdef _MSC_VER
#  define forceinline __forceinline
#else
#  ifdef __GNUC__
#    define forceinline inline __attribute__((always_inline))
#  else
#    define forceinline inline
#  endif
#endif

#define BRGToColor24(b,g,r) ( ((b)<<kFrg_outColor32_blue_shl)|((g)<<kFrg_outColor32_green_shl)|((r)<<kFrg_outColor32_red_shl) )
#define BRGToColor32(b,g,r,a) ( BRGToColor24(b,g,r)|((a)<<kFrg_outColor32_alpha_shl) )
#define color32ToColor64(color32) ((color32) |(((TUInt64)(color32))<<32))
enum { kBGRMask=BRGToColor24(0xFF,0xFF,0xFF) };
const TUInt64 kBGRMask64= color32ToColor64(kBGRMask);

static forceinline TUInt64 toColor64(TUInt32 _c0,TUInt32 _c1){  return ((TUInt64)_c0<<kFrg_pairColor0_shl) | ((TUInt64)_c1<<kFrg_pairColor1_shl); }
static forceinline TUInt64 toAlpha64(TUInt32 _a0,TUInt32 _a1){  return ((TUInt64)_a0<<(kFrg_outColor32_alpha_shl+kFrg_pairColor0_shl)) | ((TUInt64)_a1<<(kFrg_outColor32_alpha_shl+kFrg_pairColor1_shl)); }


struct TPairColor{
    TUInt32     c0;
    TUInt32     c1;
    forceinline TPairColor(){}
    forceinline TPairColor(TUInt32 _c0,TUInt32 _c1):c0(_c0),c1(_c1){}
};

static forceinline void setPairColor(void* pPairColor,TUInt64 c64){  *(TPairColor*)pPairColor=(const TPairColor&)c64; }


//---------

void frg_table_BGR24_to_32bit(void* pDstColor,const TByte* pBGR24,int colorCount){
    TUInt32* pline=(TUInt32*)pDstColor;
    const int fast_width=colorCount&(~3);
    for (int x=0; x<fast_width; x+=4,pBGR24+=3*4) {
        pline[x+0]=BRGToColor24(pBGR24[ 0],pBGR24[ 1],pBGR24[ 2]);
        pline[x+1]=BRGToColor24(pBGR24[ 3],pBGR24[ 4],pBGR24[ 5]);
        pline[x+2]=BRGToColor24(pBGR24[ 6],pBGR24[ 7],pBGR24[ 8]);
        pline[x+3]=BRGToColor24(pBGR24[ 9],pBGR24[10],pBGR24[11]);
    }
    for (int x=fast_width; x<colorCount; ++x,pBGR24+=3) {
        pline[x]=BRGToColor24(pBGR24[0],pBGR24[1],pBGR24[2]);
    }
}


///

#define fill8Pixels(pline,color32,color64)\
    if (kIs_ARCH64){     \
        *(TPairColor*)&pline[0]=color64;\
        *(TPairColor*)&pline[2]=color64;\
        *(TPairColor*)&pline[4]=color64;\
        *(TPairColor*)&pline[6]=color64;\
    }else{               \
        pline[0]=color32;\
        pline[1]=color32;\
        pline[2]=color32;\
        pline[3]=color32;\
        pline[4]=color32;\
        pline[5]=color32;\
        pline[6]=color32;\
        pline[7]=color32;\
    }


#define fill8PixelsWithAlpha(pline,color24,color48,alphaLine)\
    if (kIs_ARCH64){    \
        setPairColor(&pline[0], color48 | toAlpha64(alphaLine[0],alphaLine[1]));\
        setPairColor(&pline[2], color48 | toAlpha64(alphaLine[2],alphaLine[3]));\
        setPairColor(&pline[4], color48 | toAlpha64(alphaLine[4],alphaLine[5]));\
        setPairColor(&pline[6], color48 | toAlpha64(alphaLine[6],alphaLine[7]));\
    }else{              \
        pline[0]=color24 | (alphaLine[0]<<kFrg_outColor32_alpha_shl);\
        pline[1]=color24 | (alphaLine[1]<<kFrg_outColor32_alpha_shl);\
        pline[2]=color24 | (alphaLine[2]<<kFrg_outColor32_alpha_shl);\
        pline[3]=color24 | (alphaLine[3]<<kFrg_outColor32_alpha_shl);\
        pline[4]=color24 | (alphaLine[4]<<kFrg_outColor32_alpha_shl);\
        pline[5]=color24 | (alphaLine[5]<<kFrg_outColor32_alpha_shl);\
        pline[6]=color24 | (alphaLine[6]<<kFrg_outColor32_alpha_shl);\
        pline[7]=color24 | (alphaLine[7]<<kFrg_outColor32_alpha_shl);\
    }


#define copy8Pixels(pline,src_pline)\
    if (kIs_ARCH64){    \
        *(TPairColor*)&pline[0]=*(const TPairColor*)&src_pline[0];\
        *(TPairColor*)&pline[2]=*(const TPairColor*)&src_pline[2];\
        *(TPairColor*)&pline[4]=*(const TPairColor*)&src_pline[4];\
        *(TPairColor*)&pline[6]=*(const TPairColor*)&src_pline[6];\
    }else{              \
        pline[0]=src_pline[0];\
        pline[1]=src_pline[1];\
        pline[2]=src_pline[2];\
        pline[3]=src_pline[3];\
        pline[4]=src_pline[4];\
        pline[5]=src_pline[5];\
        pline[6]=src_pline[6];\
        pline[7]=src_pline[7];\
    }


#define copy8PixelsFromTable_single_a(pline,a32,a64,colorTable,i0,i1,i2,i3,i4,i5,i6,i7)\
    if (kIs_ARCH64){    \
        setPairColor(&pline[0], a64 | toColor64(colorTable[i0],colorTable[i1]));\
        setPairColor(&pline[2], a64 | toColor64(colorTable[i2],colorTable[i3]));\
        setPairColor(&pline[4], a64 | toColor64(colorTable[i4],colorTable[i5]));\
        setPairColor(&pline[6], a64 | toColor64(colorTable[i6],colorTable[i7]));\
    }else{              \
        pline[0]=colorTable[i0] | a32;\
        pline[1]=colorTable[i1] | a32;\
        pline[2]=colorTable[i2] | a32;\
        pline[3]=colorTable[i3] | a32;\
        pline[4]=colorTable[i4] | a32;\
        pline[5]=colorTable[i5] | a32;\
        pline[6]=colorTable[i6] | a32;\
        pline[7]=colorTable[i7] | a32;\
    }

#define copy8Pixels_single_a(pline,a32,a64,src_line)\
    copy8PixelsFromTable_single_a(pline,a32,a64,src_line,0,1,2,3,4,5,6,7)


#define copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,i0,i1,i2,i3,i4,i5,i6,i7)\
    if (kIs_ARCH64){    \
        setPairColor(&pline[0], toColor64(colorTable[i0],colorTable[i1]) | toAlpha64(alphaLine[0],alphaLine[1]));\
        setPairColor(&pline[2], toColor64(colorTable[i2],colorTable[i3]) | toAlpha64(alphaLine[2],alphaLine[3]));\
        setPairColor(&pline[4], toColor64(colorTable[i4],colorTable[i5]) | toAlpha64(alphaLine[4],alphaLine[5]));\
        setPairColor(&pline[6], toColor64(colorTable[i6],colorTable[i7]) | toAlpha64(alphaLine[6],alphaLine[7]));\
    }else{              \
        pline[0]=colorTable[i0] | (alphaLine[0]<<kFrg_outColor32_alpha_shl);\
        pline[1]=colorTable[i1] | (alphaLine[1]<<kFrg_outColor32_alpha_shl);\
        pline[2]=colorTable[i2] | (alphaLine[2]<<kFrg_outColor32_alpha_shl);\
        pline[3]=colorTable[i3] | (alphaLine[3]<<kFrg_outColor32_alpha_shl);\
        pline[4]=colorTable[i4] | (alphaLine[4]<<kFrg_outColor32_alpha_shl);\
        pline[5]=colorTable[i5] | (alphaLine[5]<<kFrg_outColor32_alpha_shl);\
        pline[6]=colorTable[i6] | (alphaLine[6]<<kFrg_outColor32_alpha_shl);\
        pline[7]=colorTable[i7] | (alphaLine[7]<<kFrg_outColor32_alpha_shl);\
    }


#define copy8PixelsWithAlpha(pline,alphaLine,src_pline)\
    copy8PixelsFromTableWithAlpha(pline,alphaLine,src_pline,0,1,2,3,4,5,6,7)

#define copy8PixelsFromTable(pline,colorTable,i0,i1,i2,i3,i4,i5,i6,i7)\
    if (kIs_ARCH64){    \
        *(TPairColor*)&pline[0]=TPairColor(colorTable[i0],colorTable[i1]);\
        *(TPairColor*)&pline[2]=TPairColor(colorTable[i2],colorTable[i3]);\
        *(TPairColor*)&pline[4]=TPairColor(colorTable[i4],colorTable[i5]);\
        *(TPairColor*)&pline[6]=TPairColor(colorTable[i6],colorTable[i7]);\
    }else{              \
        pline[0]=colorTable[i0];\
        pline[1]=colorTable[i1];\
        pline[2]=colorTable[i2];\
        pline[3]=colorTable[i3];\
        pline[4]=colorTable[i4];\
        pline[5]=colorTable[i5];\
        pline[6]=colorTable[i6];\
        pline[7]=colorTable[i7];\
    }




#define copyLinePixelsWithBGRAndAlpha(__INC_SIGN,__SRC_SIGN)\
    if(dst->width==kFrg_ClipWidth){             \
        for (int y=0; y<dst->height; ++y) {     \
            if (kIs_ARCH64){                    \
                setPairColor(&pline[0], (toColor64(src_pline[           0],src_pline[__INC_SIGN 1]) & kBGRMask64) | toAlpha64(alphaLine[0],alphaLine[1]));\
                setPairColor(&pline[2], (toColor64(src_pline[__INC_SIGN 2],src_pline[__INC_SIGN 3]) & kBGRMask64) | toAlpha64(alphaLine[2],alphaLine[3]));\
                setPairColor(&pline[4], (toColor64(src_pline[__INC_SIGN 4],src_pline[__INC_SIGN 5]) & kBGRMask64) | toAlpha64(alphaLine[4],alphaLine[5]));\
                setPairColor(&pline[6], (toColor64(src_pline[__INC_SIGN 6],src_pline[__INC_SIGN 7]) & kBGRMask64) | toAlpha64(alphaLine[6],alphaLine[7]));\
            }else{  \
                pline[0]=(src_pline[           0]&kBGRMask) | (alphaLine[0]<<kFrg_outColor32_alpha_shl);\
                pline[1]=(src_pline[__INC_SIGN 1]&kBGRMask) | (alphaLine[1]<<kFrg_outColor32_alpha_shl);\
                pline[2]=(src_pline[__INC_SIGN 2]&kBGRMask) | (alphaLine[2]<<kFrg_outColor32_alpha_shl);\
                pline[3]=(src_pline[__INC_SIGN 3]&kBGRMask) | (alphaLine[3]<<kFrg_outColor32_alpha_shl);\
                pline[4]=(src_pline[__INC_SIGN 4]&kBGRMask) | (alphaLine[4]<<kFrg_outColor32_alpha_shl);\
                pline[5]=(src_pline[__INC_SIGN 5]&kBGRMask) | (alphaLine[5]<<kFrg_outColor32_alpha_shl);\
                pline[6]=(src_pline[__INC_SIGN 6]&kBGRMask) | (alphaLine[6]<<kFrg_outColor32_alpha_shl);\
                pline[7]=(src_pline[__INC_SIGN 7]&kBGRMask) | (alphaLine[7]<<kFrg_outColor32_alpha_shl);\
            }       \
            alphaLine+=alpha_byte_width;        \
            src_pline=(TUInt32*)( ((TByte*)src_pline) __SRC_SIGN byte_width );  \
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );                     \
        }                                       \
    }else{                                      \
        for (int y=0; y<dst->height; ++y) {     \
            for (int x=0; x<dst->width; ++x){   \
                pline[x]=(src_pline[__INC_SIGN x]&kBGRMask) | (alphaLine[x]<<kFrg_outColor32_alpha_shl);\
            }                                   \
            alphaLine+=alpha_byte_width;        \
            src_pline=(TUInt32*)( ((TByte*)src_pline) __SRC_SIGN byte_width );  \
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );                     \
        }           \
    }



////

void frg_fillPixels_32bit(const struct frg_TPixelsRef* dst,const TByte* pBGRA32){
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    const TUInt32 color32=BRGToColor32(pBGRA32[0],pBGRA32[1],pBGRA32[2],pBGRA32[3]);
    
    const int fast_width=width&(~7);
    const TPairColor color64(color32,color32);
    for (int y=0; y<dst->height; ++y) {
        for (int x=0; x<fast_width; x+=8)
            fill8Pixels((pline+x),color32,color64);
        for (int x=fast_width; x<width; ++x)
            pline[x]=color32;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_fillPixels_32bit_withAlpha(const struct frg_TPixelsRef* dst,const TByte* pBGR24,const TByte* alphaLine,int alpha_byte_width){
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    const TUInt32 color24=BRGToColor24(pBGR24[0],pBGR24[1],pBGR24[2]);
    
    const int fast_width=width&(~7);
    const TUInt64 color48=color32ToColor64(color24);
    for (int y=0; y<dst->height; ++y) {
        for (int x=0; x<fast_width; x+=8)
            fill8PixelsWithAlpha((pline+x),color24,color48,(alphaLine+x));
        for (int x=fast_width; x<width; ++x)
            pline[x]=color24 | (alphaLine[x]<<kFrg_outColor32_alpha_shl);
        alphaLine+=alpha_byte_width;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_single_bgra_w8(const struct frg_TPixelsRef* dst,TUInt32 color24,TUInt32 alpha){
    const TUInt32 color32=color24 | (alpha<<kFrg_outColor32_alpha_shl);
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    
    const TPairColor color64(color32,color32);
    for (int y=0; y<dst->height; ++y) {
        fill8Pixels(pline,color32,color64);
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_single_bgr(const struct frg_TPixelsRef* dst,const TUInt32 color24,const TByte* alphaLine,int alpha_byte_width){
    TInt32 byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (dst->width==kFrg_ClipWidth){
        const TUInt64 color48=color32ToColor64(color24);
        for (int y=0; y<dst->height; ++y){
            fill8PixelsWithAlpha(pline,color24,color48,alphaLine);
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x){
                pline[x]=color24 | (alphaLine[x]<<kFrg_outColor32_alpha_shl);
            }
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


void frg_copyPixels_32bit_index_single_a_w8_4bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha){
    alpha<<=kFrg_outColor32_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    
    const TUInt64 a64=color32ToColor64(alpha);
    for (int y=0; y<dst->height; ++y) {
        const TFastInt index01=index2List[0];
        const TFastInt index23=index2List[1];
        const TFastInt index45=index2List[2];
        const TFastInt index67=index2List[3];
        copy8PixelsFromTable_single_a(pline,alpha,a64,colorTable,
                                      (index01&15),(index01>>4),
                                      (index23&15),(index23>>4),
                                      (index45&15),(index45>>4),
                                      (index67&15),(index67>>4));
        index2List+=4;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}


void frg_copyPixels_32bit_index_4bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,TInt32 alpha_byte_width){
    TInt32 byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (dst->width==kFrg_ClipWidth){
        for (int y=0; y<dst->height; ++y) {
            const TFastInt index01=index2List[0];
            const TFastInt index23=index2List[1];
            const TFastInt index45=index2List[2];
            const TFastInt index67=index2List[3];
            copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,
                                         (index01&15),(index01>>4),
                                         (index23&15),(index23>>4),
                                         (index45&15),(index45>>4),
                                         (index67&15),(index67>>4));
            index2List+=4;
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        TFastInt  indexPos=0;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x,++indexPos){
                const TFastInt index=(index2List[indexPos>>1]>>(indexPos*4&7))&15;
                pline[x]=colorTable[index] | (alphaLine[x]<<kFrg_outColor32_alpha_shl);
            }
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}

void frg_copyPixels_32bit_index_single_a_w8_3bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha){
    alpha<<=kFrg_outColor32_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    
    const TUInt64 a64=color32ToColor64(alpha);
    for (int y=0; y<dst->height; ++y) {
        const TFastInt index0=index2List[0];
        const TFastInt index1=index2List[1];
        const TFastInt index2=index2List[2];
        copy8PixelsFromTable_single_a(pline,alpha,a64,colorTable,
                                      (                   index0&7),(              (index0>>3)&7),
                                      (((index0>>6)|(index1<<2))&7),(              (index1>>1)&7),
                                      (              (index1>>4)&7),(((index1>>7)|(index2<<1))&7),
                                      (              (index2>>2)&7),(                  index2>>5));
        index2List+=3;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_index_3bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,TInt32 alpha_byte_width){
    TInt32 byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (dst->width==kFrg_ClipWidth){
        for (int y=0; y<dst->height; ++y) {
            const TFastInt index0=index2List[0];
            const TFastInt index1=index2List[1];
            const TFastInt index2=index2List[2];
            copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,
                                         (                   index0&7),(              (index0>>3)&7),
                                         (((index0>>6)|(index1<<2))&7),(              (index1>>1)&7),
                                         (              (index1>>4)&7),(((index1>>7)|(index2<<1))&7),
                                         (              (index2>>2)&7),(                  index2>>5));
            index2List+=3;
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        TFastInt  curValue=0;
        TFastInt  curBit=0;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x){
                if (curBit<3){
                    curValue|=((*index2List)<<curBit);
                    curBit+=8;
                    ++index2List;
                }
                pline[x]=colorTable[curValue&7] | (alphaLine[x]<<kFrg_outColor32_alpha_shl);
                curValue>>=3;
                curBit-=3;
            }
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


void frg_copyPixels_32bit_index_single_a_w8_2bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha){
    alpha<<=kFrg_outColor32_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    
    const TUInt64 a64=color32ToColor64(alpha);
    for (int y=0; y<dst->height; ++y) {
        const TFastInt index0123=index2List[0];
        const TFastInt index4567=index2List[1];
        copy8PixelsFromTable_single_a(pline,alpha,a64,colorTable,
                                      ( index0123    &3),((index0123>>2)&3),
                                      ((index0123>>4)&3),((index0123>>6)&3),
                                      ( index4567    &3),((index4567>>2)&3),
                                      ((index4567>>4)&3),((index4567>>6)&3));
        index2List+=2;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_index_2bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,TInt32 alpha_byte_width){
    TInt32 byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (dst->width==kFrg_ClipWidth){
        for (int y=0; y<dst->height; ++y) {
            const TFastInt index0123=index2List[0];
            const TFastInt index4567=index2List[1];
            copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,
                                         ( index0123    &3),((index0123>>2)&3),
                                         ((index0123>>4)&3),((index0123>>6)&3),
                                         ( index4567    &3),((index4567>>2)&3),
                                         ((index4567>>4)&3),((index4567>>6)&3));
            
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            index2List+=2;
            alphaLine+=alpha_byte_width;
        }
    }else{
        TFastInt  indexPos=0;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x,++indexPos){
                const TFastInt index=(index2List[indexPos>>2]>>(indexPos*2&7))&3;
                pline[x]=colorTable[index] | (alphaLine[x]<<kFrg_outColor32_alpha_shl);
            }
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            alphaLine+=alpha_byte_width;
        }
    }
}


void frg_copyPixels_32bit_index_single_a_w8_1bit(const struct frg_TPixelsRef* dst,const TUInt32* _colorTable,const TByte* index2List,TUInt32 alpha){
    alpha<<=kFrg_outColor32_alpha_shl;
    TUInt32 colorTable[2];
    colorTable[0]=_colorTable[0] | alpha; 
    colorTable[1]=_colorTable[1] | alpha;
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    
    for (int y=0; y<dst->height; ++y) {
        const TFastInt indexs=index2List[y];
        copy8PixelsFromTable(pline,colorTable,
                             ( indexs    &1),((indexs>>1)&1),
                             ((indexs>>2)&1),((indexs>>3)&1),
                             ((indexs>>4)&1),((indexs>>5)&1),
                             ((indexs>>6)&1),((indexs>>7)&1));
        
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_index_1bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,TInt32 alpha_byte_width){
    TInt32 byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (dst->width==kFrg_ClipWidth){
        for (int y=0; y<dst->height; ++y) {
            const TFastInt indexs=index2List[0];
            copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,
                                         ( indexs    &1),((indexs>>1)&1),
                                         ((indexs>>2)&1),((indexs>>3)&1),
                                         ((indexs>>4)&1),((indexs>>5)&1),
                                         ((indexs>>6)&1),((indexs>>7)&1));
            
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            index2List+=1;
            alphaLine+=alpha_byte_width;
        }
    }else{
        TFastInt  indexPos=0;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x,++indexPos){
                const TFastInt index=(index2List[indexPos>>3]>>(indexPos&7))&1;
                pline[x]=colorTable[index] | (alphaLine[x]<<kFrg_outColor32_alpha_shl);
            }
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            alphaLine+=alpha_byte_width;
        }
    }
}

////

void frg_copyPixels_32bit_match(const struct frg_TPixelsRef* dst,const void* src_line0,enum frg_TMatchType matchType,const TByte* matchXY,const TByte* alphaLine,TInt32 alpha_byte_width){
    const TUInt32* src_pline=(const TUInt32*)src_line0;
    TInt32 byte_width=dst->byte_width;
    src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width*(matchXY[2]|(matchXY[3]<<8)) +(matchXY[0]|(matchXY[1]<<8))*sizeof(TUInt32) );
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    switch (matchType) {
        case kFrg_MatchType_move_bgra_w8:{
            for (int y=0; y<dst->height; ++y){
                copy8Pixels(pline,src_pline);
                src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            }
        } break;
        case kFrg_MatchType_left_right_bgra_w8:{
            src_pline+=8-1;//dst->width-1;
            for (int y=0; y<dst->height; ++y) {
                copy8PixelsFromTable(pline,src_pline,
                                     ( 0),(-1),
                                     (-2),(-3),
                                     (-4),(-5),
                                     (-6),(-7));
                
                src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            }
        } break;
        case kFrg_MatchType_up_down_bgra_w8:{
            src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width*(dst->height-1) );
            for (int y=0; y<dst->height; ++y){
                copy8Pixels(pline,src_pline);
                src_pline=(TUInt32*)( ((TByte*)src_pline)-byte_width );
                pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            }
        } break;
        case kFrg_MatchType_move_bgr:{
            copyLinePixelsWithBGRAndAlpha(+,+);
        } break;
        case kFrg_MatchType_left_right_bgr:{
            src_pline+=dst->width-1;
            copyLinePixelsWithBGRAndAlpha(-,+);
        } break;
        case kFrg_MatchType_up_down_bgr:{
            src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width*(dst->height-1) );
            copyLinePixelsWithBGRAndAlpha(+,-);
        } break;
        default:{
            assert(false);
        } break;
    }
}


void frg_copyPixels_32bit_directColor_single_a_w8(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,TUInt32 alpha){
    alpha<<=kFrg_outColor32_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    
    const TUInt64 a64=color32ToColor64(alpha);
    for (int y=0; y<dst->height; ++y) {
        copy8Pixels_single_a(pline,alpha,a64,colorTable);
        colorTable+=kFrg_ClipWidth; //colorTable+=dst->width;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_directColor(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* alphaLine,TInt32 alpha_byte_width){
    if (dst->width==kFrg_ClipWidth){
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            copy8PixelsWithAlpha(pline,alphaLine,colorTable);
            
            alphaLine+=alpha_byte_width;
            colorTable+=kFrg_ClipWidth; //colorTable+=dst->width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x){
                pline[x]=colorTable[x] | (alphaLine[x]<<kFrg_outColor32_alpha_shl);
            }
            alphaLine+=alpha_byte_width;
            colorTable+=dst->width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


#endif //_IS_NEED_INLINE_FRG_DRAW_CODE

