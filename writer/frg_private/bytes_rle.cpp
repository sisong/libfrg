//bytes_rle.cpp
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
#include "bytes_rle.h"
#include "assert.h"
#include "frg_writer_base.h"

namespace frg {
    
namespace {
    
    class TBytesZiper_rle{
    public:
        inline static const unsigned char* getEqualEnd(const unsigned char* cur,const unsigned char* src_end,unsigned char value){
            while (cur!=src_end) {
                if (*cur!=value)
                    return cur;
                ++cur;
            }
            return src_end;
        }
        
        static void save(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int rle_parameter){
            assert(rle_parameter>=TBytesRle::kRle_size_bestSize);
            //assert(rle_parameter<=TBytesRle::kRle_size_bestUnRleSpeed);
            const int kRleMinZipSize=rle_parameter+1; //增大则压缩率变小,解压稍快; 1时,压缩率最大.
            const int kRleMinSameSize=kRleMinZipSize+1;
            std::vector<unsigned char> codeBuf;
            std::vector<unsigned char> ctrlBuf;
            
            const unsigned char* notSame=src;
            while (src!=src_end) {
                //find equal length
                unsigned char value=*src;
                const unsigned char* eqEnd=getEqualEnd(src+1,src_end,value);
                const int sameCount=(int)(eqEnd-src);
                if ( (sameCount>kRleMinSameSize) || ( (sameCount==kRleMinSameSize)&&( (value==0)||(value==255) ) ) ){//可以压缩.
                    if (notSame!=src){
                        pushNotSame(codeBuf,ctrlBuf,notSame,(TInt32)(src-notSame));
                    }
                    pushSame(codeBuf,ctrlBuf,value, sameCount);
                    
                    src+=sameCount;
                    notSame=src;
                }else{
                    src=eqEnd;
                }
            }
            if (notSame!=src_end){
                pushNotSame(codeBuf,ctrlBuf,notSame,(TInt32)(src_end-notSame));
            }
            
            pack32Bit(out_code,(TInt32)ctrlBuf.size());
            out_code.insert(out_code.end(),ctrlBuf.begin(),ctrlBuf.end());
            out_code.insert(out_code.end(),codeBuf.begin(),codeBuf.end());
        }
        
    private:
        static void pushSame(std::vector<unsigned char>& out_code,std::vector<unsigned char>& out_ctrl,unsigned char cur,TInt32 count){
            assert(count>0);
            TByteRleType type;
            if (cur==0)
                type=kByteRleType_rle0;
            else if (cur==255)
                type=kByteRleType_rle255;
            else
                type=kByteRleType_rle;
            const TInt32 packCount=count-1;
            pack32BitWithTag(out_ctrl,packCount, type,kByteRleType_bit);
            if (type==kByteRleType_rle)
                out_code.push_back(cur);
        }
        
        static void pushNotSame(std::vector<unsigned char>& out_code,std::vector<unsigned char>& out_ctrl,const unsigned char* byteStream,TInt32 count){
            assert(count>0);
            if (count==1){
                pushSame(out_code,out_ctrl,*byteStream,1);
                return;
            }
            
            pack32BitWithTag(out_ctrl,count-1, kByteRleType_unrle,kByteRleType_bit);
            out_code.insert(out_code.end(),byteStream,byteStream+count);
        }
    };
    
    
}//end namespace
} //end namespace frg


void TBytesRle::save(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int rle_parameter){
    frg::TBytesZiper_rle::save(out_code, src, src_end,rle_parameter);
}
