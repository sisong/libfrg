// frg_match_iamge.cpp
// for frg_writer
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
#include "frg_match_image.h"

namespace frg{

    struct TCountInfos{
        TUInt32 sum;
        TUInt32 xorValue;

        TCountInfos():
            sum(0),xorValue(0) {}
        TCountInfos(const TCountInfos& ci):
            sum(ci.sum),xorValue(ci.xorValue) {}
        inline void addColor(const Color32& color,TUInt32 colorMask){
            TUInt32 bgr=color.getBGR()&colorMask;
            sum+=bgr;
            xorValue^=bgr;
        }
        inline void delColor(const Color32& color,TUInt32 colorMask){
            TUInt32 bgr=color.getBGR()&colorMask;
            sum-=bgr;
            xorValue^=bgr;
        }
        inline void addHLine(const TPixels32Ref& ref,TInt32 y,TInt32 x0,TInt32 x1,TUInt32 colorMask){
            const Color32* pixelsLine=ref.getLinePixels(y);
            for (TInt32 x=x0;x<x1;++x)
                addColor(pixelsLine[x],colorMask);
        }
       inline void delHLine(const TPixels32Ref& ref,TInt32 y,TInt32 x0,TInt32 x1,TUInt32 colorMask){
            const Color32* pixelsLine=ref.getLinePixels(y);
            for (TInt32 x=x0;x<x1;++x)
                delColor(pixelsLine[x],colorMask);
        }
       inline void addVLine(const TPixels32Ref& ref,TInt32 x,TInt32 y0,TInt32 y1,TUInt32 colorMask){
            const Color32* pixel=&ref.pixels(x,y0);
            for (TInt32 y=y0;y<y1;++y){
                addColor(*pixel,colorMask);
                pixel=ref.nextLine(pixel);
            }
        }
       inline void delVLine(const TPixels32Ref& ref,TInt32 x,TInt32 y0,TInt32 y1,TUInt32 colorMask){
            const Color32* pixel=&ref.pixels(x,y0);
            for (TInt32 y=y0;y<y1;++y){
                delColor(*pixel,colorMask);
                pixel=ref.nextLine(pixel);
            }
        }
       inline void addCount(const TCountInfos& c){
            sum+=c.sum;
            xorValue^=c.xorValue;
        }
       inline void delCount(const TCountInfos& c){
            sum-=c.sum;
            xorValue^=c.xorValue;
        }

        void addRef(const TPixels32Ref& ref,TInt32 x0,TInt32 y0,TInt32 x1,TInt32 y1,TUInt32 colorMask){
            for (TInt32 y=y0;y<y1;++y){
                addHLine(ref,y,x0,x1,colorMask);
            }
        }
       inline void addRef(const TPixels32Ref& ref,TUInt32 colorMask){
            addRef(ref,0,0,ref.width,ref.height,colorMask);
        }

       inline TUInt32 keyValue()const{
           return (TUInt32)hash_value((const char*)this,sizeof(*this));
        }
    };
    
   inline static bool isSameColor(const Color32 cur,const Color32 match){
        if (cur.a==0)
            return true;
        if (match.a==0)
            return false;
        return (match.getBGR()==cur.getBGR());
    }
    
    static bool isSameColor(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[x]))
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.nextLine(pixelLine1);
        }
        return true;
    }
    
    static bool isSameAlpha(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (pixelLine0[x].a!=pixelLine1[x].a)
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.nextLine(pixelLine1);
        }
        return true;
    }

    static bool isSameColor_left_right(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[(sw-1)-x]))
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.nextLine(pixelLine1);
        }
        return true;
    }
    
    static bool isSameAlpha_left_right(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (pixelLine0[x].a!=pixelLine1[(sw-1)-x].a)
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.nextLine(pixelLine1);
        }
        return true;
    }
    
    static bool isSameColor_up_down(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy+(sh-1));
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[x]))
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.prevLine(pixelLine1);
        }
        return true;
    }
    
    static bool isSameAlpha_up_down(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy+(sh-1));
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (pixelLine0[x].a!=pixelLine1[x].a)
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.prevLine(pixelLine1);
        }
        return true;
    }

////////////////////////////////////////////

    void TColorMatch::initColorMatch(const TPixels32Ref& ref,TInt32 subWidth,TInt32 subHeight,TUInt32 colorMask){
        m_ref=ref;
        m_matchSubWidth=subWidth;
        m_matchSubHeight=subHeight;
        m_colorMask=colorMask;
        createMatchMap(m_ref,m_matchSubWidth,m_matchSubHeight,m_matchMap,m_colorMask);
    }
       
    bool TColorMatch::isMatchAt(TInt32 subX0,TInt32 subY0,TInt32 subWidth,TInt32 subHeight,TInt32 match_x0,TInt32 match_y0,frg_TMatchType* out_matchType){
        const TInt32 sw=subWidth;
        const TInt32 sh=subHeight;
        if (match_x0+sw>m_ref.width) return false;
        if (match_y0+sh>m_ref.height) return false;
        if ((match_y0+sh<=subY0)||((match_y0<=subY0)&&(match_x0+sw<=subX0))){
            if (isSameColor(m_ref,subX0,subY0,match_x0,match_y0,sw,sh)){
                if ((sw==kFrg_ClipWidth)&&isSameAlpha(m_ref,subX0,subY0,match_x0,match_y0,sw,sh))
                    *out_matchType=kFrg_MatchType_move_bgra_w8;
                else
                    *out_matchType=kFrg_MatchType_move_bgr;
                return true;
            }if (isSameColor_up_down(m_ref,subX0,subY0,match_x0,match_y0,sw,sh)){
                if ((sw==kFrg_ClipWidth)&&isSameAlpha_up_down(m_ref,subX0,subY0,match_x0,match_y0,sw,sh))
                    *out_matchType=kFrg_MatchType_up_down_bgra_w8;
                else
                    *out_matchType=kFrg_MatchType_up_down_bgr;
                return true;
            }else if (isSameColor_left_right(m_ref,subX0,subY0,match_x0,match_y0,sw,sh)){
                if ((sw==kFrg_ClipWidth)&&isSameAlpha_left_right(m_ref,subX0,subY0,match_x0,match_y0,sw,sh))
                    *out_matchType=kFrg_MatchType_left_right_bgra_w8;
                else
                    *out_matchType=kFrg_MatchType_left_right_bgr;
                return true;
            }else{
                return false;
            }
        }else{
            return false;
        }
    }

    bool TColorMatch::findMatch(TInt32 subX0,TInt32 subY0,TInt32 subWidth,TInt32 subHeight,TInt32* out_x0,TInt32* out_y0,frg_TMatchType* out_matchType){
        assert(subX0%kFrg_ClipWidth==0);
        assert(subY0%kFrg_ClipHeight==0);
        const TInt32 sw=subWidth;
        const TInt32 sh=subHeight;
        if ((sw!=m_matchSubWidth)||(sh!=m_matchSubHeight)) 
            return false;

        TUInt32 keyValue;
        {
            TCountInfos ci;
            ci.addRef(m_ref,subX0,subY0,subX0+sw,subY0+sh,m_colorMask);
            keyValue=ci.keyValue();
        }

        {
            std::pair<TMatchMap::const_iterator,TMatchMap::const_iterator> itPair(m_matchMap.equal_range(keyValue));
            TMatchMap::const_iterator it(itPair.first);
            TMatchMap::const_iterator itEnd(itPair.second);
            assert(it!=itEnd);
            //if (it==itEnd) return false;
            TInt32 minX0=-1;
            TInt32 minY0=-1;
            for (;it!=itEnd;++it){
                const TUInt32 packedXY=it->second;
                TInt32 ox=unpackMatchX(packedXY);
                TInt32 oy=unpackMatchY(packedXY);
                if ((oy+sh<=subY0)||((oy<=subY0)&&(ox+sw<=subX0))){
                    if (isSameColor(m_ref,subX0,subY0,ox,oy,sw,sh)){
                        if ((sw==kFrg_ClipWidth)&&isSameAlpha(m_ref,subX0,subY0,ox,oy,sw,sh))
                            *out_matchType=kFrg_MatchType_move_bgra_w8;
                        else
                            *out_matchType=kFrg_MatchType_move_bgr;
                        minX0=ox;
                        minY0=oy;
                        break;
                    }else if (isSameColor_up_down(m_ref,subX0,subY0,ox,oy,sw,sh)){
                        if ((sw==kFrg_ClipWidth)&&isSameAlpha_up_down(m_ref,subX0,subY0,ox,oy,sw,sh))
                            *out_matchType=kFrg_MatchType_up_down_bgra_w8;
                        else
                            *out_matchType=kFrg_MatchType_up_down_bgr;
                        minX0=ox;
                        minY0=oy;
                        break;
                    }else if (isSameColor_left_right(m_ref,subX0,subY0,ox,oy,sw,sh)){
                        if ((sw==kFrg_ClipWidth)&&isSameAlpha_left_right(m_ref,subX0,subY0,ox,oy,sw,sh))
                            *out_matchType=kFrg_MatchType_left_right_bgra_w8;
                        else
                            *out_matchType=kFrg_MatchType_left_right_bgr;
                        minX0=ox;
                        minY0=oy;
                        break;
                    }
                }
            }
            if (minX0>=0){
                *out_x0=minX0;
                *out_y0=minY0;
                return true;
            }else
                return false;
        }
    }

    void TColorMatch::createMatchMap(const TPixels32Ref& ref,TInt32 subWidth,TInt32 subHeight,TMatchMap& out_matchMap,TUInt32 colorMask){
        const TInt32 sw=subWidth;
        const TInt32 sh=subHeight;
        assert(sw>0);
        assert(sh>0);
        assert(sw<(1<<16));
        assert(sh<(1<<16));

        out_matchMap.clear();
        if ((ref.width>=subWidth)&&(ref.height>=subHeight)){
            std::vector<TCountInfos> ci_xList(ref.width);
            for (TInt32 x=0;x<ref.width;++x){
                ci_xList[x].addVLine(ref,x,0,sh-1,colorMask);
            }

            for (TInt32 y=0;y<=ref.height-sh;++y){
                const TInt32 hy=y+sh-1;
                const Color32* pline=ref.getLinePixels(hy);
                for (TInt32 x=0;x<ref.width;++x)
                    ci_xList[x].addColor(pline[x],colorMask);
                if(y>0){
                    const Color32* npline=ref.getLinePixels(y-1);
                    for (TInt32 x=0;x<ref.width;++x){
                        ci_xList[x].delColor(npline[x],colorMask);
                    }
                }
                TCountInfos ci_x0;
                for (TInt32 x=0;x<sw-1;++x)
                    ci_x0.addCount(ci_xList[x]);
                for (TInt32 x=0;x<=ref.width-sw;++x){
                    TInt32 vx=x+sw-1;
                    ci_x0.addCount(ci_xList[vx]);
                    if (x>0) ci_x0.delCount(ci_xList[x-1]);
                    out_matchMap.insert(TMatchMap::value_type(ci_x0.keyValue(),packMatchXY(x,y)));
                }
            }
        }
    }

}//end namespace frg
