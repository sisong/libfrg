//  frg_draw.h
//  for frg_reader
//
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
#ifndef _LIBFRG_frg_draw_h
#define _LIBFRG_frg_draw_h

#include "frg_private_reader_base.h"

#ifdef __cplusplus
extern "C" {
#endif
    
typedef TUInt32 TFastUInt;
struct frg_TPixelsRef;

FRG_READER_STATIC void frg_table_BGR24_to_32bit(void* pDstColor,const TByte* pBGR24,int colorCount);

FRG_READER_STATIC void frg_fillPixels_32bit(const struct frg_TPixelsRef* dst,const TByte* pBGRA32);
FRG_READER_STATIC void frg_fillPixels_32bit_withAlpha(const struct frg_TPixelsRef* dst,const TByte* pBGR24,const TByte* alphaLine,int alpha_byte_width);

FRG_READER_STATIC void frg_copyPixels_32bit_single_bgra_w8(const struct frg_TPixelsRef* dst,TUInt32 color32);
FRG_READER_STATIC void frg_copyPixels_32bit_single_bgr(const struct frg_TPixelsRef* dst,const TUInt32 color24,const TByte* alphaLine,int alpha_byte_width);
FRG_READER_STATIC void frg_copyPixels_32bit_index_single_a_w8_4bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha);
FRG_READER_STATIC void frg_copyPixels_32bit_index_4bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,int alpha_byte_width);
FRG_READER_STATIC void frg_copyPixels_32bit_index_single_a_w8_3bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha);
FRG_READER_STATIC void frg_copyPixels_32bit_index_3bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,int alpha_byte_width);
FRG_READER_STATIC void frg_copyPixels_32bit_index_single_a_w8_2bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha);
FRG_READER_STATIC void frg_copyPixels_32bit_index_2bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,int alpha_byte_width);
FRG_READER_STATIC void frg_copyPixels_32bit_index_single_a_w8_1bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha);
FRG_READER_STATIC void frg_copyPixels_32bit_index_1bit(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,int alpha_byte_width);
FRG_READER_STATIC void frg_copyPixels_32bit_match(const struct frg_TPixelsRef* dst,const TUInt32* src_pline,enum frg_TMatchType matchType,const TByte* alphaLine,int alpha_byte_width);
FRG_READER_STATIC void frg_copyPixels_32bit_directColor_single_a_w8(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,TUInt32 alpha);
FRG_READER_STATIC void frg_copyPixels_32bit_directColor(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* alphaLine,int alpha_byte_width);

//////

typedef void (*TProc_frg_copyPixels_32bit_index_single_a_xbit)(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,TUInt32 alpha);
typedef void (*TProc_frg_copyPixels_32bit_index_xbit)(const struct frg_TPixelsRef* dst,const TUInt32* colorTable,const TByte* index2List,const TByte* alphaLine,int alpha_byte_width);
    static const TProc_frg_copyPixels_32bit_index_single_a_xbit
      _frg_copyPixels_32bit_index_single_a_w8_xbit[4]=
    {
        frg_copyPixels_32bit_index_single_a_w8_1bit,
        frg_copyPixels_32bit_index_single_a_w8_2bit,
        frg_copyPixels_32bit_index_single_a_w8_3bit,
        frg_copyPixels_32bit_index_single_a_w8_4bit
    };
    static const TProc_frg_copyPixels_32bit_index_xbit
      _frg_copyPixels_32bit_index_xbit[4]=
    {
        frg_copyPixels_32bit_index_1bit,
        frg_copyPixels_32bit_index_2bit,
        frg_copyPixels_32bit_index_3bit,
        frg_copyPixels_32bit_index_4bit
    };

    
#define frg_copyPixels_32bit_index_single_a_w8_xbit(bit,dst,_colorTable,index2List,alpha) \
    _frg_copyPixels_32bit_index_single_a_w8_xbit[bit-1](dst,_colorTable,index2List,alpha)

#define frg_copyPixels_32bit_index_xbit(bit,dst,colorTable,index2List,alpha,alpha_byte_width) \
    _frg_copyPixels_32bit_index_xbit[bit-1](dst,colorTable,index2List,alpha,alpha_byte_width)


#ifdef __cplusplus
}
#endif


#endif
