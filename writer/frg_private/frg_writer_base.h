//  frg_writer_base.h
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
#ifndef _LIBFRG_frg_writer_base_h
#define _LIBFRG_frg_writer_base_h

#include "../../reader/frg_private_reader_base.h"
#include "assert.h" //assert
#include "string.h" //memset
#include <vector>
#include <algorithm>
#include "pack_uint.h"
#include "../frg_writer.h"

//定义TFRG_map和TFRG_multimap; 可以尝试使用<unordered_map>或<hash_map>,编码速度略快.
#include <map>
#define TFRG_map        std::map
#define TFRG_multimap   std::multimap

namespace frg {
    
    typedef ptrdiff_t   TInt;
    typedef size_t      TUInt;
    typedef signed int  TInt32;
    
#ifdef _MSC_VER
    typedef    signed __int64      TInt64;
#else
    typedef    signed long long    TInt64;
#endif
    
static const TUInt32 kMaxImageWidth =(1<<16)-1;
static const TUInt32 kMaxImageHeight=kMaxImageWidth;

inline static TUInt32 packMatchXY(TUInt32 x,TUInt32 y){
    assert((x|y)<(1<<16));
    return x|(y<<16);
}
    
inline static int unpackMatchX(TUInt32 xy){
    return xy&((1<<16)-1);
}
inline static int unpackMatchY(TUInt32 xy){
    return xy>>16;
}
    
inline static TUInt32 SafeToUInt32(TUInt v,const char* _outLimit_errorMsg){
    if (sizeof(TUInt)==sizeof(TUInt32))
        return (TUInt32)v;
    TUInt32 v32=(TUInt32)v;
    if (v32!=v)
        throw TFrgRunTimeError(_outLimit_errorMsg);
    return v32;
}


inline static void writeUInt16(std::vector<TByte>& out_code,TUInt32 value){
    assert((value>>16)==0);
    out_code.push_back(value);
    out_code.push_back(value>>8);
}

inline static void writeUInt32(std::vector<TByte>& out_code,TUInt32 value){
    out_code.push_back(value);
    out_code.push_back(value>>8);
    out_code.push_back(value>>16);
    out_code.push_back(value>>24);
}

template<class TUInt>
inline static int packUIntWithTagOutSize(TUInt iValue,int kTagBit){//返回pack后字节大小.
    const unsigned int kMaxValueWithTag=(1<<(7-kTagBit))-1;
    int result=0;
    while (iValue>kMaxValueWithTag) {
        ++result;
        iValue>>=7;
    }
    return (result+1);
}
    
template<class TUInt>
static inline int packUIntOutSize(TUInt iValue){
    return packUIntWithTagOutSize(iValue, 0);
}

}//end namespace frg
#endif
