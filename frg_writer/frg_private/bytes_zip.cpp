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

    class TBytesZiper_suffix{
    public:
        TBytesZiper_suffix(const TByte* src,const TByte* src_end)
        :m_sstring((const char*)src,(const char*)src_end){
            m_sstring.R_create();
            m_sstring.LCP_create();
        }

        void  saveCodeTo(std::vector<TByte>& out_code,int zip_parameter){
            assert(zip_parameter>=TBytesZip::kZip_bestSize);
            //assert(zip_parameter<=TBytesZiper::kZip_bestUnZipSpeed);
            const int kMinZipLength=zip_parameter; //增大该值,则压缩率变小,解压稍快  0时，压缩率最大.
            const int sstrSize=(int)m_sstring.size();

            std::vector<TByte> codeBuf;
            std::vector<TByte> ctrlBuf;

            TInt32 nozipBegin=0;
            TInt32 curIndex=1;
            while (curIndex<sstrSize) {
                TInt32 matchLength;
                TInt32 matchPos;
                TInt32 zipLength;
                if (getBestMatch(curIndex,&matchLength,&matchPos,&zipLength,pack32BitWithTagOutSize(curIndex-nozipBegin,0),kMinZipLength)){
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

        void _getBestMatch(TSuffixIndex curString,TInt32& curBestZipLength,TInt32& curBestMatchString,TInt32& curMatchLength,int it_inc)const{
            //const TInt32 it_cur=m_sstring.lower_bound(m_sstring.ssbegin+curString,m_sstring.ssend);
            const TInt32 it_cur=m_sstring.lower_bound_withR(curString);
            int it=it_cur+it_inc;
            int it_end;
            const TInt32* LCP;
            if (it_inc==1){
                it_end=(int)m_sstring.size();
                LCP=&m_sstring.LCP[it_cur];
            }else{
                assert(it_inc==-1);
                it_end=-1;
                LCP=&m_sstring.LCP[it_cur-1];
            }

            const int kMaxForwardOffsert=1*1024*1024; //增大该值可能增大匹配机率(从而增大压缩率),但会降低压缩速度.
            const int kMaxValue_lcp=((TUInt32)1<<31)-1;
            int lcp=kMaxValue_lcp;
            for (;it!=it_end;it+=it_inc,LCP+=it_inc){
                int curLCP=*LCP;
                if (curLCP<lcp)
                    lcp=curLCP;

                if ((lcp-2)<=curBestZipLength)
                    return;

                TSuffixIndex matchString=m_sstring.SA[it];
                const int curForwardOffsert=(curString-matchString);
                if ((curForwardOffsert>0)&&(curForwardOffsert<=kMaxForwardOffsert)){
                    TInt32 zipLength=lcp-pack32BitWithTagOutSize(curForwardOffsert,1)-pack32BitWithTagOutSize(lcp,0);
                    if (zipLength>curBestZipLength){
                        curBestZipLength=zipLength;
                        curBestMatchString=matchString;
                        curMatchLength=lcp;
                    }
                }
            }
        }

        inline bool getBestMatch(TSuffixIndex curString,TInt32* out_matchLength,TInt32* out_matchPos,TInt32* out_zipLength,int unzipLengthPackSize,int kMinZipLength)const{
            *out_zipLength=kMinZipLength-1+unzipLengthPackSize;
            *out_matchPos=-1;
            *out_matchLength=0;
            _getBestMatch(curString,*out_zipLength,*out_matchPos,*out_matchLength,1);
            _getBestMatch(curString,*out_zipLength,*out_matchPos,*out_matchLength,-1);

            if ((*out_matchPos)<0)
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

#if 0
//测试其他压缩算法:lzo
//#include "other_lib/liblzo/minilzo.h"
//#define HEAP_ALLOC(var,size) lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
//#include "other_lib/liblzo/lzo1x.h"

    void TBytesZiper::load(TByte* out_dst,TByte* out_dstEnd,const TByte* zip_code,const TByte* zip_code_end){
        /*//lzo
        Int _isNeedRle=*zip_code; ++zip_code;
                int ir=lzo_init();
        assert (ir== LZO_E_OK);
        lzo_uint dstLength=out_dstEnd-out_dst;
        lzo1x_decompress(zip_code,zip_code_end-zip_code,out_dst,&dstLength,0);
        return;
        //*/

        TBytesZiper_suffix::load(out_dst,out_dstEnd, zip_code, zip_code_end);
    }
#endif


    void TBytesZip::save(std::vector<TByte>& out_code,const TByte* src,const TByte* src_end,int zip_parameter){
        /*//lzo
        //static HEAP_ALLOC(wrkmem,LZO1X_1_MEM_COMPRESS);
        char wrkmem[LZO1X_999_MEM_COMPRESS];
        if (isUseRleFirst){
            out_code.push_back(1);
        }else{
            out_code.push_back(0);
        }
        int ir=lzo_init();
        assert (ir== LZO_E_OK);

        std::vector<TByte> tempCode;
        lzo_uint inLen=src_end-src;
        lzo_uint outLen= (inLen + inLen / 16 + 64 + 3);
        tempCode.resize(outLen);

        int r = lzo1x_999_compress(src,inLen,&tempCode[0],&outLen,wrkmem);
        assert(r==LZO_E_OK);
        tempCode.resize(outLen);
        out_code.insert(out_code.end(), tempCode.begin(),tempCode.end());
        return;
        //*/

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
