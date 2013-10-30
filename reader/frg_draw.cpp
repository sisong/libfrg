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
#else
    enum { kIs_ARCH64=0 };
    static const int kFrg_pairColor0_shl=0;
    static const int kFrg_pairColor1_shl=0;
#endif

#ifdef _MSC_VER
typedef unsigned __int64    TUInt64;
#else
typedef unsigned long long  TUInt64;
#endif


#define BRGToColor24(b,g,r) ( ((b)<<kFrg_outColor32_blue_shl)|((g)<<kFrg_outColor32_green_shl)|((r)<<kFrg_outColor32_red_shl) )
#define BRGToColor32(b,g,r,a) ( BRGToColor24(b,g,r)|((a)<<kFrg_outColor32_alpha_shl) )
enum { kBGRMask=BRGToColor24(0xFF,0xFF,0xFF) };

struct TPairColor{
    TUInt32     c0;
    TUInt32     c1;
    inline explicit TPairColor(){}
    inline explicit TPairColor(const TUInt32 _c0,const TUInt32 _c1):c0(_c0),c1(_c1){}
    
    static inline TPairColor orAlphaPair(TUInt64 c64,TUInt64 a0,TUInt64 a1) {
        c64=c64 | (a0<<(kFrg_outColor32_alpha_shl+kFrg_pairColor0_shl)) | (a1<<(kFrg_outColor32_alpha_shl+kFrg_pairColor1_shl));
        return *(TPairColor*)&c64;
    }
    static inline TPairColor orColorPair(TUInt64 a64,TUInt64 c0,TUInt64 c1) {
        a64=a64 | (c0<<kFrg_pairColor0_shl) | (c1<<kFrg_pairColor1_shl);
        return *(TPairColor*)&a64;
    }
};

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

////

void frg_fillPixels_32bit(const struct frg_TPixelsRef* dst,const TByte* pBGRA32){
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    const TUInt32 color32=BRGToColor32(pBGRA32[0],pBGRA32[1],pBGRA32[2],pBGRA32[3]);
    
    if (kIs_ARCH64){
        const TPairColor color64(color32,color32);
        const int fast_width=width&(~15);
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<fast_width; x+=16) {
                *(TPairColor*)&pline[x+ 0]=color64;
                *(TPairColor*)&pline[x+ 2]=color64;
                *(TPairColor*)&pline[x+ 4]=color64;
                *(TPairColor*)&pline[x+ 6]=color64;
                *(TPairColor*)&pline[x+ 8]=color64;
                *(TPairColor*)&pline[x+10]=color64;
                *(TPairColor*)&pline[x+12]=color64;
                *(TPairColor*)&pline[x+14]=color64;
            }
            for (int x=fast_width; x<width; ++x) {
                pline[x]=color64.c0;
            }
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        const int fast_width=width&(~7);
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<fast_width; x+=8) {
                pline[x+0]=color32;
                pline[x+1]=color32;
                pline[x+2]=color32;
                pline[x+3]=color32;
                pline[x+4]=color32;
                pline[x+5]=color32;
                pline[x+6]=color32;
                pline[x+7]=color32;
            }
            for (int x=fast_width; x<width; ++x) {
                pline[x]=color32;
            }
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}

void frg_fillPixels_32bit_withAlpha(const struct frg_TPixelsRef* dst,const TByte* pBGR24,const TByte* alpha,int alpha_byte_width){

    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    const TUInt32 color24=BRGToColor24(pBGR24[0],pBGR24[1],pBGR24[2]);
    if (kIs_ARCH64){
        const int fast_width=width&(~7);
        const TUInt64 color48=color24|(((TUInt64)color24)<<32);
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<fast_width; x+=8) {
                *(TPairColor*)&pline[x+0]=TPairColor::orAlphaPair(color48,alpha[x+0],alpha[x+1]);
                *(TPairColor*)&pline[x+2]=TPairColor::orAlphaPair(color48,alpha[x+2],alpha[x+3]);
                *(TPairColor*)&pline[x+4]=TPairColor::orAlphaPair(color48,alpha[x+4],alpha[x+5]);
                *(TPairColor*)&pline[x+6]=TPairColor::orAlphaPair(color48,alpha[x+6],alpha[x+7]);
            }
            for (int x=fast_width; x<width; ++x) {
                pline[x]=((TUInt32)color48) | (alpha[x]<<kFrg_outColor32_alpha_shl);
            }
            alpha+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        const int fast_width=width&(~3);
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<fast_width; x+=4) {
                pline[x+0]=color24 | (alpha[x+0]<<kFrg_outColor32_alpha_shl);
                pline[x+1]=color24 | (alpha[x+1]<<kFrg_outColor32_alpha_shl);
                pline[x+2]=color24 | (alpha[x+2]<<kFrg_outColor32_alpha_shl);
                pline[x+3]=color24 | (alpha[x+3]<<kFrg_outColor32_alpha_shl);
            }
            for (int x=fast_width; x<width; ++x) {
                pline[x]=color24 | (alpha[x]<<kFrg_outColor32_alpha_shl);
            }
            alpha+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


void frg_copyPixels_32bit_single_bgra_w8(const struct frg_TPixelsRef* dst,TUInt32 color24,TUInt32 alpha){
    const TUInt32 color32=color24 | (alpha<<kFrg_outColor32_alpha_shl);
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    if (kIs_ARCH64){
        const TPairColor color64(color32,color32);
        for (int y=0; y<dst->height; ++y) {
            *(TPairColor*)&pline[0]=color64;
            *(TPairColor*)&pline[2]=color64;
            *(TPairColor*)&pline[4]=color64;
            *(TPairColor*)&pline[6]=color64;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        for (int y=0; y<dst->height; ++y) {
            pline[0]=color32;
            pline[1]=color32;
            pline[2]=color32;
            pline[3]=color32;
            pline[4]=color32;
            pline[5]=color32;
            pline[6]=color32;
            pline[7]=color32;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width);
        }
    }
}

void frg_copyPixels_32bit_single_bgr(const struct frg_TPixelsRef* dst,const TUInt32 color24,const TByte* alpha,int alpha_byte_width){
    if (dst->width==kFrg_ClipWidth){
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        if (kIs_ARCH64){
            const TUInt64 color48=color24|(((TUInt64)color24)<<32);
            for (int y=0; y<dst->height; ++y) {
                *(TPairColor*)&pline[0]=TPairColor::orAlphaPair(color48,alpha[0],alpha[1]);
                *(TPairColor*)&pline[2]=TPairColor::orAlphaPair(color48,alpha[2],alpha[3]);
                *(TPairColor*)&pline[4]=TPairColor::orAlphaPair(color48,alpha[4],alpha[5]);
                *(TPairColor*)&pline[6]=TPairColor::orAlphaPair(color48,alpha[6],alpha[7]);
                
                alpha+=alpha_byte_width;
                pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            }
        }else{
            for (int y=0; y<dst->height; ++y) {
                pline[0]=color24 | (alpha[0]<<kFrg_outColor32_alpha_shl);
                pline[1]=color24 | (alpha[1]<<kFrg_outColor32_alpha_shl);
                pline[2]=color24 | (alpha[2]<<kFrg_outColor32_alpha_shl);
                pline[3]=color24 | (alpha[3]<<kFrg_outColor32_alpha_shl);
                pline[4]=color24 | (alpha[4]<<kFrg_outColor32_alpha_shl);
                pline[5]=color24 | (alpha[5]<<kFrg_outColor32_alpha_shl);
                pline[6]=color24 | (alpha[6]<<kFrg_outColor32_alpha_shl);
                pline[7]=color24 | (alpha[7]<<kFrg_outColor32_alpha_shl);
                
                alpha+=alpha_byte_width;
                pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            }
        }
    }else{
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x){
                pline[x]=color24 | (alpha[x]<<kFrg_outColor32_alpha_shl);
            }
            alpha+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


void frg_copyPixels_32bit_index_single_a_w8_4bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha){
    alpha<<=kFrg_outColor32_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    if (kIs_ARCH64){
        const TUInt64 a64=alpha|(((TUInt64)alpha)<<32);
        for (int y=0; y<dst->height; ++y) {
            const TUInt64 index01=index2List[0];
            const TUInt64 index23=index2List[1];
            const TUInt64 index45=index2List[2];
            const TUInt64 index67=index2List[3];
            *(TPairColor*)&pline[0]=TPairColor::orColorPair(a64,colorTable[index01&15],colorTable[index01>>4]);
            *(TPairColor*)&pline[2]=TPairColor::orColorPair(a64,colorTable[index23&15],colorTable[index23>>4]);
            *(TPairColor*)&pline[4]=TPairColor::orColorPair(a64,colorTable[index45&15],colorTable[index45>>4]);
            *(TPairColor*)&pline[6]=TPairColor::orColorPair(a64,colorTable[index67&15],colorTable[index67>>4]);
            
            index2List+=4;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        for (int y=0; y<dst->height; ++y) {
            const int index01=index2List[0];
            pline[0]=colorTable[index01&15] | alpha;
            pline[1]=colorTable[index01>>4] | alpha;
            const int index23=index2List[1];
            pline[2]=colorTable[index23&15] | alpha;
            pline[3]=colorTable[index23>>4] | alpha;
            const int index45=index2List[2];
            pline[4]=colorTable[index45&15] | alpha;
            pline[5]=colorTable[index45>>4] | alpha;
            const int index67=index2List[3];
            pline[6]=colorTable[index67&15] | alpha;
            pline[7]=colorTable[index67>>4] | alpha;
            
            index2List+=4;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


void frg_copyPixels_32bit_index_4bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alpha,TInt32 alpha_byte_width){
    if (dst->width==kFrg_ClipWidth){
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            int index01=index2List[0];
            pline[0]=colorTable[index01&15] | (alpha[0]<<kFrg_outColor32_alpha_shl);
            pline[1]=colorTable[index01>>4] | (alpha[1]<<kFrg_outColor32_alpha_shl);
            int index23=index2List[1];
            pline[2]=colorTable[index23&15] | (alpha[2]<<kFrg_outColor32_alpha_shl);
            pline[3]=colorTable[index23>>4] | (alpha[3]<<kFrg_outColor32_alpha_shl);
            int index45=index2List[2];
            pline[4]=colorTable[index45&15] | (alpha[4]<<kFrg_outColor32_alpha_shl);
            pline[5]=colorTable[index45>>4] | (alpha[5]<<kFrg_outColor32_alpha_shl);
            int index67=index2List[3];
            pline[6]=colorTable[index67&15] | (alpha[6]<<kFrg_outColor32_alpha_shl);
            pline[7]=colorTable[index67>>4] | (alpha[7]<<kFrg_outColor32_alpha_shl);
            
            index2List+=4;
            alpha+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        int  indexPos=0;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x,++indexPos){
                int index=(index2List[indexPos>>1]>>(indexPos*4&7))&15;
                pline[x]=colorTable[index] | (alpha[x]<<kFrg_outColor32_alpha_shl);
            }
            alpha+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}

void frg_copyPixels_32bit_index_single_a_w8_3bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha){
    alpha<<=kFrg_outColor32_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    if (kIs_ARCH64){
        const TUInt64 a64=alpha|(((TUInt64)alpha)<<32);
        for (int y=0; y<dst->height; ++y) {
            const TUInt64 index0=index2List[0];
            const TUInt64 index1=index2List[1];
            const TUInt64 index2=index2List[2];
            *(TPairColor*)&pline[0]=TPairColor::orColorPair(a64,colorTable[index0&7],colorTable[(index0>>3)&7]);
            *(TPairColor*)&pline[2]=TPairColor::orColorPair(a64,colorTable[((index0>>6)|(index1<<2))&7],colorTable[(index1>>1)&7]);
            *(TPairColor*)&pline[4]=TPairColor::orColorPair(a64,colorTable[(index1>>4)&7],colorTable[((index1>>7)|(index2<<1))&7]);
            *(TPairColor*)&pline[6]=TPairColor::orColorPair(a64,colorTable[(index2>>2)&7],colorTable[index2>>5]);
            
            index2List+=3;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        for (int y=0; y<dst->height; ++y) {
            int index0=index2List[0];
            pline[0]=colorTable[index0&7]      | alpha;
            pline[1]=colorTable[(index0>>3)&7] | alpha;
            int index1=index2List[1];
            pline[2]=colorTable[((index0>>6)|(index1<<2))&7] | alpha;
            pline[3]=colorTable[(index1>>1)&7] | alpha;
            pline[4]=colorTable[(index1>>4)&7] | alpha;
            int index2=index2List[2];
            pline[5]=colorTable[((index1>>7)|(index2<<1))&7] | alpha;
            pline[6]=colorTable[(index2>>2)&7] | alpha;
            pline[7]=colorTable[index2>>5]     | alpha;
            
            index2List+=3;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}

void frg_copyPixels_32bit_index_3bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alpha,TInt32 alpha_byte_width){
    if (dst->width==kFrg_ClipWidth){
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            int index0=index2List[0];
            pline[0]=colorTable[index0&7]      | (alpha[0]<<kFrg_outColor32_alpha_shl);
            pline[1]=colorTable[(index0>>3)&7] | (alpha[1]<<kFrg_outColor32_alpha_shl);
            int index1=index2List[1];
            pline[2]=colorTable[((index0>>6)|(index1<<2))&7] | (alpha[2]<<kFrg_outColor32_alpha_shl);
            pline[3]=colorTable[(index1>>1)&7] | (alpha[3]<<kFrg_outColor32_alpha_shl);
            pline[4]=colorTable[(index1>>4)&7] | (alpha[4]<<kFrg_outColor32_alpha_shl);
            int index2=index2List[2];
            pline[5]=colorTable[((index1>>7)|(index2<<1))&7] | (alpha[5]<<kFrg_outColor32_alpha_shl);
            pline[6]=colorTable[(index2>>2)&7] | (alpha[6]<<kFrg_outColor32_alpha_shl);
            pline[7]=colorTable[index2>>5]     | (alpha[7]<<kFrg_outColor32_alpha_shl);
            
            index2List+=3;
            alpha+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        int  curValue=0;
        int  curBit=0;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x){
                if (curBit<3){
                    curValue|=((*index2List)<<curBit);
                    curBit+=8;
                    ++index2List;
                }
                pline[x]=colorTable[curValue&7] | (alpha[x]<<kFrg_outColor32_alpha_shl);
                curValue>>=3;
                curBit-=3;
            }
            alpha+=alpha_byte_width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


void frg_copyPixels_32bit_index_single_a_w8_2bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha){
    alpha<<=kFrg_outColor32_alpha_shl;
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    if (kIs_ARCH64){
        const TUInt64 a64=alpha|(((TUInt64)alpha)<<32);
        for (int y=0; y<dst->height; ++y) {
            const TUInt64 index0123=index2List[0];
            const TUInt64 index4567=index2List[1];
            *(TPairColor*)&pline[0]=TPairColor::orColorPair(a64,colorTable[ index0123    &3],colorTable[(index0123>>2)&3]);
            *(TPairColor*)&pline[2]=TPairColor::orColorPair(a64,colorTable[(index0123>>4)&3],colorTable[(index0123>>6)&3]);
            *(TPairColor*)&pline[4]=TPairColor::orColorPair(a64,colorTable[ index4567    &3],colorTable[(index4567>>2)&3]);
            *(TPairColor*)&pline[6]=TPairColor::orColorPair(a64,colorTable[(index4567>>4)&3],colorTable[(index4567>>6)&3]);
            
            index2List+=2;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        for (int y=0; y<dst->height; ++y) {
            int index0123=index2List[0];
            pline[0]=colorTable[ index0123    &3] | alpha;
            pline[1]=colorTable[(index0123>>2)&3] | alpha;
            pline[2]=colorTable[(index0123>>4)&3] | alpha;
            pline[3]=colorTable[(index0123>>6)&3] | alpha;
            int index4567=index2List[1];
            pline[4]=colorTable[ index4567    &3] | alpha;
            pline[5]=colorTable[(index4567>>2)&3] | alpha;
            pline[6]=colorTable[(index4567>>4)&3] | alpha;
            pline[7]=colorTable[(index4567>>6)&3] | alpha;
            
            index2List+=2;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}

void frg_copyPixels_32bit_index_2bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alpha,TInt32 alpha_byte_width){
    if (dst->width==kFrg_ClipWidth){
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            int index0123=index2List[0];
            pline[0]=colorTable[ index0123    &3] | (alpha[0]<<kFrg_outColor32_alpha_shl);
            pline[1]=colorTable[(index0123>>2)&3] | (alpha[1]<<kFrg_outColor32_alpha_shl);
            pline[2]=colorTable[(index0123>>4)&3] | (alpha[2]<<kFrg_outColor32_alpha_shl);
            pline[3]=colorTable[(index0123>>6)&3] | (alpha[3]<<kFrg_outColor32_alpha_shl);
            int index4567=index2List[1];
            pline[4]=colorTable[ index4567    &3] | (alpha[4]<<kFrg_outColor32_alpha_shl);
            pline[5]=colorTable[(index4567>>2)&3] | (alpha[5]<<kFrg_outColor32_alpha_shl);
            pline[6]=colorTable[(index4567>>4)&3] | (alpha[6]<<kFrg_outColor32_alpha_shl);
            pline[7]=colorTable[(index4567>>6)&3] | (alpha[7]<<kFrg_outColor32_alpha_shl);
            
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            index2List+=2;
            alpha+=alpha_byte_width;
        }
    }else{
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        int  indexPos=0;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x,++indexPos){
                int index=(index2List[indexPos>>2]>>(indexPos*2&7))&3;
                pline[x]=colorTable[index] | (alpha[x]<<kFrg_outColor32_alpha_shl);
            }
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            alpha+=alpha_byte_width;
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
        int indexs=index2List[y];
        pline[0]=colorTable[ indexs    &1];
        pline[1]=colorTable[(indexs>>1)&1];
        pline[2]=colorTable[(indexs>>2)&1];
        pline[3]=colorTable[(indexs>>3)&1];
        pline[4]=colorTable[(indexs>>4)&1];
        pline[5]=colorTable[(indexs>>5)&1];
        pline[6]=colorTable[(indexs>>6)&1];
        pline[7]=colorTable[(indexs>>7)&1];
        
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_index_1bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alpha,TInt32 alpha_byte_width){
    if (dst->width==kFrg_ClipWidth){
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            int indexs=index2List[0];
            pline[0]=colorTable[ indexs    &1] | (alpha[0]<<kFrg_outColor32_alpha_shl);
            pline[1]=colorTable[(indexs>>1)&1] | (alpha[1]<<kFrg_outColor32_alpha_shl);
            pline[2]=colorTable[(indexs>>2)&1] | (alpha[2]<<kFrg_outColor32_alpha_shl);
            pline[3]=colorTable[(indexs>>3)&1] | (alpha[3]<<kFrg_outColor32_alpha_shl);
            pline[4]=colorTable[(indexs>>4)&1] | (alpha[4]<<kFrg_outColor32_alpha_shl);
            pline[5]=colorTable[(indexs>>5)&1] | (alpha[5]<<kFrg_outColor32_alpha_shl);
            pline[6]=colorTable[(indexs>>6)&1] | (alpha[6]<<kFrg_outColor32_alpha_shl);
            pline[7]=colorTable[(indexs>>7)&1] | (alpha[7]<<kFrg_outColor32_alpha_shl);
            
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            index2List+=1;
            alpha+=alpha_byte_width;
        }
    }else{
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        int  indexPos=0;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x,++indexPos){
                int index=(index2List[indexPos>>3]>>(indexPos&7))&1;
                pline[x]=colorTable[index] | (alpha[x]<<kFrg_outColor32_alpha_shl);
            }
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            alpha+=alpha_byte_width;
        }
    }
}

////

void frg_copyPixels_32bit_match(const struct frg_TPixelsRef* dst,const void* src_line0,enum frg_TMatchType matchType,const TByte* matchXY,const TByte* alpha,TInt32 alpha_byte_width){
    const TUInt32* src_pline=(const TUInt32*)src_line0;
    TInt32 byte_width=dst->byte_width;
    src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width*(matchXY[2]|(matchXY[3]<<8)) +(matchXY[0]|(matchXY[1]<<8))*sizeof(TUInt32) );
    TUInt32* pline=(TUInt32*)dst->pColor;
    switch (matchType) {
        case kFrg_MatchType_move_bgra_w8:{
            if (kIs_ARCH64){
                for (int y=0; y<dst->height; ++y) {
                    *(TPairColor*)&pline[0]=*(const TPairColor*)&src_pline[0];
                    *(TPairColor*)&pline[2]=*(const TPairColor*)&src_pline[2];
                    *(TPairColor*)&pline[4]=*(const TPairColor*)&src_pline[4];
                    *(TPairColor*)&pline[6]=*(const TPairColor*)&src_pline[6];
                    
                    src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }else{
                for (int y=0; y<dst->height; ++y) {
                    pline[0]=src_pline[0];
                    pline[1]=src_pline[1];
                    pline[2]=src_pline[2];
                    pline[3]=src_pline[3];
                    pline[4]=src_pline[4];
                    pline[5]=src_pline[5];
                    pline[6]=src_pline[6];
                    pline[7]=src_pline[7];
                    
                    src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }
        } break;
        case kFrg_MatchType_left_right_bgra_w8:{
            src_pline+=dst->height-1;
            for (int y=0; y<dst->height; ++y) {
                pline[0]=src_pline[-0];
                pline[1]=src_pline[-1];
                pline[2]=src_pline[-2];
                pline[3]=src_pline[-3];
                pline[4]=src_pline[-4];
                pline[5]=src_pline[-5];
                pline[6]=src_pline[-6];
                pline[7]=src_pline[-7];
                
                src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                pline=(TUInt32*)( ((TByte*)pline)+byte_width );
            }
        } break;
        case kFrg_MatchType_up_down_bgra_w8:{
            src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width*(dst->height-1) );
            if (kIs_ARCH64){
                for (int y=0; y<dst->height; ++y) {
                    *(TPairColor*)&pline[0]=*(const TPairColor*)&src_pline[0];
                    *(TPairColor*)&pline[2]=*(const TPairColor*)&src_pline[2];
                    *(TPairColor*)&pline[4]=*(const TPairColor*)&src_pline[4];
                    *(TPairColor*)&pline[6]=*(const TPairColor*)&src_pline[6];
                    
                    src_pline=(TUInt32*)( ((TByte*)src_pline)-byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }else{
                for (int y=0; y<dst->height; ++y) {
                    pline[0]=src_pline[0];
                    pline[1]=src_pline[1];
                    pline[2]=src_pline[2];
                    pline[3]=src_pline[3];
                    pline[4]=src_pline[4];
                    pline[5]=src_pline[5];
                    pline[6]=src_pline[6];
                    pline[7]=src_pline[7];
                    
                    src_pline=(TUInt32*)( ((TByte*)src_pline)-byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }
        } break;
        case kFrg_MatchType_move_bgr:{
            if(dst->width==kFrg_ClipWidth){
                for (int y=0; y<dst->height; ++y) {
                    pline[0]=(src_pline[0]&kBGRMask) | (alpha[0]<<kFrg_outColor32_alpha_shl);
                    pline[1]=(src_pline[1]&kBGRMask) | (alpha[1]<<kFrg_outColor32_alpha_shl);
                    pline[2]=(src_pline[2]&kBGRMask) | (alpha[2]<<kFrg_outColor32_alpha_shl);
                    pline[3]=(src_pline[3]&kBGRMask) | (alpha[3]<<kFrg_outColor32_alpha_shl);
                    pline[4]=(src_pline[4]&kBGRMask) | (alpha[4]<<kFrg_outColor32_alpha_shl);
                    pline[5]=(src_pline[5]&kBGRMask) | (alpha[5]<<kFrg_outColor32_alpha_shl);
                    pline[6]=(src_pline[6]&kBGRMask) | (alpha[6]<<kFrg_outColor32_alpha_shl);
                    pline[7]=(src_pline[7]&kBGRMask) | (alpha[7]<<kFrg_outColor32_alpha_shl);
                    
                    alpha+=alpha_byte_width;
                    src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }else{
                for (int y=0; y<dst->height; ++y) {
                    for (int x=0; x<dst->width; ++x){
                        pline[x]=(src_pline[x]&kBGRMask) | (alpha[x]<<kFrg_outColor32_alpha_shl);
                    }
                    alpha+=alpha_byte_width;
                    src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }
        } break;
        case kFrg_MatchType_left_right_bgr:{
            src_pline+=dst->height-1;
            if(dst->width==kFrg_ClipWidth){
                for (int y=0; y<dst->height; ++y) {
                    pline[0]=(src_pline[-0]&kBGRMask) | (alpha[0]<<kFrg_outColor32_alpha_shl);
                    pline[1]=(src_pline[-1]&kBGRMask) | (alpha[1]<<kFrg_outColor32_alpha_shl);
                    pline[2]=(src_pline[-2]&kBGRMask) | (alpha[2]<<kFrg_outColor32_alpha_shl);
                    pline[3]=(src_pline[-3]&kBGRMask) | (alpha[3]<<kFrg_outColor32_alpha_shl);
                    pline[4]=(src_pline[-4]&kBGRMask) | (alpha[4]<<kFrg_outColor32_alpha_shl);
                    pline[5]=(src_pline[-5]&kBGRMask) | (alpha[5]<<kFrg_outColor32_alpha_shl);
                    pline[6]=(src_pline[-6]&kBGRMask) | (alpha[6]<<kFrg_outColor32_alpha_shl);
                    pline[7]=(src_pline[-7]&kBGRMask) | (alpha[7]<<kFrg_outColor32_alpha_shl);
                    
                    alpha+=alpha_byte_width;
                    src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }else{
                for (int y=0; y<dst->height; ++y) {
                    for (int x=0; x<dst->width; ++x){
                        pline[x]=(src_pline[-x]&kBGRMask) | (alpha[x]<<kFrg_outColor32_alpha_shl);
                    }
                    alpha+=alpha_byte_width;
                    src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }
        } break;
        case kFrg_MatchType_up_down_bgr:{
            src_pline=(TUInt32*)( ((TByte*)src_pline)+byte_width*(dst->height-1) );
            if(dst->width==kFrg_ClipWidth){
                for (int y=0; y<dst->height; ++y) {
                    pline[0]=(src_pline[0]&kBGRMask) | (alpha[0]<<kFrg_outColor32_alpha_shl);
                    pline[1]=(src_pline[1]&kBGRMask) | (alpha[1]<<kFrg_outColor32_alpha_shl);
                    pline[2]=(src_pline[2]&kBGRMask) | (alpha[2]<<kFrg_outColor32_alpha_shl);
                    pline[3]=(src_pline[3]&kBGRMask) | (alpha[3]<<kFrg_outColor32_alpha_shl);
                    pline[4]=(src_pline[4]&kBGRMask) | (alpha[4]<<kFrg_outColor32_alpha_shl);
                    pline[5]=(src_pline[5]&kBGRMask) | (alpha[5]<<kFrg_outColor32_alpha_shl);
                    pline[6]=(src_pline[6]&kBGRMask) | (alpha[6]<<kFrg_outColor32_alpha_shl);
                    pline[7]=(src_pline[7]&kBGRMask) | (alpha[7]<<kFrg_outColor32_alpha_shl);
                    
                    alpha+=alpha_byte_width;
                    src_pline=(TUInt32*)( ((TByte*)src_pline)-byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }else{
                for (int y=0; y<dst->height; ++y) {
                    for (int x=0; x<dst->width; ++x){
                        pline[x]=(src_pline[x]&kBGRMask) | (alpha[x]<<kFrg_outColor32_alpha_shl);
                    }
                    alpha+=alpha_byte_width;
                    src_pline=(TUInt32*)( ((TByte*)src_pline)-byte_width );
                    pline=(TUInt32*)( ((TByte*)pline)+byte_width );
                }
            }
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
    if (kIs_ARCH64){
        const TUInt64 a64=alpha|(((TUInt64)alpha)<<32);
        for (int y=0; y<dst->height; ++y) {
            *(TPairColor*)&pline[0]=TPairColor::orColorPair(a64,colorTable[0],colorTable[1]);
            *(TPairColor*)&pline[2]=TPairColor::orColorPair(a64,colorTable[2],colorTable[3]);
            *(TPairColor*)&pline[4]=TPairColor::orColorPair(a64,colorTable[4],colorTable[5]);
            *(TPairColor*)&pline[6]=TPairColor::orColorPair(a64,colorTable[6],colorTable[7]);
            
            colorTable+=kFrg_ClipWidth; //colorTable+=dst->width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        for (int y=0; y<dst->height; ++y) {
            pline[0]=colorTable[0] | alpha;
            pline[1]=colorTable[1] | alpha;
            pline[2]=colorTable[2] | alpha;
            pline[3]=colorTable[3] | alpha;
            pline[4]=colorTable[4] | alpha;
            pline[5]=colorTable[5] | alpha;
            pline[6]=colorTable[6] | alpha;
            pline[7]=colorTable[7] | alpha;
            
            colorTable+=kFrg_ClipWidth; //colorTable+=dst->width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}

void frg_copyPixels_32bit_directColor(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* alpha,TInt32 alpha_byte_width){
    if (dst->width==kFrg_ClipWidth){
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            pline[0]=colorTable[0] | (alpha[0]<<kFrg_outColor32_alpha_shl);
            pline[1]=colorTable[1] | (alpha[1]<<kFrg_outColor32_alpha_shl);
            pline[2]=colorTable[2] | (alpha[2]<<kFrg_outColor32_alpha_shl);
            pline[3]=colorTable[3] | (alpha[3]<<kFrg_outColor32_alpha_shl);
            pline[4]=colorTable[4] | (alpha[4]<<kFrg_outColor32_alpha_shl);
            pline[5]=colorTable[5] | (alpha[5]<<kFrg_outColor32_alpha_shl);
            pline[6]=colorTable[6] | (alpha[6]<<kFrg_outColor32_alpha_shl);
            pline[7]=colorTable[7] | (alpha[7]<<kFrg_outColor32_alpha_shl);
            
            alpha+=alpha_byte_width;
            colorTable+=kFrg_ClipWidth; //colorTable+=dst->width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }else{
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<dst->width; ++x){
                pline[x]=colorTable[x] | (alpha[x]<<kFrg_outColor32_alpha_shl);
            }
            alpha+=alpha_byte_width;
            colorTable+=dst->width;
            pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        }
    }
}


#endif //_IS_NEED_INLINE_FRG_DRAW_CODE

