//  frg_reader.c
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
#include "../../lz4/lz4.h"//http://code.google.com/p/lz4/
#include "frg_draw.h"
#ifdef _IS_NEED_INLINE_FRG_DRAW_CODE
    #undef  _IS_NEED_INLINE_FRG_DRAW_CODE
        #include "frg_draw.c"
    #define _IS_NEED_INLINE_FRG_DRAW_CODE
#endif

#ifdef FRG_READER_RUN_MEM_SAFE_CHECK
#   define __RUN_MEM_SAFE_CHECK
#endif

#define loadUInt32(pcode) ((TUInt32)(   \
                              (TUInt32)(((const TByte*)(pcode))[0])     \
                            | (TUInt32)(((const TByte*)(pcode))[1]<<8)  \
                            | (TUInt32)(((const TByte*)(pcode))[2]<<16) \
                            | (TUInt32)(((const TByte*)(pcode))[3]<<24) ))
#define readUInt32(ppcode) loadUInt32(*(ppcode)); { *(ppcode)+=4; }
#define toAlign4(pointer) (  (const TByte*)0+( ((const TByte*)pointer-(const TByte*)0+3)&(~(size_t)3) )  )

FRG_READER_STATIC frg_BOOL _bytesRle_load(TByte* out_data,TByte* out_dataEnd,const TByte* rle_code,const TByte* rle_code_end);//rle解码.
FRG_READER_STATIC frg_BOOL _colorUnZiper_loadColor(const struct frg_TPixelsRef* dst_image,const TByte* code,const TByte* code_end,const TByte* alpha,int alpha_byte_width,TByte* tempMemory,TByte* tempMemory_end);

static /* inline */ frg_BOOL frgZip_decompress(unsigned char* out_data,unsigned char* out_data_end,
                              const unsigned char* frgZip_code,const unsigned char* frgZip_code_end){
    #ifdef __RUN_MEM_SAFE_CHECK
        return 0<=LZ4_decompress_safe((const char*)frgZip_code,(char*)out_data,(int)(frgZip_code_end-frgZip_code),(int)(out_data_end-out_data));
    #else
        return 0<=LZ4_decompress_fast((const char*)frgZip_code,(char*)out_data,(int)(out_data_end-out_data));
    #endif
}

///////

int FRG_READER_EXPORT_API getFrgHeadSize(void){
    return kFrgFileHeadSize;
}

#ifdef __RUN_MEM_SAFE_CHECK
#   define kSafeColorTable_extendMemSize ((unsigned int)(kFrg_MaxSubTableSize*kFrg_outColor_size))
#endif

frg_BOOL FRG_READER_EXPORT_API readFrgImageInfo(const TByte* frgCode_begin,const TByte* frgCode_end,struct frg_TFrgImageInfo* out_frgImageInfo){
    const struct TFrgFileHead* fhead=(const struct TFrgFileHead*)frgCode_begin;
    struct frg_TFrgImageInfo tmp_frgImageInfo;
    if (out_frgImageInfo==0) out_frgImageInfo=&tmp_frgImageInfo;
    
    assert(frgCode_end>=frgCode_begin);
    if (!(frgCode_end>=frgCode_begin)) return frg_FALSE;
    if ((TUInt)kFrgFileHeadSize>(TUInt)(frgCode_end-frgCode_begin)) return frg_FALSE;
    if ( loadUInt32(&fhead->frgTagAndVersion[0])!=loadUInt32(&kFrgTagAndVersion[0]) )
        return frg_FALSE;
    
    out_frgImageInfo->imageFileSize=loadUInt32(&fhead->imageFileSize);
    if(out_frgImageInfo->imageFileSize<(TUInt32)kFrgFileHeadSize) return frg_FALSE;
    out_frgImageInfo->imageWidth=loadUInt32(&fhead->headInfo.imageWidth);
    if(out_frgImageInfo->imageWidth<0) return frg_FALSE;
    out_frgImageInfo->imageHeight=loadUInt32(&fhead->headInfo.imageHeight);
    if(out_frgImageInfo->imageHeight<0) return frg_FALSE;
    out_frgImageInfo->decoder_tempMemoryByteSize=loadUInt32(&fhead->headInfo.decoder_tempMemoryByteSize);
#ifdef __RUN_MEM_SAFE_CHECK
    unsigned int tempMemoryByteSize_safe=out_frgImageInfo->decoder_tempMemoryByteSize+kSafeColorTable_extendMemSize;
    if (tempMemoryByteSize_safe < out_frgImageInfo->decoder_tempMemoryByteSize) return frg_FALSE;
    out_frgImageInfo->decoder_tempMemoryByteSize=tempMemoryByteSize_safe;
#endif
    //特殊实现的解码器可以对decoder_tempMemoryByteSize送回0,并自己申请解码需要的内存;或者返回自己需要的内存(较困难).
    
    return frg_TRUE;
}

////

frg_BOOL FRG_READER_EXPORT_API readFrgImage(const TByte* frgCode_begin,const TByte* frgCode_end,const struct frg_TPixelsRef* dst_image,TByte* tempMemory,TByte* tempMemory_end){
    const struct TFrgFileHead* fhead=(const struct TFrgFileHead*)frgCode_begin;
    {   //head check
        struct frg_TFrgImageInfo frgInfo;
        assert(kFrg_outColor_size==sizeof(TUInt32));
        assert(tempMemory_end>=tempMemory);
        if (!(tempMemory_end>=tempMemory)) return frg_FALSE;
        if (dst_image->colorType!=kFrg_ColorType_32bit_A8R8G8B8) return frg_FALSE;
        if ((dst_image->width<0)||(dst_image->height<0)) return frg_FALSE;
        if (!readFrgImageInfo(frgCode_begin,frgCode_end,&frgInfo))
            return frg_FALSE;
        if ((TUInt)frgInfo.imageFileSize!=(TUInt)(frgCode_end-frgCode_begin)) return frg_FALSE;
        if ((frgInfo.imageWidth!=dst_image->width)||(frgInfo.imageHeight!=dst_image->height)
            ||((TUInt)frgInfo.decoder_tempMemoryByteSize>((TUInt)(tempMemory_end-tempMemory)))) return frg_FALSE;
        
        if ((dst_image->width==0)||(dst_image->height==0)) return frg_TRUE;
        
        if (fhead->headInfo.encodingFormat!=kEncodingFormat_stream) return frg_FALSE;
        if (fhead->headInfo.savedColorFormat!=kSavedColorFormat_A8R8G8B8) return frg_FALSE;
    }
    {
        const TUInt32 _headInfoCodeSize=loadUInt32(&fhead->headInfo.headInfoCodeSize);
        const TByte* frgCodeData=frgCode_begin+(sizeof(struct TFrgFileHead)-sizeof(struct TFrgHeadInfo))
                                +sizeof(_headInfoCodeSize)+_headInfoCodeSize;//skip head code
        
        const TByte _colorInfo=fhead->headInfo.colorInfo;
        const frg_BOOL isSingleAlpha=(_colorInfo&kColorInfo_isSingleAlpha)!=0;
        const frg_BOOL isSingleBGR=(_colorInfo&kColorInfo_isSingleBGR)!=0;
        const frg_BOOL isAlphaDataUseBytesZip=(_colorInfo&kColorInfo_isAlphaDataUseBytesZip)!=0;
        const frg_BOOL isRGBDataUseBytesZip=(_colorInfo&kColorInfo_isRGBDataUseBytesZip)!=0;
        const frg_BOOL isAlphaDataUseRLE=(_colorInfo&kColorInfo_isAlphaDataNotUseBytesRLE)==0;
        const TByte* singleBGRA=&(fhead->headInfo.singleColor_b); //b,g,r,a
        
        const TByte* alphaBuf=0;
        int alpha_byte_width=0;
        
        //single color
        if (isSingleAlpha&&isSingleBGR){
            frg_fillPixels_32bit(dst_image,singleBGRA);
            return frg_TRUE;
        }
        
        //read alpha
        if (isSingleAlpha){
            const TUInt alphaBufSize=dst_image->width;
            //alpha_byte_width=0;
            if (alphaBufSize>(TUInt)(tempMemory_end-tempMemory)) return frg_FALSE;
            alphaBuf=tempMemory; tempMemory+=alphaBufSize;
            memset((TByte*)alphaBuf,singleBGRA[3],alphaBufSize);
        }else{
            const TByte *code,*code_end;
            TUInt alphaBufSize,codeSize;
            
            if (4>(TUInt)(frgCode_end-frgCodeData)) return frg_FALSE;
            codeSize=readUInt32(&frgCodeData);
            if (codeSize>(TUInt)(frgCode_end-frgCodeData)) return frg_FALSE;
            code=frgCodeData;
            frgCodeData+=codeSize;
            code_end=frgCodeData;
            
            alpha_byte_width=dst_image->width;
            alphaBufSize=(TUInt)alpha_byte_width*(TUInt)dst_image->height;
            if ( (sizeof(TUInt)<sizeof(int)*2) && (alphaBufSize/(TUInt)alpha_byte_width!=(TUInt)dst_image->height) ) return frg_FALSE;
            if((!isAlphaDataUseRLE)&&(!isAlphaDataUseBytesZip)){
                if (codeSize!=alphaBufSize) return frg_FALSE;
                alphaBuf=code; //没有任何压缩.
            }else{
                TByte* _tempMemory_back;
                if (alphaBufSize>(TUInt)(tempMemory_end-tempMemory)) return frg_FALSE;
                alphaBuf=tempMemory; tempMemory+=alphaBufSize;
                
                _tempMemory_back=tempMemory;
                if (isAlphaDataUseBytesZip){
                    TUInt alpha_code_size;
                    TByte* _alpha_code_buf=0;
                    if (codeSize<4) return frg_FALSE;
                    alpha_code_size=readUInt32(&code);
                    if (isAlphaDataUseRLE){
                        if (alpha_code_size>(TUInt)(tempMemory_end-tempMemory)) return frg_FALSE;
                        _alpha_code_buf=tempMemory; tempMemory+=alpha_code_size;
                    }else{
                        if (alpha_code_size!=alphaBufSize) return frg_FALSE;
                        _alpha_code_buf=(TByte*)alphaBuf;
                    }
                    if (!frgZip_decompress(_alpha_code_buf,_alpha_code_buf+alpha_code_size,code,code_end))
                        return frg_FALSE;
                    code=_alpha_code_buf;
                    code_end=_alpha_code_buf+alpha_code_size;
                }
                if (isAlphaDataUseRLE) {
                    if (!_bytesRle_load((TByte*)alphaBuf,(TByte*)alphaBuf+alphaBufSize,code,code_end))
                        return frg_FALSE;
                }
                tempMemory=_tempMemory_back;
            }
        }
        
        //read bgr
        if (isSingleBGR) {
            frg_fillPixels_32bit_withAlpha(dst_image,singleBGRA,alphaBuf,alpha_byte_width);
        }else{
            const TByte *code,*code_end;
            TUInt codeSize;
            if (4>(TUInt)(frgCode_end-frgCodeData)) return frg_FALSE;
            codeSize=readUInt32(&frgCodeData);
            if (codeSize>(TUInt)(frgCode_end-frgCodeData)) return frg_FALSE;
            code=frgCodeData;
            frgCodeData+=codeSize;
            code_end=frgCodeData;
            
            //TByte* _tempMemory_back=tempMemory;
            if (isRGBDataUseBytesZip){
                TUInt bgr_code_size;
                TByte* _bgr_code_buf;
                
                if (codeSize<4) return frg_FALSE;
                bgr_code_size=readUInt32(&code);
                if (bgr_code_size>(TUInt)(tempMemory_end-tempMemory)) return frg_FALSE;
                _bgr_code_buf=tempMemory; tempMemory+=bgr_code_size;
                if (!frgZip_decompress(_bgr_code_buf,_bgr_code_buf+bgr_code_size,code,code_end))
                    return frg_FALSE;
                code=_bgr_code_buf;
                code_end=_bgr_code_buf+bgr_code_size;
            }
            if (!_colorUnZiper_loadColor(dst_image,code,code_end,alphaBuf,alpha_byte_width,tempMemory,tempMemory_end))
                return frg_FALSE;
            //tempMemory=_tempMemory_back;
        }
    }
    return frg_TRUE;
}

/////////

//变长正整数编码方案(x bit额外类型标志位,x<=7),从高位开始输出1--n byte:
// x0*  7-x bit
// x1* 0*  7+7-x bit
// x1* 1* 0*  7+7+7-x bit
// x1* 1* 1* 0*  7+7+7+7-x bit
// x1* 1* 1* 1* 0*  7+7+7+7+7-x bit
// ......
static TUInt unpackUIntWithTag(const TByte** src_code,const TByte* src_code_end,const int kTagBit){//读出整数并前进指针.
#ifdef __RUN_MEM_SAFE_CHECK
    const int kPackMaxTagBit=7;
#endif
    TUInt           value;
    TByte           code;
    const TByte*    pcode=*src_code;
    
#ifdef __RUN_MEM_SAFE_CHECK
    assert((0<=kTagBit)&&(kTagBit<=kPackMaxTagBit));
    if (src_code_end-pcode<=0) return 0;
#endif
    code=*pcode; ++pcode;
    value=code&((1<<(7-kTagBit))-1);
    if ((code&(1<<(7-kTagBit)))!=0){
        do {
#ifdef __RUN_MEM_SAFE_CHECK
            assert((value>>(sizeof(value)*8-7))==0);
            if (src_code_end==pcode) break;
#endif
            code=*pcode; ++pcode;
            value=(value<<7) | (code&((1<<7)-1));
        } while ((code&(1<<7))!=0);
    }
    (*src_code)=pcode;
    return value;
}

#define unpackUInt(src_code,src_code_end) unpackUIntWithTag(src_code,src_code_end,0)

frg_BOOL _colorUnZiper_loadColor(const struct frg_TPixelsRef* dst_image,const TByte* code,const TByte* code_end,const TByte* alpha,int alpha_byte_width,TByte* tempMemory,TByte* tempMemory_end){
    const int nodeWidth=((TUInt32)(dst_image->width-1))/kFrg_ClipWidth+1;
    const int nodeHeight=((TUInt32)(dst_image->height-1))/kFrg_ClipHeight+1;
    const TByte *nodeInfoList,*nodeInfoList_end,*indexCodeList,*indexCodeList_end,*matchXYList,*matchXYList_end;
    const TUInt32 *colorTable,*colorTable_end;
    int lastNodeWidth,lastNodeHeight;
    int ny;
    struct frg_TPixelsRef subRef;
#ifdef __RUN_MEM_SAFE_CHECK
    const TUInt32* _colorTable0_back;
#endif
    
    {
        const TByte* color24Table;
        const TUInt nodeCount=unpackUInt(&code,code_end);
        const TUInt tableSize=unpackUInt(&code,code_end);
        const TUInt indexCodeSize=unpackUInt(&code,code_end);
        const TUInt matchCount=unpackUInt(&code,code_end);
        if (nodeCount!=(TUInt)nodeWidth*(TUInt)nodeHeight) return frg_FALSE;
        if( (sizeof(TUInt)<sizeof(int)*2) && (nodeCount/(TUInt)nodeWidth!=(TUInt)nodeHeight) ) return frg_FALSE;
        
#ifdef __RUN_MEM_SAFE_CHECK
        if (nodeCount>(TUInt)(code_end-code)) return frg_FALSE;
#endif
        nodeInfoList=code;   code+=nodeCount;
        nodeInfoList_end=code;
#ifdef __RUN_MEM_SAFE_CHECK
        if ( ((tableSize>>(sizeof(TUInt)*8-2))!=0) || (tableSize*3>(TUInt)(code_end-code)) ) return frg_FALSE;
#endif
        color24Table=code;   code+=tableSize*3;
        
#ifdef __RUN_MEM_SAFE_CHECK
        if (indexCodeSize>(TUInt)(code_end-code)) return frg_FALSE;
#endif
        indexCodeList=code; code+=indexCodeSize;
        indexCodeList_end=code;
        
#ifdef __RUN_MEM_SAFE_CHECK
        if ( ((matchCount>>(sizeof(TUInt)*8-2))!=0) || (matchCount*4>(TUInt)(code_end-code)) ) return frg_FALSE;
#endif
        matchXYList=code; code+=matchCount*4;
        matchXYList_end=code;
        if (code!=code_end) return frg_FALSE;
        
        //colorTable
        if ((TUInt)(tempMemory_end-tempMemory)<(TUInt)(tableSize*4+3)) return frg_FALSE;
        colorTable=(const TUInt32*)toAlign4(tempMemory);//tempMemory+= tableSize*4+3;
        colorTable_end=colorTable+tableSize;
        frg_table_BGR24_to_32bit((TUInt32*)colorTable,color24Table,tableSize);
#ifdef __RUN_MEM_SAFE_CHECK
        _colorTable0_back=colorTable;
        if (kSafeColorTable_extendMemSize>(TUInt)(tempMemory_end-tempMemory-(tableSize*4+3))) return frg_FALSE;
        memset((void*)&colorTable[tableSize],0,kSafeColorTable_extendMemSize);
#endif
    }
    
    lastNodeWidth=((TUInt32)(dst_image->width-1))%kFrg_ClipWidth+1;
    lastNodeHeight=((TUInt32)(dst_image->height-1))%kFrg_ClipHeight+1;
    
    subRef.byte_width=dst_image->byte_width;
    subRef.colorType=dst_image->colorType;
    subRef.height=kFrg_ClipHeight;
    for (ny=0; ny<nodeHeight;++ny,alpha+=alpha_byte_width*kFrg_ClipHeight){
        int nx;
        const TByte* curAlpha=alpha;
        subRef.pColor=(TByte*)dst_image->pColor+ny*kFrg_ClipHeight*dst_image->byte_width;
        subRef.width=kFrg_ClipWidth;
        if (ny==nodeHeight-1) subRef.height=lastNodeHeight;
        for (nx=nodeWidth-1; nx>=0;--nx,curAlpha+=kFrg_ClipWidth,subRef.pColor=(TUInt32*)( ((TByte*)subRef.pColor)+kFrg_ClipWidth*kFrg_outColor_size )){
            TByte nodeInfo=*nodeInfoList; ++nodeInfoList;
            if (nx==0) subRef.width=lastNodeWidth;
            //assert(nodeInfoList<=nodeInfoList_end); //safe
            switch ((enum frg_TClipType)(nodeInfo>>4)) { //type
                case kFrg_ClipType_index_single_a_w8:{
                    TUInt tableLength=1+( nodeInfo&15 );
                    const int bit=kFrg_SubTableSize_to_indexBit[tableLength];
#ifdef __RUN_MEM_SAFE_CHECK
                    if (tableLength>(TUInt)(colorTable_end-colorTable)) return frg_FALSE;
                    if ((TUInt)(subRef.height*bit)>(TUInt)(indexCodeList_end-indexCodeList)) return frg_FALSE;
#endif
                    frg_copyPixels_32bit_index_single_a_w8_xbit(bit,&subRef,colorTable,indexCodeList,*curAlpha);
                    colorTable+=tableLength;
                    indexCodeList+=subRef.height*bit;//*kFrg_ClipWidth/8
                } break;
                case kFrg_ClipType_index:{
                    TUInt tableLength=1+( nodeInfo&15 );
                    const int bit=kFrg_SubTableSize_to_indexBit[tableLength];
#ifdef __RUN_MEM_SAFE_CHECK
                    if (tableLength>(TUInt)(colorTable_end-colorTable)) return frg_FALSE;
                    if ((TUInt)((subRef.width*subRef.height*bit+7)>>3)>(TUInt)(indexCodeList_end-indexCodeList)) return frg_FALSE;
#endif
                    frg_copyPixels_32bit_index_xbit(bit,&subRef,colorTable,indexCodeList,curAlpha,alpha_byte_width);
                    colorTable+=tableLength;
                    indexCodeList+=(subRef.width*subRef.height*bit+7)>>3;
                } break;
                case kFrg_ClipType_single_bgra_w8:{
                    TUInt tableForwardLength=nodeInfo&15;
#ifdef __RUN_MEM_SAFE_CHECK
                    if (tableForwardLength>(TUInt)(colorTable-_colorTable0_back)) return frg_FALSE;
                    //kSafeColorTable_extendMemSize已经避免了损坏的tableForwardLength使colorTable-tableForwardLength+(1<<4)>colorTable_end而造成的越界.
#endif
                    frg_copyPixels_32bit_single_bgra_w8(&subRef,*(colorTable-tableForwardLength) | ((*curAlpha)<<kFrg_outColor_alpha_shl));
                    if (tableForwardLength==0){
#ifdef __RUN_MEM_SAFE_CHECK
                        if (colorTable>=colorTable_end) return frg_FALSE;
#endif
                        ++colorTable;
                    }
                } break;
                case kFrg_ClipType_single_bgr:{
                    TUInt tableForwardLength=nodeInfo&15;
#ifdef __RUN_MEM_SAFE_CHECK
                    if (tableForwardLength>(TUInt)(colorTable-_colorTable0_back)) return frg_FALSE;
#endif
                    frg_copyPixels_32bit_single_bgr(&subRef,*(colorTable-tableForwardLength),curAlpha,alpha_byte_width);
                    if (tableForwardLength==0){
#ifdef __RUN_MEM_SAFE_CHECK
                        if (colorTable>=colorTable_end) return frg_FALSE;
#endif
                        ++colorTable;
                    }
                } break;
                case kFrg_ClipType_match_table_single_a_w8:{
                    const int bit=(nodeInfo&3)+1;
                    TUInt tableForwardLength=unpackUInt(&indexCodeList,indexCodeList_end);
#ifdef __RUN_MEM_SAFE_CHECK
                    if (tableForwardLength>(TUInt)(colorTable-_colorTable0_back)) return frg_FALSE;
                    if ((TUInt32)(subRef.height*bit)>(TUInt)(indexCodeList_end-indexCodeList)) return frg_FALSE;
#endif
                    frg_copyPixels_32bit_index_single_a_w8_xbit(bit,&subRef,colorTable-tableForwardLength,indexCodeList,*curAlpha);
                    indexCodeList+=subRef.height*bit;//*kFrg_ClipWidth/8
                } break;
                case kFrg_ClipType_match_table:{
                    const int bit=(nodeInfo&3)+1;
                    TUInt tableForwardLength=unpackUInt(&indexCodeList,indexCodeList_end);
#ifdef __RUN_MEM_SAFE_CHECK
                    if (tableForwardLength>(TUInt)(colorTable-_colorTable0_back)) return frg_FALSE;
                    if ((TUInt)((subRef.width*subRef.height*bit+7)>>3)>(TUInt)(indexCodeList_end-indexCodeList)) return frg_FALSE;
#endif
                    frg_copyPixels_32bit_index_xbit(bit,&subRef,colorTable-tableForwardLength,indexCodeList,curAlpha,alpha_byte_width);
                    indexCodeList+=(subRef.width*subRef.height*bit+7)>>3;
                } break;
                case kFrg_ClipType_match_image:{
                    enum frg_TMatchType matchType=(enum frg_TMatchType)(nodeInfo&((1<<3)-1)); //3bit
#ifdef __RUN_MEM_SAFE_CHECK
                    if (4>(TUInt)(matchXYList_end-matchXYList)) return frg_FALSE;
#endif
                    const TUInt32* pXYColor_matched=(const TUInt32*)( (matchXYList[0]|(matchXYList[1]<<8))*sizeof(TUInt32)
                                                +(matchXYList[2]|(matchXYList[3]<<8))*dst_image->byte_width
                                                +((TByte*)dst_image->pColor) );
                    frg_copyPixels_32bit_match(&subRef,pXYColor_matched,matchType,curAlpha,alpha_byte_width);
                    matchXYList+=4;
                } break;
                case kFrg_ClipType_directColor:{
#ifdef __RUN_MEM_SAFE_CHECK
                    if ((TUInt)((TUInt)subRef.width*subRef.height)>(TUInt)(colorTable_end-colorTable)) return frg_FALSE;
#endif
                    if (nodeInfo&1)
                        frg_copyPixels_32bit_directColor_single_a_w8(&subRef,colorTable,(*curAlpha));
                    else
                        frg_copyPixels_32bit_directColor(&subRef,colorTable,curAlpha,alpha_byte_width);
                    colorTable+=subRef.width*subRef.height;
                } break;
            }//end case
        }//for nx
    }//for ny
    return (nodeInfoList==nodeInfoList_end)&&(colorTable==colorTable_end)&&(indexCodeList==indexCodeList_end)&&(matchXYList==matchXYList_end);
}

/////////////

frg_BOOL _bytesRle_load(TByte* out_data,TByte* out_dataEnd,const TByte* rle_code,const TByte* rle_code_end){
    const TByte*    ctrlBuf,*ctrlBuf_end;
    
    TUInt ctrlSize= unpackUInt(&rle_code,rle_code_end);
#ifdef __RUN_MEM_SAFE_CHECK
    if (ctrlSize>(TUInt)(rle_code_end-rle_code)) return frg_FALSE;
#endif
    ctrlBuf=rle_code;
    rle_code+=ctrlSize;
    ctrlBuf_end=rle_code;
    while (ctrlBuf_end-ctrlBuf>0){
        enum TByteRleType type=(enum TByteRleType)((*ctrlBuf)>>(8-kByteRleType_bit));
        TUInt length= 1 + unpackUIntWithTag(&ctrlBuf,ctrlBuf_end,kByteRleType_bit);
#ifdef __RUN_MEM_SAFE_CHECK
        if (length>(TUInt)(out_dataEnd-out_data)) return frg_FALSE;
#endif
        switch (type){
            case kByteRleType_rle0:{
                memset(out_data,0,length);
                out_data+=length;
            }break;
            case kByteRleType_rle255:{
                memset(out_data,255,length);
                out_data+=length;
            }break;
            case kByteRleType_rle:{
#ifdef __RUN_MEM_SAFE_CHECK
                if (1>(TUInt)(rle_code_end-rle_code)) return frg_FALSE;
#endif
                memset(out_data,*rle_code,length);
                ++rle_code;
                out_data+=length;
            }break;
            case kByteRleType_unrle:{
#ifdef __RUN_MEM_SAFE_CHECK
                if (length>(TUInt)(rle_code_end-rle_code)) return frg_FALSE;
#endif
                memcpy(out_data,rle_code,length);
                rle_code+=length;
                out_data+=length;
            }break;
        }
    }
    return (ctrlBuf==ctrlBuf_end)&&(rle_code==rle_code_end)&&(out_data==out_dataEnd);
}

