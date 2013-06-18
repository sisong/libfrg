//  FRZ1_decompress.h
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
#ifndef _FRZ1_DECOMPRESS_H_ 
#define _FRZ1_DECOMPRESS_H_

#ifdef __cplusplus
extern "C" {
#endif

//用来启用针对小内存的memcpy优化,要求允许CPU读写未对齐内存;速度影响较小(打开可能快10%以上,CPU平台是否可用和效果需要测试).
//#define FRZ_DECOMPRESS_USE_MEMCPY_TINY__MEM_NOTMUST_ALIGN
//x86 x64 powerpc 默认打开.
#ifndef FRZ_DECOMPRESS_USE_MEMCPY_TINY__MEM_NOTMUST_ALIGN
#   if defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64)\
      || defined(__386__) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_M_I386) || defined(__I86__) || defined(_I386)\
      || defined(__powerpc__) || defined(__powerpc) || defined(__ppc__) || defined(__PPC__) || defined(_M_PPC) || defined(_ARCH_PPC) || defined(_ARCH_PWR)
#       define FRZ_DECOMPRESS_USE_MEMCPY_TINY__MEM_NOTMUST_ALIGN
#   endif
#endif


#define frz_BOOL    int
#define frz_FALSE   0
//#define frz_TRUE    (!frz_FALSE)

frz_BOOL FRZ1_decompress     (unsigned char* out_data,unsigned char* out_data_end,
                              const unsigned char* frz1_code,const unsigned char* frz1_code_end);

frz_BOOL FRZ1_decompress_safe(unsigned char* out_data,unsigned char* out_data_end,
                              const unsigned char* frz1_code,const unsigned char* frz1_code_end);

    
#ifdef __cplusplus
}
#endif

#endif //_FRZ1_DECOMPRESS_H_
