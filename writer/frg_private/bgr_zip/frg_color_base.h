//  frg_color_base.h
//  for frg_writer
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
#ifndef _LIBFRG_frg_color_base_h
#define _LIBFRG_frg_color_base_h

#include "../frg_color_tools.h"

namespace frg{
    
    typedef TBGRA32 Color32;
    struct PACKED Color24{
        TByte b;
        TByte g;
        TByte r;
        inline Color24(){}
        inline Color24(const Color24& v):b(v.b),g(v.g),r(v.r){}
        inline Color24(TByte _r,TByte _g,TByte _b):b(_b),g(_g),r(_r){}
        inline TUInt32 getBGR()const{ return b|(g<<8)|(r<<16); }
    };
    
    typedef size_t          TUInt;
    typedef ptrdiff_t       TInt;
    
#ifdef _MSC_VER
    typedef    signed __int64      TInt64;
    typedef    unsigned __int64    TUInt64;
#else
    typedef    signed long long    TInt64;
    typedef    unsigned long long  TUInt64;
#endif 

    template<class T>
    inline static const T sqr(const T& a){
        return a*a;
    }
    
	inline static size_t hash_value(const char* str,const char* strEnd){
		size_t result = 2166136261U;
		while(str!=strEnd){
            result = ((result<<5)-result) + (*(const unsigned char*)str);
			++str;
		}
		return result;
	}
	inline static size_t hash_value(const char* str,int strSize){
        return hash_value(str,str+strSize);
    }

}//end namespace frg

#endif
