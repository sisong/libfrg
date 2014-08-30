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

       inline TUInt keyValue()const{
           return (TUInt)hash_value((const char*)this,sizeof(*this));
        }
    };

   inline static bool isSameColor(const Color32 cur,const Color32 match,TUInt32 colorMask){
        if (cur.a==0)
            return true;
        if (match.a==0)
            return false;
        return ((match.getBGR()&colorMask)==(cur.getBGR()&colorMask));
    }

    static bool isSameColor(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh,TUInt32 colorMask){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[x],colorMask))
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

    static bool isSameColor_left_right(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh,TUInt32 colorMask){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[(sw-1)-x],colorMask))
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

    static bool isSameColor_up_down(const TPixels32Ref& ref,TInt32 curX,TInt32 curY,TInt32 ox,TInt32 oy,TInt32 sw,TInt32 sh,TUInt32 colorMask){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy+(sh-1));
        for (TInt32 y=0;y<sh;++y){
            for (TInt32 x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[x],colorMask))
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

    ///

    template<class TFilterFunc>
    void filter(const TPixels32Ref& ref,TUInt32 colorMask,TFilterFunc filterFunc){
        if ((ref.width<kFrg_ClipWidth) || (ref.height<kFrg_ClipHeight))
            return;

        std::vector<TCountInfos> ci_xList(ref.width);
        for (TInt32 x=0;x<ref.width;++x){
            ci_xList[x].addVLine(ref,x,0,kFrg_ClipHeight-1,colorMask);
        }

        for (TInt32 y=0;y<=ref.height-kFrg_ClipHeight;++y){
            const TInt32 hy=y+kFrg_ClipHeight-1;
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
            for (TInt32 x=0;x<kFrg_ClipWidth-1;++x)
                ci_x0.addCount(ci_xList[x]);
            for (TInt32 x=0;x<=ref.width-kFrg_ClipWidth;++x){
                TInt32 vx=x+kFrg_ClipWidth-1;
                ci_x0.addCount(ci_xList[vx]);
                if (x>0) ci_x0.delCount(ci_xList[x-1]);
                if (filterFunc(ci_x0.keyValue(),x,y))
                    return;
            }
        }
    }


////////////////////////////////////////////

        struct TCreateMatchMap_filter{
            TColorMatch::TMatchMap&             m_matchMap;
            const TFRG_map<TUInt,int>&          m_nodeKeysSet;
            inline TCreateMatchMap_filter(const TFRG_map<TUInt,int>& nodeKeysSet,TColorMatch::TMatchMap& matchMap)
                :m_matchMap(matchMap),m_nodeKeysSet(nodeKeysSet){ }
            inline bool operator()(TUInt key,TInt32 x,TInt32 y){
                TFRG_map<TUInt,int>::const_iterator keyIt(m_nodeKeysSet.find(key));
                if (keyIt==m_nodeKeysSet.end())
                    return false;
                if (((TUInt32)x%kFrg_ClipWidth==0)&&((TUInt32)y%kFrg_ClipHeight==0)&&(keyIt->second==1)){//self node
                    return false;
                }
                m_matchMap.insert(TColorMatch::TMatchMap::value_type((TUInt32)key,packMatchXY(x, y)));
                return false;
            }
        };

    static void createNodeKeys(TFRG_map<TUInt,int>& out_nodeKeysSet,std::vector<TUInt32>& out_nodeKeys,
                               int nodeWidth,int nodeHeight,const TPixels32Ref& ref,TUInt32 colorMask){
        out_nodeKeysSet.clear();
        out_nodeKeys.resize(0);
        for (int y=0;y<ref.height;y+=kFrg_ClipHeight){
            for (int x=0; x<ref.width; x+=kFrg_ClipWidth) {
                if ((y+kFrg_ClipHeight<=ref.height)&&(x+kFrg_ClipWidth<=ref.width)){
                    TCountInfos ci;
                    ci.addRef(ref,x,y,x+kFrg_ClipWidth,y+kFrg_ClipHeight,colorMask);
                    TUInt key=ci.keyValue();
                    ++out_nodeKeysSet[key];
                    out_nodeKeys.push_back((TUInt32)key);
                }else{
                    out_nodeKeys.push_back(0);
                }
            }
        }
    }

    void TColorMatch::initColorMatch(const TPixels32Ref& ref,TUInt32 colorMask){
        assert(ref.width<(1<<16));
        assert(ref.height<(1<<16));
        m_ref=ref;
        m_colorMask=colorMask;

        //create MatchSets
        m_nodeKeys.clear();
        m_nodeWidth =(m_ref.width +kFrg_ClipWidth -1)/kFrg_ClipWidth;
        int nodeHeight=(m_ref.height+kFrg_ClipHeight-1)/kFrg_ClipHeight;
        TFRG_map<TUInt,int> out_nodeKeysSet;
        createNodeKeys(out_nodeKeysSet,m_nodeKeys,m_nodeWidth,nodeHeight,m_ref,m_colorMask);
        {
            TCreateMatchMap_filter  createMatchMap_filter(out_nodeKeysSet,m_matchMap);
            filter<TCreateMatchMap_filter&>(ref,colorMask,createMatchMap_filter);
        }
    }

    bool TColorMatch::isMatchAt(TInt32 subX0,TInt32 subY0,TInt32 subWidth,TInt32 subHeight,TInt32 match_x0,TInt32 match_y0,frg_TMatchType* out_matchType){
        if ((match_x0<0)||(match_x0+subWidth>m_ref.width)) return false;
        if ((match_y0<0)||(match_y0+subHeight>m_ref.height)) return false;
        if ((match_y0+subHeight<=subY0)||((match_y0<=subY0)&&(match_x0+subWidth<=subX0))){
            if (isSameColor(m_ref,subX0,subY0,match_x0,match_y0,subWidth,subHeight,m_colorMask)){
                if ((subWidth==kFrg_ClipWidth)&&isSameAlpha(m_ref,subX0,subY0,match_x0,match_y0,subWidth,subHeight))
                    *out_matchType=kFrg_MatchType_move_bgra_w8;
                else
                    *out_matchType=kFrg_MatchType_move_bgr;
                return true;
            }if (isSameColor_up_down(m_ref,subX0,subY0,match_x0,match_y0,subWidth,subHeight,m_colorMask)){
                if ((subWidth==kFrg_ClipWidth)&&isSameAlpha_up_down(m_ref,subX0,subY0,match_x0,match_y0,subWidth,subHeight))
                    *out_matchType=kFrg_MatchType_up_down_bgra_w8;
                else
                    *out_matchType=kFrg_MatchType_up_down_bgr;
                return true;
            }else if (isSameColor_left_right(m_ref,subX0,subY0,match_x0,match_y0,subWidth,subHeight,m_colorMask)){
                if ((subWidth==kFrg_ClipWidth)&&isSameAlpha_left_right(m_ref,subX0,subY0,match_x0,match_y0,subWidth,subHeight))
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

    bool TColorMatch::findMatch(TInt32 nodeX,TInt32 nodeY,TInt32* out_x0,TInt32* out_y0,frg_TMatchType* out_matchType){
        const TUInt32 keyValue=m_nodeKeys[nodeY*m_nodeWidth+nodeX];

        std::pair<TMatchMap::const_iterator,TMatchMap::const_iterator> itPair(m_matchMap.equal_range(keyValue));
        TMatchMap::const_iterator it(itPair.first);
        TMatchMap::const_iterator itEnd(itPair.second);
        if (it==itEnd) return false;

        const TInt32 subX0=nodeX*kFrg_ClipWidth;
        const TInt32 subY0=nodeY*kFrg_ClipHeight;
        bool isMatched=false;
        for (;it!=itEnd;++it){
            const TUInt32 packedXY=it->second;
            TInt32 cur_x0=unpackMatchX(packedXY);
            TInt32 cur_y0=unpackMatchY(packedXY);
            isMatched=isMatchAt(subX0,subY0,kFrg_ClipWidth,kFrg_ClipHeight,cur_x0,cur_y0,out_matchType);
            if (isMatched){
                *out_x0=cur_x0;
                *out_y0=cur_y0;
                break;// ok finded one; 也可以继续寻找更好的匹配,但可能会很慢.
            }
        }
        return isMatched;
    }

}//end namespace frg
