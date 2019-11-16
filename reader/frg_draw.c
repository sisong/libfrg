//  frg_draw.c
//  private for frg_reader
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
#include "frg_reader.h"
#include "frg_draw.h"

#ifndef _IS_NEED_INLINE_FRG_DRAW_CODE

#ifdef __cplusplus
extern "C" {
#endif
    
#define BRGToColor24(b,g,r) ( ((TUInt32)(b)<<kFrg_outColor_blue_shl)|((TUInt32)(g)<<kFrg_outColor_green_shl)|((TUInt32)(r)<<kFrg_outColor_red_shl) )
#define BRGToColor32(b,g,r,a) ( BRGToColor24(b,g,r)|((TUInt32)(a)<<kFrg_outColor_alpha_shl) )
enum { kBGRMask=BRGToColor24(0xFF,0xFF,0xFF) };

static const int _temp_bl=1;
#define kIs_LITTLE_ENDIAN ((*(const char*)&_temp_bl)!=0)

typedef TUInt TFastUInt;
struct TPairColor{
    TUInt32     c0;
    TUInt32     c1;
};

#define DefinePairColor(pairColorName,_c0,_c1)  struct TPairColor pairColorName; { pairColorName.c0=(_c0); pairColorName.c1=(_c1); }
#define SetPairColor(pdst,_c0,_c1) { ((TUInt32*)(pdst))[0]=(_c0); ((TUInt32*)(pdst))[1]=(_c1); }
    
///

#define fill8Pixels(pline,pairColor){ \
    *(struct TPairColor*)&pline[0]=pairColor;\
    *(struct TPairColor*)&pline[2]=pairColor;\
    *(struct TPairColor*)&pline[4]=pairColor;\
    *(struct TPairColor*)&pline[6]=pairColor;\
}

#define fill8PixelsWithAlpha(pline,color24,alphaLine){ \
    pline[0]=color24 | (alphaLine[0]<<kFrg_outColor_alpha_shl);\
    pline[1]=color24 | (alphaLine[1]<<kFrg_outColor_alpha_shl);\
    pline[2]=color24 | (alphaLine[2]<<kFrg_outColor_alpha_shl);\
    pline[3]=color24 | (alphaLine[3]<<kFrg_outColor_alpha_shl);\
    pline[4]=color24 | (alphaLine[4]<<kFrg_outColor_alpha_shl);\
    pline[5]=color24 | (alphaLine[5]<<kFrg_outColor_alpha_shl);\
    pline[6]=color24 | (alphaLine[6]<<kFrg_outColor_alpha_shl);\
    pline[7]=color24 | (alphaLine[7]<<kFrg_outColor_alpha_shl);\
}

#define copy8Pixels(pline,src_pline){ \
    *(struct TPairColor*)&pline[0]=*(const struct TPairColor*)&src_pline[0];\
    *(struct TPairColor*)&pline[2]=*(const struct TPairColor*)&src_pline[2];\
    *(struct TPairColor*)&pline[4]=*(const struct TPairColor*)&src_pline[4];\
    *(struct TPairColor*)&pline[6]=*(const struct TPairColor*)&src_pline[6];\
}

#define copy8PixelsFromTable_single_a(pline,a32,colorTable,i0,i1,i2,i3,i4,i5,i6,i7){ \
    pline[0]=colorTable[i0] | a32;\
    pline[1]=colorTable[i1] | a32;\
    pline[2]=colorTable[i2] | a32;\
    pline[3]=colorTable[i3] | a32;\
    pline[4]=colorTable[i4] | a32;\
    pline[5]=colorTable[i5] | a32;\
    pline[6]=colorTable[i6] | a32;\
    pline[7]=colorTable[i7] | a32;\
}

#define copy8Pixels_single_a(pline,a32,src_line)\
    copy8PixelsFromTable_single_a(pline,a32,src_line,0,1,2,3,4,5,6,7)

#define copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,i0,i1,i2,i3,i4,i5,i6,i7){ \
    pline[0]=colorTable[i0] | (alphaLine[0]<<kFrg_outColor_alpha_shl);\
    pline[1]=colorTable[i1] | (alphaLine[1]<<kFrg_outColor_alpha_shl);\
    pline[2]=colorTable[i2] | (alphaLine[2]<<kFrg_outColor_alpha_shl);\
    pline[3]=colorTable[i3] | (alphaLine[3]<<kFrg_outColor_alpha_shl);\
    pline[4]=colorTable[i4] | (alphaLine[4]<<kFrg_outColor_alpha_shl);\
    pline[5]=colorTable[i5] | (alphaLine[5]<<kFrg_outColor_alpha_shl);\
    pline[6]=colorTable[i6] | (alphaLine[6]<<kFrg_outColor_alpha_shl);\
    pline[7]=colorTable[i7] | (alphaLine[7]<<kFrg_outColor_alpha_shl);\
}

#define copy8PixelsWithAlpha(pline,alphaLine,src_pline)\
    copy8PixelsFromTableWithAlpha(pline,alphaLine,src_pline,0,1,2,3,4,5,6,7)

#define copy8PixelsFromTable(pline,colorTable,i0,i1,i2,i3,i4,i5,i6,i7){ \
    SetPairColor(&pline[0],colorTable[i0],colorTable[i1]);\
    SetPairColor(&pline[2],colorTable[i2],colorTable[i3]);\
    SetPairColor(&pline[4],colorTable[i4],colorTable[i5]);\
    SetPairColor(&pline[6],colorTable[i6],colorTable[i7]);\
}


#define copyLinePixelsWithBGRAndAlpha(__SRC_INC_SIGN,__SRC_LINE_SIGN)\
    if(width==kFrg_ClipWidth){              \
        int y;                              \
        for (y=0; y<height; ++y) {          \
            pline[0]=(src_pline[               0]&kBGRMask) | (alphaLine[0]<<kFrg_outColor_alpha_shl);\
            pline[1]=(src_pline[__SRC_INC_SIGN 1]&kBGRMask) | (alphaLine[1]<<kFrg_outColor_alpha_shl);\
            pline[2]=(src_pline[__SRC_INC_SIGN 2]&kBGRMask) | (alphaLine[2]<<kFrg_outColor_alpha_shl);\
            pline[3]=(src_pline[__SRC_INC_SIGN 3]&kBGRMask) | (alphaLine[3]<<kFrg_outColor_alpha_shl);\
            pline[4]=(src_pline[__SRC_INC_SIGN 4]&kBGRMask) | (alphaLine[4]<<kFrg_outColor_alpha_shl);\
            pline[5]=(src_pline[__SRC_INC_SIGN 5]&kBGRMask) | (alphaLine[5]<<kFrg_outColor_alpha_shl);\
            pline[6]=(src_pline[__SRC_INC_SIGN 6]&kBGRMask) | (alphaLine[6]<<kFrg_outColor_alpha_shl);\
            pline[7]=(src_pline[__SRC_INC_SIGN 7]&kBGRMask) | (alphaLine[7]<<kFrg_outColor_alpha_shl);\
                                            \
            alphaLine+=alpha_byte_width;    \
            src_pline=(TUInt32*)( ((TByte*)src_pline) __SRC_LINE_SIGN byte_width ); \
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );                         \
        }                                   \
    }else{                                  \
        int y;                              \
        for (y=0; y<height; ++y) {          \
            int x;                          \
            for (x=0; x<width; ++x){        \
                pline[x]=(src_pline[__SRC_INC_SIGN x]&kBGRMask) | (alphaLine[x]<<kFrg_outColor_alpha_shl);\
            }                               \
            alphaLine+=alpha_byte_width;    \
            src_pline=(TUInt32*)( ((TByte*)src_pline) __SRC_LINE_SIGN byte_width ); \
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );                         \
        }                                   \
    }


#ifdef __cplusplus
}
#endif

//---------

void frg_table_BGR24_to_32bit(void* pDstColor,const TByte* pBGR24,TUInt colorCount){
    const TUInt fast_width=colorCount&(~3);
    TUInt32* pline=(TUInt32*)pDstColor;
    TUInt x;
    for (x=0; x<fast_width; x+=4,pBGR24+=3*4) {
        pline[x+0]=BRGToColor24(pBGR24[ 0],pBGR24[ 1],pBGR24[ 2]);
        pline[x+1]=BRGToColor24(pBGR24[ 3],pBGR24[ 4],pBGR24[ 5]);
        pline[x+2]=BRGToColor24(pBGR24[ 6],pBGR24[ 7],pBGR24[ 8]);
        pline[x+3]=BRGToColor24(pBGR24[ 9],pBGR24[10],pBGR24[11]);
    }
    for (x=fast_width; x<colorCount; ++x,pBGR24+=3) {
        pline[x]=BRGToColor24(pBGR24[0],pBGR24[1],pBGR24[2]);
    }
}

////

void frg_fillPixels_32bit(const struct frg_TPixelsRef* dst,const TByte* pBGRA32){
    const int height=dst->height;
    const int byte_width=dst->byte_width;
    const int width=dst->width;
    const int fast_width=width&(~7);
    const TUInt32 color32=BRGToColor32(pBGRA32[0],pBGRA32[1],pBGRA32[2],pBGRA32[3]);
    TUInt32* pline=(TUInt32*)dst->pColor;
    int y;
    DefinePairColor(pairColor,color32,color32);
    
    for (y=0; y<height; ++y) {
        int x;
        for (x=0; x<fast_width; x+=8)
            fill8Pixels((pline+x),pairColor);
        for (x=fast_width; x<width; ++x)
            pline[x]=color32;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_fillPixels_32bit_withAlpha(const struct frg_TPixelsRef* dst,const TByte* pBGR24,const TByte* alphaLine,int alpha_byte_width){
    const int height=dst->height;
    const int byte_width=dst->byte_width;
    const int width=dst->width;
    const int fast_width=width&(~7);
    const TUInt32 color24=BRGToColor24(pBGR24[0],pBGR24[1],pBGR24[2]);
    TUInt32* pline=(TUInt32*)dst->pColor;
    int y;
    
    for (y=0; y<height; ++y) {
        int x;
        for (x=0; x<fast_width; x+=8)
            fill8PixelsWithAlpha((pline+x),color24,(alphaLine+x));
        for (x=fast_width; x<width; ++x)
            pline[x]=color24 | (alphaLine[x]<<kFrg_outColor_alpha_shl);
        alphaLine+=alpha_byte_width;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_single_bgra_w8(const struct frg_TPixelsRef* dst,TUInt32 color32){
    const int height=dst->height;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    int y;
    DefinePairColor(pairColor,color32,color32);
    /*
    for (y=0; y<height; ++y) {
        fill8Pixels(pline,pairColor);
     
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
    */
    for (y=-height;;) {
        fill8Pixels(pline,pairColor);
        
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        ++y; if (!y) break;
    }
}

void frg_copyPixels_32bit_single_bgr(const struct frg_TPixelsRef* dst,const TUInt32 color24,const TByte* alphaLine,int alpha_byte_width){
    const int height=dst->height;
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (width==kFrg_ClipWidth){
        int y;
        for (y=0; y<height; ++y){
            fill8PixelsWithAlpha(pline,color24,alphaLine);
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        int y;
        for (y=0; y<height; ++y) {
            int x;
            for (x=0; x<width; ++x){
                pline[x]=color24 | (alphaLine[x]<<kFrg_outColor_alpha_shl);
            }
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


void frg_copyPixels_32bit_index_single_a_w8_4bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* indexsList,TUInt32 alpha){
    const int height=dst->height;
    const int byte_width=dst->byte_width;
    const TUInt32 alpha32=alpha<<kFrg_outColor_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;

    struct TByte4{
        TByte b0;
        TByte b1;
        TByte b2;
        TByte b3;
    };
    struct TPackByte4{
        union{
            TFastUInt       value;
            struct TByte4   bs;
        };
    };
    
    if (kIs_LITTLE_ENDIAN){
        int y;
        indexsList+=height*4;
        for (y=-height;;) {
            struct TPackByte4 indexs;
            indexs.bs=((const struct TByte4*)indexsList)[y];
            copy8PixelsFromTable_single_a(pline,alpha32,colorTable,
                                          ((indexs.value    )&15),((indexs.value>>(   4))&15),
                                          ((indexs.value>> 8)&15),((indexs.value>>( 8+4))&15),
                                          ((indexs.value>>16)&15),((indexs.value>>(16+4))&15),
                                          ((indexs.value>>24)&15),((indexs.value>>(24+4))&15));
            
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            ++y; if (!y) break;
        }
    }else{ //is BIG_ENDIAN
        int y;
        for (y=-height;;) {
            const TFastUInt index01=indexsList[0];
            const TFastUInt index23=indexsList[1];
            const TFastUInt index45=indexsList[2];
            const TFastUInt index67=indexsList[3];
            copy8PixelsFromTable_single_a(pline,alpha32,colorTable,
                                          (index01&15),(index01>>4),
                                          (index23&15),(index23>>4),
                                          (index45&15),(index45>>4),
                                          (index67&15),(index67>>4));
            indexsList+=4;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            ++y; if (!y) break;
        }
    }
}

void frg_copyPixels_32bit_index_4bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* indexsList,const TByte* alphaLine,int alpha_byte_width){
    const int height=dst->height;
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (width==kFrg_ClipWidth){
        int y;
        for (y=0; y<height; ++y) {
            const TFastUInt index01=indexsList[0];
            const TFastUInt index23=indexsList[1];
            const TFastUInt index45=indexsList[2];
            const TFastUInt index67=indexsList[3];
            copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,
                                         (index01&15),(index01>>4),
                                         (index23&15),(index23>>4),
                                         (index45&15),(index45>>4),
                                         (index67&15),(index67>>4));
            indexsList+=4;
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        int y;
        TFastUInt indexPos=0;
        for (y=0; y<height; ++y) {
            int x;
            for (x=0; x<width; ++x,++indexPos){
                TFastUInt index=(indexsList[indexPos>>1]>>(indexPos*4&7))&15;
                pline[x]=colorTable[index] | (alphaLine[x]<<kFrg_outColor_alpha_shl);
            }
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}

void frg_copyPixels_32bit_index_single_a_w8_3bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* indexsList,TUInt32 alpha){
    const int height=dst->height;
    const int byte_width=dst->byte_width;
    const TUInt32 alpha32=alpha<<kFrg_outColor_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;

    struct TByte3{
        TByte b0;
        TByte b1;
        TByte b2;
    };
    struct TPackByte3{
        union{
            TFastUInt       value;
            struct TByte3   bs;
        };
    };
    if (kIs_LITTLE_ENDIAN){
        int y;
        for (y=-height;;) {
            struct TPackByte3 indexs;
            indexs.bs=*(const struct TByte3*)indexsList;
            copy8PixelsFromTable_single_a(pline,alpha32,colorTable,
                                          ((indexs.value    )&7),((indexs.value>> 3)&7),
                                          ((indexs.value>> 6)&7),((indexs.value>> 9)&7),
                                          ((indexs.value>>12)&7),((indexs.value>>15)&7),
                                          ((indexs.value>>18)&7),((indexs.value>>21)&7));
            indexsList+=3;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            ++y; if (!y) break;
        }
    }else{ //is BIG_ENDIAN
        int y;
        for (y=-height;;) {
            const TFastUInt index0=indexsList[0];
            const TFastUInt index1=indexsList[1];
            const TFastUInt index2=indexsList[2];
            copy8PixelsFromTable_single_a(pline,alpha32,colorTable,
                                          (                   index0&7),(              (index0>>3)&7),
                                          (((index0>>6)|(index1<<2))&7),(              (index1>>1)&7),
                                          (              (index1>>4)&7),(((index1>>7)|(index2<<1))&7),
                                          (              (index2>>2)&7),(               index2>>5   ));
            indexsList+=3;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            ++y; if (!y) break;
        }
    }
}

void frg_copyPixels_32bit_index_3bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* indexsList,const TByte* alphaLine,int alpha_byte_width){
    const int height=dst->height;
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (width==kFrg_ClipWidth){
        int y;
        for (y=0; y<height; ++y) {
            const TFastUInt index0=indexsList[0];
            const TFastUInt index1=indexsList[1];
            const TFastUInt index2=indexsList[2];
            copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,
                                          (                   index0&7),(              (index0>>3)&7),
                                          (((index0>>6)|(index1<<2))&7),(              (index1>>1)&7),
                                          (              (index1>>4)&7),(((index1>>7)|(index2<<1))&7),
                                          (              (index2>>2)&7),(               index2>>5   ));
            indexsList+=3;
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        int y;
        TFastUInt  curValue=0;
        TFastUInt  curBit=0;
        for (y=0; y<height; ++y) {
            int x;
            for (x=0; x<width; ++x){
                if (curBit<3){
                    curValue|=((*indexsList)<<curBit);
                    curBit+=8;
                    ++indexsList;
                }
                pline[x]=colorTable[curValue&7] | (alphaLine[x]<<kFrg_outColor_alpha_shl);
                curValue>>=3;
                curBit-=3;
            }
            alphaLine+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


void frg_copyPixels_32bit_index_single_a_w8_2bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* indexsList,TUInt32 alpha){
    const int height=dst->height;
    const int byte_width=dst->byte_width;
    const TUInt32 alpha32=alpha<<kFrg_outColor_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;

    struct TByte2{
        TByte b0;
        TByte b1;
    };
    struct TPackByte2{
        union{
            TFastUInt       value;
            struct TByte2   bs;
        };
    };
    
    if (kIs_LITTLE_ENDIAN){
        int y;
        indexsList+=height*2;
        for (y=-height;;) {
            struct TPackByte2 indexs;
            indexs.bs=((const struct TByte2*)indexsList)[y];
            copy8PixelsFromTable_single_a(pline,alpha32,colorTable,
                                          ((indexs.value    )&3),((indexs.value>>(   2))&3),
                                          ((indexs.value>> 4)&3),((indexs.value>>( 4+2))&3),
                                          ((indexs.value>> 8)&3),((indexs.value>>( 8+2))&3),
                                          ((indexs.value>>12)&3),((indexs.value>>(12+2))&3));
            
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            ++y; if (!y) break;
        }
    }else{ //is BIG_ENDIAN
        int y;
        for (y=-height;;) {
            const TFastUInt index0123=indexsList[0];
            const TFastUInt index4567=indexsList[1];
            copy8PixelsFromTable_single_a(pline,alpha32,colorTable,
                                          ( index0123    &3),((index0123>>2)&3),
                                          ((index0123>>4)&3),((index0123>>6)  ),
                                          ( index4567    &3),((index4567>>2)&3),
                                          ((index4567>>4)&3),((index4567>>6)  ));
            indexsList+=2;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            ++y; if (!y) break;
        }
    }
}

void frg_copyPixels_32bit_index_2bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* indexsList,const TByte* alphaLine,int alpha_byte_width){
    const int height=dst->height;
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (width==kFrg_ClipWidth){
        int y;
        for (y=0; y<height; ++y) {
            const TFastUInt index0123=indexsList[0];
            const TFastUInt index4567=indexsList[1];
            copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,
                                         ( index0123    &3),((index0123>>2)&3),
                                         ((index0123>>4)&3),((index0123>>6)  ),
                                         ( index4567    &3),((index4567>>2)&3),
                                         ((index4567>>4)&3),((index4567>>6)  ));
            
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            indexsList+=2;
            alphaLine+=alpha_byte_width;
        }
    }else{
        int y;
        TFastUInt  indexPos=0;
        for (y=0; y<height; ++y) {
            int x;
            for (x=0; x<width; ++x,++indexPos){
                const TFastUInt index=(indexsList[indexPos>>2]>>(indexPos*2&7))&3;
                pline[x]=colorTable[index] | (alphaLine[x]<<kFrg_outColor_alpha_shl);
            }
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            alphaLine+=alpha_byte_width;
        }
    }
}


void frg_copyPixels_32bit_index_single_a_w8_1bit(const struct frg_TPixelsRef* dst,const TUInt32* _colorTable,const TByte* indexsList,TUInt32 alpha){
    const int height=dst->height;
    const int byte_width=dst->byte_width;
    const TUInt32 alpha32=alpha<<kFrg_outColor_alpha_shl;
    TUInt32* pline=(TUInt32*)(dst->pColor);
    int y;
    TUInt32 colorTable[2];
    colorTable[0]=_colorTable[0] | alpha32;
    colorTable[1]=_colorTable[1] | alpha32;
    
    for (y=0; y<height; ++y) {
        const TFastUInt indexs=indexsList[y];
        copy8PixelsFromTable(pline,colorTable,
                             ( indexs    &1),((indexs>>1)&1),
                             ((indexs>>2)&1),((indexs>>3)&1),
                             ((indexs>>4)&1),((indexs>>5)&1),
                             ((indexs>>6)&1),((indexs>>7)  ));
        
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_index_1bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* indexsList,const TByte* alphaLine,int alpha_byte_width){
    const int height=dst->height;
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (width==kFrg_ClipWidth){
        int y;
        for (y=0; y<height; ++y) {
            const TFastUInt indexs=indexsList[0];
            copy8PixelsFromTableWithAlpha(pline,alphaLine,colorTable,
                                         ( indexs    &1),((indexs>>1)&1),
                                         ((indexs>>2)&1),((indexs>>3)&1),
                                         ((indexs>>4)&1),((indexs>>5)&1),
                                         ((indexs>>6)&1),((indexs>>7)  ));
            
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            indexsList+=1;
            alphaLine+=alpha_byte_width;
        }
    }else{
        int y;
        TFastUInt  indexPos=0;
        for (y=0; y<height; ++y) {
            int x;
            for (x=0; x<width; ++x,++indexPos){
                const TFastUInt index=(indexsList[indexPos>>3]>>(indexPos&7))&1;
                pline[x]=colorTable[index] | (alphaLine[x]<<kFrg_outColor_alpha_shl);
            }
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            alphaLine+=alpha_byte_width;
        }
    }
}

////

void frg_copyPixels_32bit_match(const struct frg_TPixelsRef* dst,const TUInt32* src_pline,enum frg_TMatchType matchType,const TByte* alphaLine,int alpha_byte_width){
    const int height=dst->height;
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    switch (matchType) {
        case kFrg_MatchType_move_bgra_w8:{
            int y;
            for (y=0; y<height; ++y){
                copy8Pixels(pline,src_pline);
                src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            }
        } break;
        case kFrg_MatchType_left_right_bgra_w8:{
            int y;
            src_pline+=8-1;//dst->width-1;
            for (y=0; y<height; ++y) {
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
            int y;
            src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width*(height-1) );
            for (y=0; y<height; ++y){
                copy8Pixels(pline,src_pline);
                src_pline=(TUInt32*)( ((TByte*)src_pline)-byte_width );
                pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            }
        } break;
        case kFrg_MatchType_move_bgr:{
            copyLinePixelsWithBGRAndAlpha(+,+);
        } break;
        case kFrg_MatchType_left_right_bgr:{
            src_pline+=width-1;
            copyLinePixelsWithBGRAndAlpha(-,+);
        } break;
        case kFrg_MatchType_up_down_bgr:{
            src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width*(height-1) );
            copyLinePixelsWithBGRAndAlpha(+,-);
        } break;
        default:{
            assert(0);
        } break;
    }
}


void frg_copyPixels_32bit_directColor_single_a_w8(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,TUInt32 alpha){
    const int height=dst->height;
    const int byte_width=dst->byte_width;
    const TUInt32 alpha32=alpha<<kFrg_outColor_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    int y;
    for (y=0; y<height; ++y) {
        copy8Pixels_single_a(pline,alpha32,colorTable);
        colorTable+=kFrg_ClipWidth; //colorTable+=dst->width;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_directColor(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* alphaLine,int alpha_byte_width){
    const int height=dst->height;
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    
    if (width==kFrg_ClipWidth){
        int y;
        for (y=0; y<height; ++y) {
            copy8PixelsWithAlpha(pline,alphaLine,colorTable);
            
            alphaLine+=alpha_byte_width;
            colorTable+=kFrg_ClipWidth; //colorTable+=dst->width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        int y;
        for (y=0; y<height; ++y) {
            int x;
            for (x=0; x<width; ++x){
                pline[x]=colorTable[x] | (alphaLine[x]<<kFrg_outColor_alpha_shl);
            }
            alphaLine+=alpha_byte_width;
            colorTable+=width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


#endif //_IS_NEED_INLINE_FRG_DRAW_CODE

