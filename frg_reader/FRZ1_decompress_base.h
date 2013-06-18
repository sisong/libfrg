//  FRZ1_decompress_base.h
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
#ifndef _FRZ1_DECOMPRESS_BASE_H_ 
#define _FRZ1_DECOMPRESS_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char   TFRZ_Byte;
typedef unsigned short  TFRZ_UInt16;
typedef signed int      TFRZ_Int32;
typedef unsigned int    TFRZ_UInt32;
    
enum TFRZCodeType{
    kFRZCodeType_nozip = 0,    //0表示后面存的未压缩数据 (包中连续储存多个字节数据)
    kFRZCodeType_zip   = 1     //1表示后面存的压缩(替代)数据.
};
static const int kFRZCodeType_bit=1;

#ifdef __cplusplus
}
#endif

#endif //_FRZ1_DECOMPRESS_BASE_H_
