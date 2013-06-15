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
#include "bytes_zip.h"
#include  <queue>
#include "suffix_string.h"

namespace frg {
    
    static const int _kMaxForwardOffsert_zip_parameter_table_size=8+1;
    static const int _kMaxForwardOffsert_zip_parameter_table[_kMaxForwardOffsert_zip_parameter_table_size]={
        8*1024*1024, 6*1024*1024, 4*1024*1024, 2*1024*1024, 1*1024*1024, //0..4
        880*1024,780*1024,700*1024,600*1024//5..8
    };
    static const int _kMaxForwardOffsert_zip_parameter_table_minValue=200*1024;
   
    class TBytesZiper_suffix{
    public:
        TBytesZiper_suffix(const TByte* src,const TByte* src_end)
        :m_sstring((const char*)src,(const char*)src_end){
            m_sstring.R_create();
            m_sstring.LCP_create();
        }

        void  saveCodeTo(std::vector<TByte>& out_code,int zip_parameter){
            assert(zip_parameter>=TBytesZip::kZip_bestSize);//增大该值,则压缩率变小,解压稍快  0时，压缩率最大.
            //assert(zip_parameter<=TBytesZiper::kZip_bestUnZipSpeed);
            const int sstrSize=(int)m_sstring.size();

            std::vector<TByte> codeBuf;
            std::vector<TByte> ctrlBuf;

            TInt32 nozipBegin=0;
            TInt32 curIndex=1;
            while (curIndex<sstrSize) {
                TInt32 matchLength;
                TInt32 matchPos;
                TInt32 zipLength;
                if (getBestMatch(curIndex,&matchLength,&matchPos,&zipLength,zip_parameter)){
                    ++m_forwardOffsert_memcache[memcacheKey(matchPos)];
                    if (curIndex!=nozipBegin){//out no zip data
                        pushNoZipData(codeBuf,ctrlBuf,nozipBegin,curIndex);
                    }

                    const TInt32 frontMatchPos=curIndex-matchPos;
                    pushZipData(codeBuf,ctrlBuf,matchLength,frontMatchPos);

                    curIndex+=matchLength;
                    assert(curIndex<=sstrSize);
                    nozipBegin=curIndex;
                }else{
                    ++curIndex;
                }
            }
            if (nozipBegin<sstrSize){
                pushNoZipData(codeBuf,ctrlBuf,nozipBegin,(TInt32)sstrSize);
            }

            pack32Bit(out_code,(TInt32)ctrlBuf.size());
            out_code.insert(out_code.end(),ctrlBuf.begin(),ctrlBuf.end());
            out_code.insert(out_code.end(),codeBuf.begin(),codeBuf.end());
        }
    private:
        TSuffixString m_sstring;
        std::map<int,int> m_forwardOffsert_memcache;
        inline static int memcacheKey(int matchpos){ return matchpos>>3; }

        void _getBestMatch(TSuffixIndex curString,TInt32& curBestZipLength,TInt32& curBestMatchString,TInt32& curBestMatchLength,int it_inc,int kMaxForwardOffsert){
            //const TInt32 it_cur=m_sstring.lower_bound(m_sstring.ssbegin+curString,m_sstring.ssend);
            const TInt32 it_cur=m_sstring.lower_bound_withR(curString);//查找curString自己的位置.
            int it=it_cur+it_inc;
            int it_end;
            const TInt32* LCP;//当前的后缀字符串和下一个后缀字符串的相等长度.
            if (it_inc==1){
                it_end=(int)m_sstring.size();
                LCP=&m_sstring.LCP[it_cur];
            }else{
                assert(it_inc==-1);
                it_end=-1;
                LCP=&m_sstring.LCP[it_cur]-1;
            }

            const int kMaxValue_lcp=((TUInt32)1<<31)-1;
            int lcp=kMaxValue_lcp;
            for (;it!=it_end;it+=it_inc,LCP+=it_inc){
                int curLCP=*LCP;
                if (curLCP<lcp)
                    lcp=curLCP;

                if ((lcp-2)<=curBestZipLength)//不可能压缩了.
                    break;

                TSuffixIndex matchString=m_sstring.SA[it];
                const int curForwardOffsert=(curString-matchString);
                if ((curForwardOffsert>0)&&(curForwardOffsert<=kMaxForwardOffsert)){
                    TInt32 zipedLength=lcp-pack32BitOutSize(curForwardOffsert-1)-pack32BitWithTagOutSize(lcp-1,kBytesZipType_bit);
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

        inline bool getBestMatch(TSuffixIndex curString,TInt32* out_curBestMatchLength,TInt32* out_curBestMatchPos,TInt32* out_curBestZipLength,int zip_parameter){
            int kMaxForwardOffsert;//增大可以提高压缩率但可能会减慢解压速度(缓存命中降低).
            const int kS=_kMaxForwardOffsert_zip_parameter_table_size;
            if (zip_parameter<kS){
                kMaxForwardOffsert=_kMaxForwardOffsert_zip_parameter_table[zip_parameter];
            }else{
                const int kMax=_kMaxForwardOffsert_zip_parameter_table[kS-1];
                const int kMin=_kMaxForwardOffsert_zip_parameter_table_minValue;
                if (zip_parameter>=TBytesZip::kZip_bestUnZipSpeed)
                    kMaxForwardOffsert=kMin;
                else
                    kMaxForwardOffsert=kMax-(kMax-kMin)*(zip_parameter-kS)/(TBytesZip::kZip_bestUnZipSpeed-kS);
            }

            *out_curBestZipLength=zip_parameter+1;//最少要压缩的字节数.
            *out_curBestMatchPos=-1;
            *out_curBestMatchLength=0;
            _getBestMatch(curString,*out_curBestZipLength,*out_curBestMatchPos,*out_curBestMatchLength,1,kMaxForwardOffsert);
            _getBestMatch(curString,*out_curBestZipLength,*out_curBestMatchPos,*out_curBestMatchLength,-1,kMaxForwardOffsert);

            if ((*out_curBestMatchPos)<0)
                return false;
            return true;
        }

        void pushNoZipData(std::vector<TByte>&out_code,std::vector<TByte>& out_ctrl,TInt32 nozipBegin,TInt32 nozipEnd)const{
            assert(nozipEnd>nozipBegin);
            assert(nozipEnd<=m_sstring.size());
            const TByte* data=(const TByte*)m_sstring.ssbegin+nozipBegin;
            const TByte* data_end=(const TByte*)m_sstring.ssbegin+nozipEnd;
            pack32BitWithTag(out_ctrl,(nozipEnd-nozipBegin)-1, kBytesZipType_nozip,kBytesZipType_bit);
            out_code.insert(out_code.end(),data,data_end);
        }

        void pushZipData(std::vector<TByte>&out_code,std::vector<TByte>& out_ctrl,TInt32 matchLength,TInt32 frontMatchPos)const{
            assert(frontMatchPos>0);
            assert(matchLength>0);
            pack32BitWithTag(out_ctrl,matchLength-1, kBytesZipType_zip,kBytesZipType_bit);
            pack32Bit(out_ctrl,frontMatchPos-1);
        }

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    void TBytesZip::save(std::vector<TByte>& out_code,const TByte* src,const TByte* src_end,int zip_parameter){
        //int pos0_forTest=(int)out_code.size();  //for test check load
        TBytesZiper_suffix ssZiper(src,src_end);
        ssZiper.saveCodeTo(out_code,zip_parameter);
        /*test check load
        {
            const TByte* code=&out_code[pos0_forTest];
            const TByte* code_end=&out_code[0]+out_code.size();
            const std::vector<TByte> old_data(src,src_end);
            std::vector<TByte> new_data(src_end-src);
            TBytesZiper_suffix::load(&new_data[0],&new_data[0]+new_data.size(),code,code_end);
            assert(old_data==new_data);
        }
        //*/
    }

} //end namespace frg
