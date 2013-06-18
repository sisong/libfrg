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
#include "FRZ1_compress.h"
#include "frg_private/bgr_zip/frg_color_zip.h"

namespace frg{
        
    static void getAlphasFromPixelsRef(std::vector<TByte>& out_alphaBuf,const TPixels32Ref& src){
        for (int y=0;y<src.height;++y){
            for (int x=0;x<src.width;++x){
                out_alphaBuf.push_back(src.pixels(x, y).a);
            }
        }
    }
    
    void TFrgParameter::setParameter(float _quality,float _compressSizeParameter){
        isDeleteEmptyColor=true;
        assert(_quality<=kFrg_quality_max);
        assert(_quality>=0);
        quality=_quality;
        if (quality>kFrg_quality_max)
            quality=kFrg_quality_max;
        else if (quality<0)
            quality=0;
        
        isMustFitColorTable=(quality<=kFrg_quality_default);
        //bgr+alpha  bzip 0-8
        //only alpha bzip 1--16   //8 -> kFrg_size_default
        //only alpha rle 3--15
        const int param_alpha_rle_count=15-3+1;
        const int param_alpha_zip_count=16-1+1;
        const int param_bgra_zip_count=8-0+1;
        const int param_count=param_alpha_rle_count+param_alpha_zip_count+param_bgra_zip_count;
        const int param_default_index=8+8-0;
        int ssp; 
        if (_compressSizeParameter<=kFrg_size_default)
            ssp=0+(int)(_compressSizeParameter*(2*(param_default_index+0.5)/100));
        else
            ssp=param_default_index+(int)((_compressSizeParameter-kFrg_size_default)*(2*(param_count-param_default_index+0.5)/100)+0.49999);
        if (ssp<0)
            ssp=0;
        else if (ssp>param_count)
            ssp=param_count;
        
        if (ssp<param_bgra_zip_count){
            zip_parameter=ssp;
            rle_parameter=TBytesRle::kRle_size_bestUnRleSpeed;
            isAlphaDataUseBytesZip=true;
            isRGBDataUseBytesZip=true;
        }else if (ssp<param_bgra_zip_count+param_alpha_zip_count){
            zip_parameter=(ssp-param_bgra_zip_count)+1;
            rle_parameter=TBytesRle::kRle_size_bestUnRleSpeed;
            isAlphaDataUseBytesZip=true;
            isRGBDataUseBytesZip=false;
        }else {
            zip_parameter=-1;//null 
            rle_parameter=(ssp-(param_bgra_zip_count+param_alpha_zip_count))+3;
            isAlphaDataUseBytesZip=false;
            isRGBDataUseBytesZip=false;
        }
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
    if (!isSingleAlpha){
        std::vector<TByte> alphaBuf;
        getAlphasFromPixelsRef(alphaBuf,src);
        
        tempMemoryByteSize_cur+=(int)alphaBuf.size(); //for save all alphas when frgFileRead
        if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;

        std::vector<TByte> rleCode;
        TBytesRle::save(rleCode, &alphaBuf[0],&alphaBuf[0]+alphaBuf.size(),parameter.rle_parameter);
        if (parameter.isAlphaDataUseBytesZip){
            std::vector<TByte> zipCode;
            FRZ1_compress(zipCode,&rleCode[0],&rleCode[0]+rleCode.size(),parameter.zip_parameter);
            writeUInt32(code_alpha, (TUInt32)rleCode.size());
            code_alpha.insert(code_alpha.end(),zipCode.begin(),zipCode.end());
            
            tempMemoryByteSize_cur+=(int)rleCode.size();//for save unzip relCode when frgFileRead
            if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
            tempMemoryByteSize_cur-=(int)rleCode.size();
        }else{
            code_alpha.insert(code_alpha.end(),rleCode.begin(),rleCode.end());
        }
    }else{//isSingleAlpha
        tempMemoryByteSize_cur+=src.width;//for save min width alphas when frgFileRead, alpha's byte_width=0
        if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
    }
    
    //处理RGB通道.
    std::vector<TByte> code_bgr;
    if (!isSingleBGR){        
        int tempMemoryByteSize_bgrZip=0;
        TColorZiper::saveTo(code_bgr, src,parameter.quality,parameter.isMustFitColorTable,&tempMemoryByteSize_bgrZip);
        tempMemoryByteSize_cur+=tempMemoryByteSize_bgrZip;//for load BGRColor from code_bgr when frgFileRead
        if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
       
        if (parameter.isRGBDataUseBytesZip){
            tempMemoryByteSize_cur+=(int)code_bgr.size(); //for save code_bgr when frgFileRead
            if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
            
            std::vector<TByte> color_temp;
            writeUInt32(color_temp, (TInt32)code_bgr.size());
            FRZ1_compress(color_temp,&code_bgr[0],&code_bgr[0]+code_bgr.size(),parameter.zip_parameter);
            code_bgr.swap(color_temp);
            
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
        colorInfo|=parameter.isAlphaDataUseBytesZip?kColorInfo_isAlphaDataUseBytesZip:0;
        colorInfo|=parameter.isRGBDataUseBytesZip?kColorInfo_isRGBDataUseBytesZip:0;
        
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
    writeUInt32(code_data,(TInt32)code_head.size());
    code_data.insert(code_data.end(), code_head.begin(),code_head.end());
    
    //alpha code
    if (!isSingleAlpha){
        writeUInt32(code_data,(TInt32)code_alpha.size());
        code_data.insert(code_data.end(), code_alpha.begin(),code_alpha.end());
    }
    //BGR code
    if (!isSingleBGR){
        writeUInt32(code_data,(TInt32)code_bgr.size());
        code_data.insert(code_data.end(), code_bgr.begin(),code_bgr.end());
    }
}

}//end namespace frg
