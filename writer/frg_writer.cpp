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
        TUInt srcSize=(TUInt)(src_end-src);
        if (srcSize>LZ4_MAX_INPUT_SIZE) throw TFrgRunTimeError("frgZip_compress() srcSize>LZ4_MAX_INPUT_SIZE.");
        TUInt oldSize=(TUInt)out_code.size();
        out_code.resize(oldSize+LZ4_compressBound((int)srcSize));
        //int codeSize=LZ4_compressHC((const char*)src,(char*)&out_code[oldSize],(int)srcSize);
        const int kLZ4HC2MaxCompressionLevel=16;
        int codeSize=LZ4_compressHC2((const char*)src,(char*)&out_code[oldSize],(int)srcSize,kLZ4HC2MaxCompressionLevel);
        if (codeSize<=0) throw TFrgRunTimeError("LZ4_compressHC2() codeSize<=0.");
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

        isDeleteEmptyColor=true; //now must true?

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


        //是否应该压缩的判断算法:
        //不压缩时的加载总时间 T0 = 数据大小B/磁盘速度Ds
        //假设压缩后大小Z1,则 加载总时间 T1= Z1/Ds+B/单位时间解码输出速度Us
        //如果要求T1<=T0,则compressRatio=Z1/B, compressRatio<=1-Ds/Us
        //
        //压缩参数 -> compressRatio
        //  0 -> ratio_min
        // 50 -> ratio_default
        //100 -> ratio_max
        const float ratio_min=0.98f;     //1 -  2/100
        const float ratio_default=0.85f; //1 - 15/100
        const float ratio_max=0.5f;      //1 - 50/100
        if (compressSizeParameter<=kFrg_size_default)
            compressRatio=(compressSizeParameter-kFrg_size_minSize)*((ratio_default-ratio_min)/(kFrg_size_default-kFrg_size_minSize))+ratio_min;
        else
            compressRatio=(compressSizeParameter-kFrg_size_default)*((ratio_max-ratio_default)/(kFrg_size_maxUnZipSpeed-kFrg_size_default))+ratio_default;

        isAlphaUseRleAdvance=true; //测试用rle是否能压得更好.
        const int rle_max_useZip=11;
        alphaRleAdvanceParameter=rle_max_useZip;
    }

    static inline bool tryCompressCodeData(std::vector<TByte>& data_ziped,const std::vector<TByte>& data,TUInt limitZipSize){
        assert(data_ziped.empty());
        TUInt32 dataSize=ToUInt32(data.size(),"tryCompressCodeData() data.size() over 32bit.");
        writeUInt32(data_ziped,dataSize);
        frgZip_compress(data_ziped,&data[0],&data[0]+data.size());
        return (data_ziped.size()<=limitZipSize);
    }


void writeFrgImage(std::vector<TByte>& outFrgCode,const TFrgPixels32Ref& _srcImage,const TFrgParameter& parameter){
    assert(_srcImage.width>=0);
    assert(_srcImage.height>=0);
    if ((TUInt)_srcImage.width>kMaxImageWidth) throw TFrgRunTimeError("writeFrgImage() srcImage.width>kMaxImageWidth.");
    if ((TUInt)_srcImage.height>kMaxImageHeight) throw TFrgRunTimeError("writeFrgImage() srcImage.height>kMaxImageHeight.");
    TPixels32Ref src;
    src.pColor=(TBGRA32*)_srcImage.pColor;
    src.width=_srcImage.width;
    src.height=_srcImage.height;
    src.byte_width=_srcImage.byte_width;
    TPixels32Buffer _srcBuf;

    //预处理,删除全透明像素.
    if(parameter.isDeleteEmptyColor){
        //复制.
        _srcBuf.resizeFast(src.width,src.height);
        pixelsCopy(_srcBuf.getRef(),src);
        delEmptyColor(_srcBuf.getRef());
        src=_srcBuf.getRef();
    }

    //单色判断.
    TBGRA32 singleColor(0,0,0,0);
    bool isSingleBGR=getIsSigleRGBColor(src,&singleColor);
    bool isSingleAlpha=getIsSigleAlphaColor(src, &singleColor.a);

    TUInt tempMemoryByteSize_max=0;
    TUInt tempMemoryByteSize_cur=0;
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

        std::vector<TByte> zip0Code;
        bool isAlphaDataUseBytesZip1=tryCompressCodeData(zip0Code,alphaBuf,(TUInt)(1.0*parameter.compressRatio*alphaBuf.size()));

        bool isAlphaUseRleAdvance_succeed=false;
        if (parameter.isAlphaUseRleAdvance){
            std::vector<TByte> rleCode;
            bytesRLE_save(rleCode,&alphaBuf[0],&alphaBuf[0]+alphaBuf.size(),parameter.alphaRleAdvanceParameter);
            std::vector<TByte> rleCode_ziped;
            //是否应该压缩的判断算法:
            //不压缩时 T0 = B/Ds
            //压缩时  T2 = Z2/Ds+rle压缩后大小R/Us+B/rle解压速度Rs
            //假设Rs==Us,要求T2<=T0,则 Z2<=(R+B)*compressRatio-R
            //并且要求T2<=T1,其中,T1=Z1/Ds+B/Us, 则 Z2<=Z1-R*(1-compressRatio)
            TUInt limitZipSize=(TUInt)(1.0*parameter.compressRatio*(alphaBuf.size()+rleCode.size())-rleCode.size());
            TUInt _limitZipSize1=(TUInt)(1.0*zip0Code.size()-rleCode.size()*(1.0-parameter.compressRatio))-1;
            if (_limitZipSize1<limitZipSize)
                limitZipSize=_limitZipSize1;
            bool isAlphaDataUseBytesZip2=tryCompressCodeData(rleCode_ziped,rleCode,limitZipSize);
            if (isAlphaDataUseBytesZip2){
                isAlphaDataUseBytesRLE=true;
                isAlphaDataUseBytesZip=true;

                tempMemoryByteSize_cur+=alphaBuf.size(); //for save all alphas when frgFileRead
                if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
                tempMemoryByteSize_cur+=rleCode.size();//for save unzip relCode when frgFileRead
                if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
                tempMemoryByteSize_cur-=rleCode.size();

                code_alpha.insert(code_alpha.end(),rleCode_ziped.begin(),rleCode_ziped.end());
                isAlphaUseRleAdvance_succeed=true;//ok  同时使用rle和数据压缩.
            }
        }
        if (!isAlphaUseRleAdvance_succeed){
            isAlphaDataUseBytesRLE=false;
            isAlphaDataUseBytesZip=isAlphaDataUseBytesZip1;
            if (isAlphaDataUseBytesZip){
                tempMemoryByteSize_cur+=alphaBuf.size(); //for save all alphas when frgFileRead
                if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
                code_alpha.insert(code_alpha.end(),zip0Code.begin(),zip0Code.end());//使用数据压缩.
            }else{
                code_alpha.insert(code_alpha.end(),alphaBuf.begin(),alphaBuf.end());//不压缩.
            }
        }
    }

    //处理RGB通道.
    std::vector<TByte> code_bgr;
    bool isRGBDataUseBytesZip=false;
    if (!isSingleBGR){
        TUInt bgrColorTableLength=0;
        bgrZiper_save(code_bgr, src,parameter.quality,parameter.isMustFitColorTable,&bgrColorTableLength);
        tempMemoryByteSize_cur+=bgrColorTableLength*sizeof(TUInt32)+3; //+3 for Align4; //for decoder load BGRColor
        if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;

        std::vector<TByte> color_bgr_ziped;
        isRGBDataUseBytesZip=tryCompressCodeData(color_bgr_ziped,code_bgr,(TUInt)(1.0*parameter.compressRatio*code_bgr.size()));
        if (isRGBDataUseBytesZip){
            tempMemoryByteSize_cur+=code_bgr.size(); //for save code_bgr when frgFileRead
            if (tempMemoryByteSize_cur>tempMemoryByteSize_max) tempMemoryByteSize_max=tempMemoryByteSize_cur;
            code_bgr.swap(color_bgr_ziped);
        }
    }

    //head code
    std::vector<TByte> code_head;
    {
        writeUInt32(code_head, src.width);
        writeUInt32(code_head, src.height);
        writeUInt32(code_head,ToUInt32(tempMemoryByteSize_max,"writeFrgImage() tempMemoryByteSize over 32bit."));//throw
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
    TUInt imageFileSize=sizeof(TUInt32)+code_data.size();
    imageFileSize+=sizeof(TUInt32)+code_head.size();
    if (!isSingleAlpha)
        imageFileSize+=sizeof(TUInt32)+code_alpha.size();
    if (!isSingleBGR)
        imageFileSize+=sizeof(TUInt32)+code_bgr.size();
    writeUInt32(code_data,ToUInt32(imageFileSize,"writeFrgImage() imageFileSize over 32bit."));

    //head code
    writeUInt32(code_data,ToUInt32(code_head.size(),"writeFrgImage() code_head.size() over 32bit."));
    code_data.insert(code_data.end(), code_head.begin(),code_head.end());

    //alpha code
    if (!isSingleAlpha){
        writeUInt32(code_data,ToUInt32(code_alpha.size(),"writeFrgImage() code_alpha.size() over 32bit."));
        code_data.insert(code_data.end(), code_alpha.begin(),code_alpha.end());
    }
    //BGR code
    if (!isSingleBGR){
        writeUInt32(code_data,ToUInt32(code_bgr.size(),"writeFrgImage() code_bgr.size() over 32bit."));
        code_data.insert(code_data.end(), code_bgr.begin(),code_bgr.end());
    }
}

}//end namespace frg
