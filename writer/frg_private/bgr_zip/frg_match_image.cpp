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
        TUInt   xorValue;

        TCountInfos():
            sum(0),xorValue(0) {}
        TCountInfos(const TCountInfos& ci):
            sum(ci.sum),xorValue(ci.xorValue) {}
        inline static TUInt ToXorValue(TUInt32 bgr){
            TUInt v=hash_value((const char*)&bgr,3);
            return v ^ (v << (v&15));
        }
        
        inline void addColor(const Color32& color,TUInt32 colorMask){
            //Color count limit: assert(kFrg_ClipWidth*kFrg_ClipHeight<=256);
            TUInt32 bgr=color.getBGR()&colorMask;
            sum+=bgr;
            xorValue^=ToXorValue(bgr);
        }
        inline void delColor(const Color32& color,TUInt32 colorMask){
            TUInt32 bgr=color.getBGR()&colorMask;
            sum-=bgr;
            xorValue^=ToXorValue(bgr);
        }
        inline void addHLine(const TPixels32Ref& ref,int y,int x0,int x1,TUInt32 colorMask){
            const Color32* pixelsLine=ref.getLinePixels(y);
            for (int x=x0;x<x1;++x)
                addColor(pixelsLine[x],colorMask);
        }
        inline void delHLine(const TPixels32Ref& ref,int y,int x0,int x1,TUInt32 colorMask){
            const Color32* pixelsLine=ref.getLinePixels(y);
            for (int x=x0;x<x1;++x)
                delColor(pixelsLine[x],colorMask);
        }
        inline void addVLine(const TPixels32Ref& ref,int x,int y0,int y1,TUInt32 colorMask){
            const Color32* pixel=&ref.pixels(x,y0);
            for (int y=y0;y<y1;++y){
                addColor(*pixel,colorMask);
                pixel=ref.nextLine(pixel);
            }
        }
        inline void delVLine(const TPixels32Ref& ref,int x,int y0,int y1,TUInt32 colorMask){
            const Color32* pixel=&ref.pixels(x,y0);
            for (int y=y0;y<y1;++y){
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

        void addRef(const TPixels32Ref& ref,int x0,int y0,int x1,int y1,TUInt32 colorMask){
            for (int y=y0;y<y1;++y){
                addHLine(ref,y,x0,x1,colorMask);
            }
        }
        inline void addRef(const TPixels32Ref& ref,TUInt32 colorMask){
            addRef(ref,0,0,ref.width,ref.height,colorMask);
        }

        inline TUInt keyValue()const{
            return xorValue ^ sum;
        }
    };

   inline static bool isSameColor(const Color32 cur,const Color32 match,TUInt32 colorMask){
        if (cur.a==0)
            return true;
        if (match.a==0)
            return false;
        return ((match.getBGR()&colorMask)==(cur.getBGR()&colorMask));
    }

    static bool isSameColor(const TPixels32Ref& ref,int curX,int curY,int ox,int oy,int sw,int sh,TUInt32 colorMask){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (int y=0;y<sh;++y){
            for (int x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[x],colorMask))
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.nextLine(pixelLine1);
        }
        return true;
    }

    static bool isSameAlpha(const TPixels32Ref& ref,int curX,int curY,int ox,int oy,int sw,int sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (int y=0;y<sh;++y){
            for (int x=0;x<sw;++x){
                if (pixelLine0[x].a!=pixelLine1[x].a)
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.nextLine(pixelLine1);
        }
        return true;
    }

    static bool isSameColor_left_right(const TPixels32Ref& ref,int curX,int curY,int ox,int oy,int sw,int sh,TUInt32 colorMask){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (int y=0;y<sh;++y){
            for (int x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[(sw-1)-x],colorMask))
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.nextLine(pixelLine1);
        }
        return true;
    }

    static bool isSameAlpha_left_right(const TPixels32Ref& ref,int curX,int curY,int ox,int oy,int sw,int sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy);
        for (int y=0;y<sh;++y){
            for (int x=0;x<sw;++x){
                if (pixelLine0[x].a!=pixelLine1[(sw-1)-x].a)
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.nextLine(pixelLine1);
        }
        return true;
    }

    static bool isSameColor_up_down(const TPixels32Ref& ref,int curX,int curY,int ox,int oy,int sw,int sh,TUInt32 colorMask){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy+(sh-1));
        for (int y=0;y<sh;++y){
            for (int x=0;x<sw;++x){
                if (!isSameColor(pixelLine0[x],pixelLine1[x],colorMask))
                    return false;
            }
            pixelLine0=ref.nextLine(pixelLine0);
            pixelLine1=ref.prevLine(pixelLine1);
        }
        return true;
    }

    static bool isSameAlpha_up_down(const TPixels32Ref& ref,int curX,int curY,int ox,int oy,int sw,int sh){
        const Color32* pixelLine0=&ref.pixels(curX,curY);
        const Color32* pixelLine1=&ref.pixels(ox,oy+(sh-1));
        for (int y=0;y<sh;++y){
            for (int x=0;x<sw;++x){
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
    void filterCountInfos(const TPixels32Ref& ref,TUInt32 colorMask,TFilterFunc filterFunc){
        if ((ref.width<kFrg_ClipWidth) || (ref.height<kFrg_ClipHeight))
            return;

        std::vector<TCountInfos> ci_xList(ref.width);
        for (int x=0;x<ref.width;++x){
            ci_xList[x].addVLine(ref,x,0,kFrg_ClipHeight-1,colorMask);
        }

        for (int y=0;y<=ref.height-kFrg_ClipHeight;++y){
            const int hy=y+kFrg_ClipHeight-1;
            const Color32* pline=ref.getLinePixels(hy);
            for (int x=0;x<ref.width;++x)
                ci_xList[x].addColor(pline[x],colorMask);
            if(y>0){
                const Color32* npline=ref.getLinePixels(y-1);
                for (int x=0;x<ref.width;++x){
                    ci_xList[x].delColor(npline[x],colorMask);
                }
            }
            TCountInfos ci_x0;
            for (int x=0;x<kFrg_ClipWidth-1;++x)
                ci_x0.addCount(ci_xList[x]);
            for (int x=0;x<=ref.width-kFrg_ClipWidth;++x){
                int vx=x+kFrg_ClipWidth-1;
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
            inline bool operator()(TUInt key,int x,int y){
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
                    TPixels32Ref  sub_ref;
                    ref.fastGetSubRef(&sub_ref,x,y,x+kFrg_ClipWidth,y+kFrg_ClipHeight);
                    TBGRA32 _tmp;
                    if (!getIsSigleRGBColor(sub_ref,&_tmp)){
                        TCountInfos ci;
                        ci.addRef(sub_ref,colorMask);
                        TUInt key=ci.keyValue();
                        ++out_nodeKeysSet[key];
                        out_nodeKeys.push_back((TUInt32)key);
                    }else{
                        out_nodeKeys.push_back(0);
                    }
                }else{
                    out_nodeKeys.push_back(0);
                }
            }
        }
    }

    void TColorMatch::initColorMatch(){
        assert((TUInt)m_ref.width<=kMaxImageWidth);
        assert((TUInt)m_ref.height<=kMaxImageHeight);

        //create MatchSets
        m_nodeKeys.clear();
        m_nodeWidth =(m_ref.width +kFrg_ClipWidth -1)/kFrg_ClipWidth;
        int nodeHeight=(m_ref.height+kFrg_ClipHeight-1)/kFrg_ClipHeight;
        TFRG_map<TUInt,int> nodeKeysSet;
        createNodeKeys(nodeKeysSet,m_nodeKeys,m_nodeWidth,nodeHeight,m_ref,m_colorMask);
        TCreateMatchMap_filter  createMatchMap_filter(nodeKeysSet,m_matchMap);
        filterCountInfos<TCreateMatchMap_filter&>(m_ref,m_colorMask,createMatchMap_filter);
    }

    bool TColorMatch::isMatchAt(int subX0,int subY0,int subWidth,int subHeight,
                                int match_x0,int match_y0,frg_TMatchType* out_matchType)const{
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

    bool TColorMatch::findNodeMatch(int nodeX,int nodeY,int* out_x0,int* out_y0,frg_TMatchType* out_matchType)const{
        const TUInt32 keyValue=m_nodeKeys[nodeY*m_nodeWidth+nodeX];

        std::pair<TMatchMap::const_iterator,TMatchMap::const_iterator> itPair(m_matchMap.equal_range(keyValue));
        TMatchMap::const_iterator it(itPair.first);
        TMatchMap::const_iterator itEnd(itPair.second);
        if (it==itEnd) return false;

        const int subX0=nodeX*kFrg_ClipWidth;
        const int subY0=nodeY*kFrg_ClipHeight;
        bool isFindedMatch=false;
        frg_TMatchType best_matchType=(frg_TMatchType)(-1);
        int bestX0=-1;
        int bestY0=-1;
        for (;it!=itEnd;++it){
            int cur_x0=unpackMatchX(it->second);
            int cur_y0=unpackMatchY(it->second);
            frg_TMatchType cur_matchType;
            bool cur_isMatched=isMatchAt(subX0,subY0,kFrg_ClipWidth,kFrg_ClipHeight,cur_x0,cur_y0,&cur_matchType);
            if (cur_isMatched){
                isFindedMatch=true;
                bestX0=cur_x0;
                bestY0=cur_y0;
                best_matchType=cur_matchType;
                break;// ok finded one; 也可以继续寻找某种标准下更好的匹配,但可能会慢些.
            }
        }
        if (isFindedMatch){
            *out_x0=bestX0;
            *out_y0=bestY0;
            *out_matchType=best_matchType;
        }
        return isFindedMatch;
    }

}//end namespace frg
