//  frg_writer.cpp
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
#include "frg_writer.h"
#include "frg_private/frg_color_tools.h"
#include "frg_private/bytes_rle.h"
#include "frg_private/bgr_zip/frg_color_zip.h"
#include "../../lz4/lz4.h"
#include "../../lz4/lz4hc.h" //http://code.google.com/p/lz4/


namespace frg{
    
    static void frgZip_compress(std::vector<unsigned char>& out_code,
                                const unsigned char* src,const unsigned char* src_end){
        int oldSize=(int)out_code.size();
        out_code.resize(oldSize+LZ4_compressBound((int)(src_end-src)));
        //int codeSize=LZ4_compressHC((const char*)src,(char*)&out_code[oldSize],(int)(src_end-src));
        int codeSize=LZ4_compressHC2((const char*)src,(char*)&out_code[oldSize],(int)(src_end-src),16);
        assert(codeSize>0);
        out_code.resize(oldSize+codeSize);
    }
    
    static void getAlphasFromPixelsRef(std::vector<TByte>& out_alphaBuf,const TPixels32Ref& src){
        for (int y=0;y<src.height;++y){
            for (int x=0;x<src.width;++x){
                out_alphaBuf.push_back(src.pixels(x, y).a);
            }
        }
    }
    
    void TFrgParameter::setParameter(float _quality,float _compressSizeParameter){
        assert(_quality<=kFrg_quality_max);
        assert(_quality>=kFrg_quality_min);
        assert(_compressSizeParameter<=kFrg_size_maxUnZipSpeed);
        assert(_compressSizeParameter>=kFrg_size_minSize);
       
        isDeleteEmptyColor=true;
      
        quality=_quality;
        if (quality>kFrg_quality_max)
            quality=kFrg_quality_max;
        else if (quality<kFrg_quality_min)
            quality=kFrg_quality_min;
        
        float compressSizeParameter=_compressSizeParameter;
        if (compressSizeParameter>kFrg_size_maxUnZipSpeed)
            compressSizeParameter=kFrg_size_maxUnZipSpeed;
        else if (compressSizeParameter<kFrg_size_minSize)
            compressSizeParameter=kFrg_size_minSize;
        
        isMustFitColorTable=(quality<=kFrg_quality_default);
        
        //  0 -> ratio_min
        // 50 -> ratio_default
        //100 -> ratio_max
        const float ratio_min=0.98f;
        const float ratio_default=0.85f;
        const float ratio_max=0.5f;
        if (compressSizeParameter<=kFrg_size_default)
            compressRatio=(compressSizeParameter-kFrg_size_minSize)*((ratio_default-ratio_min)/(kFrg_size_default-kFrg_size_minSize))+ratio_min;
        else
            compressRatio=(compressSizeParameter-kFrg_size_default)*((ratio_max-ratio_default)/(kFrg_size_maxUnZipSpeed-kFrg_size_default))+ratio_default;
        
        isAlphaUseRleAdvance=compressSizeParameter<(kFrg_size_default+kFrg_size_minSize)*0.5f; //压得更小.
        const int rle_max_useZip=11;
        alphaRleAdvanceParameter=rle_max_useZip;
        alphaRleAdvanceCompressRatio=1-((1-compressRatio)*0.25f);
    }
    
    static inline bool tryRleCodeData(std::vector<TByte>& data_ziped,const std::vector<TByte>& data,int rleParameter,float compressRatio,int oldDataSize=-1){
        bytesRLE_save(data_ziped,&data[0],&data[0]+data.size(),rleParameter);
        if (oldDataSize<0) oldDataSize=(int)data.size();
        return (data_ziped.size()<=compressRatio*oldDataSize);
    }
    static inline bool tryCompressCodeData(std::vector<TByte>& data_ziped,const std::vector<TByte>& data,float compressRatio,int oldDataSize=-1){
        writeUInt32(data_ziped, (TUInt32)data.size());
        frgZip_compress(data_ziped,&data[0],&data[0]+data.size());
        if (oldDataSize<0) oldDataSize=(int)data.size();
        return (data_ziped.size()<=compressRatio*oldDataSize);
    }

void writeFrgImage(std::vector<TByte>& outFrgCode,const TFrgPixels32Ref& _srcImage,const TFrgParameter& parameter){
    assert(_srcImage.width>=0);
    assert(_srcImage.height>=0);
    TPixels32Ref srcImage;
    srcImage.pColor=(TBGRA32*)_srcImage.pColor;
    srcImage.width=_srcImage.width;
    srcImage.height=_srcImage.height;
    srcImage.byte_width=_srcImage.byte_width;
    const TPixels32Ref* _psrc=&srcImage;
    TPixels32Buffer _srcBuf;
    
    //预处理,删除全透明像素.
    if(parameter.isDeleteEmptyColor){
        //复制.
        _srcBuf.resizeFast(srcImage.width,srcImage.height);
        _psrc=&_srcBuf.getRef();
        pixelsCopy(*_psrc,srcImage);
        delEmptyColor(*_psrc);
    }
    const TPixels32Ref& src=*_psrc;
    
    //单色判断.
    TBGRA32 singleColor(0,0,0,0);
    bool isSingleBGR=getIsSigleRGBColor(src,&singleColor);
    bool isSingleAlpha=getIsSigleAlphaColor(src, &singleColor.a);
    
    int tempMemoryByteSize_max=0;
    int tempMemoryByteSize_cur=0;
    //处理alpha通道.
    std::vector<TByte> code_alpha;
    bool isAlphaDataUseBytesZip=false;
    bool isAlphaDataUseBytesRLE=false;
    if (isSingleAlpha){//isSingleAlpha
        tempMemoryByteSize_cur+=src.width;//for save min width alphas when frgFileRead, alpha's byte_width=0
        if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
    }else{
        std::vector<TByte> alphaBuf;
        getAlphasFromPixelsRef(alphaBuf,src);
        
        std::vector<TByte> zipCode;
        bool isAlphaDataUseBytesZip0=tryCompressCodeData(zipCode,alphaBuf,parameter.compressRatio);
        while (1) {
            if (parameter.isAlphaUseRleAdvance){
                std::vector<TByte> rleCode;
                bool isAlphaDataUseBytesRLE0=tryRleCodeData(rleCode,alphaBuf,parameter.alphaRleAdvanceParameter,parameter.alphaRleAdvanceCompressRatio);
                std::vector<TByte> rleCode_ziped;
                bool isAlphaDataUseBytesZip1=isAlphaDataUseBytesRLE0 && tryCompressCodeData(rleCode_ziped,rleCode,parameter.compressRatio,(int)alphaBuf.size());
                if (isAlphaDataUseBytesZip1 && (rleCode_ziped.size()<zipCode.size())){
                    isAlphaDataUseBytesRLE=true;
                    isAlphaDataUseBytesZip=true;
                    
                    tempMemoryByteSize_cur+=(int)alphaBuf.size(); //for save all alphas when frgFileRead
                    if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
                    tempMemoryByteSize_cur+=(int)rleCode.size();//for save unzip relCode when frgFileRead
                    if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
                    tempMemoryByteSize_cur-=(int)rleCode.size();
                    
                    code_alpha.insert(code_alpha.end(),rleCode_ziped.begin(),rleCode_ziped.end());
                    break;//ok  同时使用rle和数据压缩.
                }//else ->
            }//else
            {
                isAlphaDataUseBytesRLE=false;
                isAlphaDataUseBytesZip=isAlphaDataUseBytesZip0;
                if (isAlphaDataUseBytesZip){
                    tempMemoryByteSize_cur+=(int)alphaBuf.size(); //for save all alphas when frgFileRead
                    if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
                    code_alpha.insert(code_alpha.end(),zipCode.begin(),zipCode.end());//使用数据压缩.
                }else{
                    code_alpha.insert(code_alpha.end(),alphaBuf.begin(),alphaBuf.end());//不压缩.
                }
                break;
            }
        }
    }
    
    //处理RGB通道.
    std::vector<TByte> code_bgr;
    bool isRGBDataUseBytesZip=false;
    if (!isSingleBGR){        
        int tempMemoryByteSize_bgrZip=0;
        TColorZiper::saveTo(code_bgr, src,parameter.quality,parameter.isMustFitColorTable,&tempMemoryByteSize_bgrZip);
        tempMemoryByteSize_cur+=tempMemoryByteSize_bgrZip;//for load BGRColor from code_bgr when frgFileRead
        if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
       
        
        std::vector<TByte> color_bgr_ziped;
        isRGBDataUseBytesZip=tryCompressCodeData(color_bgr_ziped,code_bgr,parameter.compressRatio);
        if (isRGBDataUseBytesZip){
            tempMemoryByteSize_cur+=(int)code_bgr.size(); //for save code_bgr when frgFileRead
            if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
            code_bgr.swap(color_bgr_ziped);
        }
    }
    
    //head code
    std::vector<TByte> code_head;
    {
        writeUInt32(code_head, src.width);
        writeUInt32(code_head, src.height);
        writeUInt32(code_head, tempMemoryByteSize_max);
        code_head.push_back(kEncodingFormat_stream);
        code_head.push_back(kSavedColorFormat_A8R8G8B8);
        
        TByte colorInfo=0;
        colorInfo|=isSingleAlpha?kColorInfo_isSingleAlpha:0;
        colorInfo|=isSingleBGR?kColorInfo_isSingleBGR:0;
        colorInfo|=isAlphaDataUseBytesZip?kColorInfo_isAlphaDataUseBytesZip:0;
        colorInfo|=isRGBDataUseBytesZip?kColorInfo_isRGBDataUseBytesZip:0;
        colorInfo|=(!isAlphaDataUseBytesRLE)?kColorInfo_isAlphaDataNotUseBytesRLE:0;
        
        code_head.push_back(colorInfo);
        code_head.push_back(0);//_reserved
        code_head.push_back(singleColor.b);
        code_head.push_back(singleColor.g);
        code_head.push_back(singleColor.r);
        code_head.push_back(singleColor.a);
    }
   
    //out
    //type+version
    std::vector<TByte>& code_data=outFrgCode;
    code_data.assign(kFrgTagAndVersion, kFrgTagAndVersion+kFrgTagAndVersionSize);
    //imageFileSize
    TUInt32 imageFileSize=sizeof(TUInt32)+(TUInt32)code_data.size();
    imageFileSize+=(TUInt32)code_head.size()+sizeof(TUInt32);
    if (!isSingleAlpha)
        imageFileSize+=(TUInt32)code_alpha.size()+sizeof(TUInt32);
    if (!isSingleBGR)
        imageFileSize+=(TUInt32)code_bgr.size()+sizeof(TUInt32);
    writeUInt32(code_data,imageFileSize);
    
    //head code
    writeUInt32(code_data,(TUInt32)code_head.size());
    code_data.insert(code_data.end(), code_head.begin(),code_head.end());
    
    //alpha code
    if (!isSingleAlpha){
        writeUInt32(code_data,(TUInt32)code_alpha.size());
        code_data.insert(code_data.end(), code_alpha.begin(),code_alpha.end());
    }
    //BGR code
    if (!isSingleBGR){
        writeUInt32(code_data,(TUInt32)code_bgr.size());
        code_data.insert(code_data.end(), code_bgr.begin(),code_bgr.end());
    }
}

}//end namespace frg
