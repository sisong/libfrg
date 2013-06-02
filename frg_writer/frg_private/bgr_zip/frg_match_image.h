// frg_match_image.h
// for frg_writer
// 2维图像的帧内匹配.
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
#ifndef WH_LIBFRG_FRG_MATCH_IMAGE_H_
#define WH_LIBFRG_FRG_MATCH_IMAGE_H_
#include "frg_color_base.h"

namespace frg{

    class TColorMatch{
    public:
        TColorMatch():m_matchSubWidth(0),m_matchSubHeight(0){}
        void initColorMatch(const TPixels32Ref& ref,TInt32 subWidth,TInt32 subHeight);
        bool findMatch(TInt32 subX0,TInt32 subY0,TInt32 subWidth,TInt32 subHeight,TInt32* out_x0,TInt32* out_y0,frg_TMatchType* out_matchType);
        bool isMatchAt(TInt32 subX0,TInt32 subY0,TInt32 subWidth,TInt32 subHeight,TInt32 match_x0,TInt32 match_y0,frg_TMatchType* out_matchType);
    private:
        typedef std::multimap<TUInt32,TUInt32> TMatchMap;

        TPixels32Ref     m_ref;
        TInt32           m_matchSubWidth;
        TInt32           m_matchSubHeight;
        TMatchMap       m_matchMap;

        static void createMatchMap(const TPixels32Ref& ref,TInt32 subWidth,TInt32 subHeight,TMatchMap& out_matchMaps);
    };

}//end namespace frg

#endif //WH_LIBFRG_FRG_MATCH_IMAGE_H_