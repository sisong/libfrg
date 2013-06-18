//bytes_zip.cpp
/*
 Copyright (c) 2012-2013 HouSisong All Rights Reserved.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */
#include "FRZ1_compress.h"
#include "FRZ1_decompress_base.h"
#include  <assert.h>
#include  <map>
#include "FRZ_private/suffix_string.h"

namespace {
    
    static const int _kMaxForwardOffsert_zip_parameter_table_size=8+1;
    static const int _kMaxForwardOffsert_zip_parameter_table[_kMaxForwardOffsert_zip_parameter_table_size]={
        8*1024*1024, 6*1024*1024, 4*1024*1024, 2*1024*1024, 1*1024*1024, //0..4
        880*1024,780*1024,700*1024,600*1024//5..8
    };
    static const int _kMaxForwardOffsert_zip_parameter_table_minValue=200*1024;
   
    
    
    //变长32bit正整数编码方案(x bit额外类型标志位,x<=3),从高位开始输出1-5byte:
    // x0*  7-x bit
    // x1* 0*  7+7-x bit
    // x1* 1* 0*  7+7+7-x bit
    // x1* 1* 1* 0*  7+7+7+7-x bit
    // x1* 1* 1* 1* 0*  7+7+7+7+7-x bit
    
    static void pack32BitWithTag(std::vector<TFRZ_Byte>& out_code,TFRZ_UInt32 iValue,int highBit,const int kTagBit){//写入并前进指针.
        const int kMaxPack32BitTagBit=3;
        assert((0<=kTagBit)&&(kTagBit<=kMaxPack32BitTagBit));
        assert((highBit>>kTagBit)==0);
        const int kMaxPack32BitSize=5;
        const unsigned int kMaxValueWithTag=(1<<(7-kTagBit))-1;
        
        TFRZ_Byte codeBuf[kMaxPack32BitSize];
        TFRZ_Byte* codeEnd=codeBuf;
        while (iValue>kMaxValueWithTag) {
            *codeEnd=iValue&((1<<7)-1); ++codeEnd;
            iValue>>=7;
        }
        out_code.push_back( (highBit<<(8-kTagBit)) | iValue | (((codeBuf!=codeEnd)?1:0)<<(7-kTagBit)));
        while (codeBuf!=codeEnd) {
            --codeEnd;
            out_code.push_back((*codeEnd) | (((codeBuf!=codeEnd)?1:0)<<7));
        }
    }
    
    inline static int pack32BitWithTagOutSize(TFRZ_UInt32 iValue,int kTagBit){//返回pack后字节大小.
        if (iValue<(TFRZ_UInt32)(1<<(7+7-kTagBit))){
            if (iValue<(TFRZ_UInt32)(1<<(7-kTagBit))){
                return 1;
            }else{
                return 2;
            }
        }else{
            if (iValue<(TFRZ_UInt32)(1<<(7+7+7-kTagBit))){
                return 3;
            }else if (iValue<(TFRZ_UInt32)(1<<(7+7+7+7-kTagBit))){
                return 4;
            }else {//if (iValue<(TFRZ_UInt32)(1<<(7+7+7+7+7-kTagBit))){
                return 5;
            }
        }
    }
    
    static inline void pack32Bit(std::vector<TFRZ_Byte>& out_code,TFRZ_UInt32 iValue){
        pack32BitWithTag(out_code, iValue, 0, 0);
    }
    static inline int pack32BitOutSize(TFRZ_UInt32 iValue){
        return pack32BitWithTagOutSize(iValue, 0);
    }

    
    
    class TBytesZiper_suffix{
    public:
        TBytesZiper_suffix(const TFRZ_Byte* src,const TFRZ_Byte* src_end)
        :m_sstring((const char*)src,(const char*)src_end){
            m_sstring.R_create();
            m_sstring.LCP_create();
        }

        void  saveCodeTo(std::vector<TFRZ_Byte>& out_code,int zip_parameter){
            assert(zip_parameter>=kFRZ_bestSize);
            assert(zip_parameter<=kFRZ_bestUncompressSpeed);
            const int sstrSize=(int)m_sstring.size();

            std::vector<TFRZ_Byte> codeBuf;
            std::vector<TFRZ_Byte> ctrlBuf;

            TFRZ_Int32 nozipBegin=0;
            TFRZ_Int32 curIndex=1;
            while (curIndex<sstrSize) {
                TFRZ_Int32 matchLength;
                TFRZ_Int32 matchPos;
                TFRZ_Int32 zipLength;
                if (getBestMatch(curIndex,&matchLength,&matchPos,&zipLength,zip_parameter,nozipBegin,sstrSize)){
                    ++m_forwardOffsert_memcache[memcacheKey(matchPos)];
                    if (curIndex!=nozipBegin){//out no zip data
                        pushNoZipData(codeBuf,ctrlBuf,nozipBegin,curIndex);
                    }

                    const TFRZ_Int32 frontMatchPos=curIndex-matchPos;
                    pushZipData(codeBuf,ctrlBuf,matchLength,frontMatchPos);

                    curIndex+=matchLength;
                    assert(curIndex<=sstrSize);
                    nozipBegin=curIndex;
                }else{
                    ++curIndex;
                }
            }
            if (nozipBegin<sstrSize){
                pushNoZipData(codeBuf,ctrlBuf,nozipBegin,(TFRZ_Int32)sstrSize);
            }

            pack32Bit(out_code,(TFRZ_Int32)ctrlBuf.size());
            out_code.insert(out_code.end(),ctrlBuf.begin(),ctrlBuf.end());
            out_code.insert(out_code.end(),codeBuf.begin(),codeBuf.end());
        }
    private:
        TSuffixString m_sstring;
        std::map<int,int> m_forwardOffsert_memcache;
        inline static int memcacheKey(int matchpos){ return matchpos>>3; }
        inline static int getCtrlLengthOutSize(int ctrlLength){ return pack32BitWithTagOutSize(ctrlLength-1,kFRZCodeType_bit); }

        void _getBestMatch(TSuffixIndex curString,TFRZ_Int32& curBestZipLength,TFRZ_Int32& curBestMatchString,TFRZ_Int32& curBestMatchLength,int it_inc,int kMaxForwardOffsert,int kStringLength,int kPkSizeAllLength){
            //const TFRZ_Int32 it_cur=m_sstring.lower_bound(m_sstring.ssbegin+curString,m_sstring.ssend);
            const TFRZ_Int32 it_cur=m_sstring.lower_bound_withR(curString);//查找curString自己的位置.
            int it=it_cur+it_inc;
            int it_end;
            const TFRZ_Int32* LCP;//当前的后缀字符串和下一个后缀字符串的相等长度.
            if (it_inc==1){
                it_end=(int)m_sstring.size();
                LCP=&m_sstring.LCP[it_cur];
            }else{
                assert(it_inc==-1);
                it_end=-1;
                LCP=&m_sstring.LCP[it_cur]-1;
            }

            const int kMaxValue_lcp=((TFRZ_UInt32)1<<31)-1;
            int lcp=kMaxValue_lcp;
            for (;it!=it_end;it+=it_inc,LCP+=it_inc){
                int curLCP=*LCP;
                if (curLCP<lcp)
                    lcp=curLCP;

                if ((lcp-2)<curBestZipLength)//不可能压缩了.
                    break;

                TSuffixIndex matchString=m_sstring.SA[it];
                const int curForwardOffsert=(curString-matchString);
                if (curForwardOffsert>0){
                    TFRZ_Int32 zipedLength=lcp-pack32BitOutSize(curForwardOffsert-1)-getCtrlLengthOutSize(lcp)-getCtrlLengthOutSize(kStringLength-lcp)+kPkSizeAllLength;
                    if (curForwardOffsert>kMaxForwardOffsert){//惩罚.
                        zipedLength-=8;
                        if (curForwardOffsert>kMaxForwardOffsert*2)
                            zipedLength-=16;
                    }
                    if (zipedLength>=curBestZipLength){
                        if( (curBestMatchString<0) || (zipedLength>curBestZipLength)
                           ||(m_forwardOffsert_memcache[memcacheKey(matchString)]>m_forwardOffsert_memcache[memcacheKey(curBestMatchString)])
                           ||((m_forwardOffsert_memcache[memcacheKey(matchString)]==m_forwardOffsert_memcache[memcacheKey(curBestMatchString)])&&(matchString>curBestMatchString))){
                            curBestZipLength=zipedLength;
                            curBestMatchString=matchString;
                            curBestMatchLength=lcp;
                        }
                    }
                }
            }
        }

        inline bool getBestMatch(TSuffixIndex curString,TFRZ_Int32* out_curBestMatchLength,TFRZ_Int32* out_curBestMatchPos,TFRZ_Int32* out_curBestZipLength,int zip_parameter,int nozipBegin,int endString){
            int kMaxForwardOffsert;//增大可以提高压缩率但可能会减慢解压速度(缓存命中降低).
            const int kS=_kMaxForwardOffsert_zip_parameter_table_size;
            if (zip_parameter<kS){
                kMaxForwardOffsert=_kMaxForwardOffsert_zip_parameter_table[zip_parameter];
            }else{
                const int kMax=_kMaxForwardOffsert_zip_parameter_table[kS-1];
                const int kMin=_kMaxForwardOffsert_zip_parameter_table_minValue;
                if (zip_parameter>=kFRZ_bestUncompressSpeed)
                    kMaxForwardOffsert=kMin;
                else
                    kMaxForwardOffsert=kMax-(kMax-kMin)*(zip_parameter-kS)/(kFRZ_bestUncompressSpeed-kS);
            }
            
            const int noZipLength=curString-nozipBegin;
            const int allLength=endString-nozipBegin;
            const int kPkSizeAllLength=getCtrlLengthOutSize(allLength);
            //   psize(noZipLength)+noZipLength
            // + psize(zipLength)+psize(ForwardOffsert)
            // + psize(allLength-noZipLength-zipLength)+ allLength-noZipLength-zipLength
            // <  psize(allLength)+allLength + zip_parameter
            int minZipLength=zip_parameter+1;//最少要压缩的字节数.
            if (noZipLength>0)
                ++minZipLength;
            *out_curBestZipLength=minZipLength;
            *out_curBestMatchPos=-1;
            *out_curBestMatchLength=0;
            _getBestMatch(curString,*out_curBestZipLength,*out_curBestMatchPos,*out_curBestMatchLength,1,kMaxForwardOffsert,allLength-noZipLength,kPkSizeAllLength);
            _getBestMatch(curString,*out_curBestZipLength,*out_curBestMatchPos,*out_curBestMatchLength,-1,kMaxForwardOffsert,allLength-noZipLength,kPkSizeAllLength);

            if ((*out_curBestMatchPos)<0)
                return false;
            return true;
        }

        void pushNoZipData(std::vector<TFRZ_Byte>&out_code,std::vector<TFRZ_Byte>& out_ctrl,TFRZ_Int32 nozipBegin,TFRZ_Int32 nozipEnd)const{
            assert(nozipEnd>nozipBegin);
            assert(nozipEnd<=m_sstring.size());
            const TFRZ_Byte* data=(const TFRZ_Byte*)m_sstring.ssbegin+nozipBegin;
            const TFRZ_Byte* data_end=(const TFRZ_Byte*)m_sstring.ssbegin+nozipEnd;
            pack32BitWithTag(out_ctrl,(nozipEnd-nozipBegin)-1, kFRZCodeType_nozip,kFRZCodeType_bit);
            out_code.insert(out_code.end(),data,data_end);
        }

        void pushZipData(std::vector<TFRZ_Byte>&out_code,std::vector<TFRZ_Byte>& out_ctrl,TFRZ_Int32 matchLength,TFRZ_Int32 frontMatchPos)const{
            assert(frontMatchPos>0);
            assert(matchLength>0);
            pack32BitWithTag(out_ctrl,matchLength-1, kFRZCodeType_zip,kFRZCodeType_bit);
            pack32Bit(out_ctrl,frontMatchPos-1);
        }

    };
    
} //end namespace
    

void FRZ1_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    TBytesZiper_suffix ssZiper(src,src_end);
    ssZiper.saveCodeTo(out_code,zip_parameter);
}
    
