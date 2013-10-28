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

enum { kBGRMask=~(((TUInt32)0xFF)<<kFrg_outColor32_alpha_shl) };

void frg_table_BGR24_to_32bit(void* pDstColor,const TByte* pBGR24,int colorCount){
    TUInt32* pline=(TUInt32*)pDstColor;
    const int fast_width=colorCount&(~3);
    for (int x=0; x<fast_width; x+=4,pBGR24+=3*4) {
        pline[x  ]=(pBGR24[ 0]<<kFrg_outColor32_blue_shl)|(pBGR24[ 1]<<kFrg_outColor32_green_shl)|(pBGR24[ 2]<<kFrg_outColor32_red_shl);
        pline[x+1]=(pBGR24[ 3]<<kFrg_outColor32_blue_shl)|(pBGR24[ 4]<<kFrg_outColor32_green_shl)|(pBGR24[ 5]<<kFrg_outColor32_red_shl);
        pline[x+2]=(pBGR24[ 6]<<kFrg_outColor32_blue_shl)|(pBGR24[ 7]<<kFrg_outColor32_green_shl)|(pBGR24[ 8]<<kFrg_outColor32_red_shl);
        pline[x+3]=(pBGR24[ 9]<<kFrg_outColor32_blue_shl)|(pBGR24[10]<<kFrg_outColor32_green_shl)|(pBGR24[11]<<kFrg_outColor32_red_shl);
    }
    for (int x=fast_width; x<colorCount; ++x,pBGR24+=3) {
        pline[x]=(pBGR24[0]<<kFrg_outColor32_blue_shl)|(pBGR24[1]<<kFrg_outColor32_green_shl)|(pBGR24[2]<<kFrg_outColor32_red_shl);
    }
}

////


void frg_fillPixels_32bit(const struct frg_TPixelsRef* dst,const TByte* pBGRA32){
    TUInt32 color32=  (pBGRA32[0]<<kFrg_outColor32_blue_shl)|(pBGRA32[1]<<kFrg_outColor32_green_shl)
                    | (pBGRA32[2]<<kFrg_outColor32_red_shl)|(pBGRA32[3]<<kFrg_outColor32_alpha_shl);
    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    const int fast_width=width&(~7);
    for (int y=0; y<dst->height; ++y) {
        for (int x=0; x<fast_width; x+=8) {
            pline[x  ]=color32;
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

void frg_fillPixels_32bit_withAlpha(const struct frg_TPixelsRef* dst,const TByte* pBGR24,const TByte* alpha,int alpha_byte_width){
    TUInt32 color24=(pBGR24[0]<<kFrg_outColor32_blue_shl)|(pBGR24[1]<<kFrg_outColor32_green_shl)|(pBGR24[2]<<kFrg_outColor32_red_shl);

    const int width=dst->width;
    const int byte_width=dst->byte_width;
    TUInt32* pline=(TUInt32*)dst->pColor;
    const int fast_width=width&(~3);
    for (int y=0; y<dst->height; ++y) {
        for (int x=0; x<fast_width; x+=4) {
            pline[x  ]=color24 | (alpha[x  ]<<kFrg_outColor32_alpha_shl);
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

//

void frg_copyPixels_32bit_single_bgra_w8(const struct frg_TPixelsRef* dst,TUInt32 color24,TUInt32 alpha){
    TUInt32 color32=color24 | (alpha<<kFrg_outColor32_alpha_shl);
    TUInt32* pline=(TUInt32*)dst->pColor;
    TInt32 byte_width=dst->byte_width;
    for (int y=0; y<dst->height; ++y) {
        pline[0]=color32;
        pline[1]=color32;
        pline[2]=color32;
        pline[3]=color32;
        pline[4]=color32;
        pline[5]=color32;
        pline[6]=color32;
        pline[7]=color32;        
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
    }
}

void frg_copyPixels_32bit_single_bgr(const struct frg_TPixelsRef* dst,const TUInt32 color24,const TByte* alpha,int alpha_byte_width){
    if (dst->width==kFrg_ClipWidth){
        TInt32 byte_width=dst->byte_width;
        TUInt32* pline=(TUInt32*)dst->pColor;
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
    for (int y=0; y<dst->height; ++y) {
        int index01=index2List[0];
        pline[0]=colorTable[index01&15] | alpha; 
        pline[1]=colorTable[index01>>4] | alpha; 
        int index23=index2List[1];
        pline[2]=colorTable[index23&15] | alpha; 
        pline[3]=colorTable[index23>>4] | alpha; 
        int index45=index2List[2];
        pline[4]=colorTable[index45&15] | alpha; 
        pline[5]=colorTable[index45>>4] | alpha; 
        int index67=index2List[3];
        pline[6]=colorTable[index67&15] | alpha; 
        pline[7]=colorTable[index67>>4] | alpha; 
        
        index2List+=4;
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
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
        int indexs=index2List[0];
        pline[0]=colorTable[ indexs    &1];
        pline[1]=colorTable[(indexs>>1)&1];
        pline[2]=colorTable[(indexs>>2)&1];
        pline[3]=colorTable[(indexs>>3)&1];
        pline[4]=colorTable[(indexs>>4)&1];
        pline[5]=colorTable[(indexs>>5)&1];
        pline[6]=colorTable[(indexs>>6)&1];
        pline[7]=colorTable[(indexs>>7)&1];
        
        pline=(TUInt32*)( ((TByte*)pline)+byte_width );
        index2List+=1;
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

