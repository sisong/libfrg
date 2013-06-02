// frg_match_table.h
//  for frg_write
// 局部调色板匹配.
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
#ifndef WH_LIBFRG_FRG_MATCH_TABLE_H_
#define WH_LIBFRG_FRG_MATCH_TABLE_H_
#include "frg_color_base.h"

namespace frg{

    class TTableMatch{
    public:
        explicit TTableMatch():m_subColors(1<<24,0),m_oldColorTableSize(0),m_colorMask(0xFFFFFF){}
        void initSetColorMask( TUInt32 colorMask ){ assert(m_oldColorTableSize==0); m_colorMask=colorMask;  }
        
        int findMatch(const std::vector<Color24>&  colorTable,const std::vector<Color24>& subTable,int* out_matchTableBit); //return -1 is not find
    private:
        std::vector<TByte>   m_subColors;
        std::vector<TUInt>  m_fashMatch4bit;
        //std::vector<UInt>  m_fashMatch3bit;
        //std::vector<UInt>  m_fashMatch2bit;
        //std::vector<UInt>  m_fashMatch1bit;
        int             m_oldColorTableSize;
        TUInt32          m_colorMask;
        int _findMatch(const std::vector<Color24>& colorTable,const std::vector<Color24>& subTable,const std::vector<TUInt>& fashMatch,int windowTableSize);
    };

}//end namespace frg

#endif //WH_LIBFRG_FRG_MATCH_TABLE_H_