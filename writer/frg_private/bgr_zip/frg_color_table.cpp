// frg_color_table.cpp
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
#include "frg_color_table.h"
#include "../color_error_diffuse.h"

namespace frg{

    /*
    struct TColorOctree{
        ...
    };*/



    //const double gray_r_coeff_f=0.299;
	//const double gray_g_coeff_f=0.587;
	//const double gray_b_coeff_f=0.114;
    const TInt32 gray_r_coeff=19;//(gray_r_coeff_f*(1<<6));
	const TInt32 gray_g_coeff=37;//(gray_g_coeff_f*(1<<6));
	const TInt32 gray_b_coeff=(1<<6)-gray_r_coeff-gray_g_coeff;//(gray_b_coeff_f*(1<<6));
    //颜色距离
    static inline TInt32 getColorDistance(const Color24& c0,const Color24& c1){
        return   sqr(c0.r-c1.r)*gray_r_coeff
                +sqr(c0.g-c1.g)*gray_g_coeff
                +sqr(c0.b-c1.b)*gray_b_coeff;
    }
    template<class TCalcColor>
    static inline TInt32 getColorDistance(const TCalcColor& c0,const TCalcColor& c1){
        return (TInt32)( (sqr((TInt64)c0.r-c1.r)*gray_r_coeff
                        + sqr((TInt64)c0.g-c1.g)*gray_g_coeff
                        + sqr((TInt64)c0.b-c1.b)*gray_b_coeff) >>(TCalcColor::kIntFloatBit*2) );
    }



    typedef TColorTableZiper::TColorErrorParameter TColorErrorParameter;

    static const int kColorErrorDiffuseCoefficientIntFloatBit=8;

    TUInt32 TColorTableZiper::getMatchColorMask(float colorQuality){
        //质量->有效bit数: 100->8.0  80->7.0   60->6.0  0->4.0
        float gBit_f;
        if (colorQuality>=80)
            gBit_f=7.0f+(colorQuality-80)*((8.0f-7.0f)/(100-80));
        else if (colorQuality>=60)
            gBit_f=6.0f+(colorQuality-60)*((7.0f-6.0f)/(80-60));
        else
            gBit_f=4.0f+(colorQuality-60)*((6.0f-4.0f)/(60-0));

        int gBit=(int)(gBit_f+0.5f);
        int rBit=(int)(8-((8-gBit_f)*1.5f)+0.5f);
        int bBit=(int)(8-((8-gBit_f)*2)+0.5f);
        if (gBit>8) gBit=8; else if (gBit<2) gBit=2;
        if (rBit>8) rBit=8; else if (rBit<2) rBit=2;
        if (bBit>8) bBit=8; else if (bBit<2) bBit=2;
        gBit=8-gBit; rBit=8-rBit; bBit=8-bBit;
        return (0xFF>>bBit<<bBit)|(0xFF>>gBit<<(gBit+8))|(0xFF>>rBit<<(rBit+16));
    }


    static void qualityToMinColorError(TColorErrorParameter* out_errorParameter,float& colorQuality,bool isMustFitColorTable){
        if (colorQuality>100) colorQuality=100;
        else if (colorQuality<0) colorQuality=0;

        const int kDiffuseCoeff0=(1<<kColorErrorDiffuseCoefficientIntFloatBit)*87/100;
        const int kDiffuseCoeff100=(1<<kColorErrorDiffuseCoefficientIntFloatBit)*97/100;
        out_errorParameter->errorDiffuse_coefficient=(int)(0.5f+kDiffuseCoeff0+(kDiffuseCoeff100-kDiffuseCoeff0)*colorQuality/100);

        const int kMaxDiffuseValue0=(1<<kColorErrorDiffuseCoefficientIntFloatBit)*(1<<5);
        const int kMaxDiffuseValue100=(1<<kColorErrorDiffuseCoefficientIntFloatBit)*(1<<2);
        out_errorParameter->maxErrorDiffuseValue=(int)(0.5f+kMaxDiffuseValue0+(kMaxDiffuseValue100-kMaxDiffuseValue0)*colorQuality/100);

        out_errorParameter->isMustFitColorTable=isMustFitColorTable;//colorQuality<=80;

        static TInt32 kSingleColorError0=getColorDistance(Color24(150,150,150),Color24(0,0,0));
        static TInt32 kSingleColorError60=getColorDistance(Color24((1<<5)-1,(1<<5)-1,(1<<6)-1),Color24(0,0,0));
        static TInt32 kSingleColorError80=getColorDistance(Color24((1<<3)-1,(1<<3)-1,(1<<3)-1),Color24(0,0,0));
        static TInt32 kSingleColorError100=0;

        /*static TInt32 kRandDiffuse0=(1<<4)*(1<<kColorErrorIntFloatBit);
        static TInt32 kRandDiffuse60=((1<<3)+(1<<2))*(1<<kColorErrorIntFloatBit);
        static TInt32 kRandDiffuse80=(1<<3)*(1<<kColorErrorIntFloatBit);
        static TInt32 kRandDiffuse100=(1)*(1<<kColorErrorIntFloatBit);*/

        static TInt32 kTableSize0=2;
        static TInt32 kTableSize60=12;
        static TInt32 kTableSize80=16;

        if ((80<=colorQuality)&&(colorQuality<=100)){
            out_errorParameter->minColorError=(int)(0.5f+(colorQuality-80)*(kSingleColorError100-kSingleColorError80)/(100-80)+kSingleColorError80);
            out_errorParameter->minColorError_optimize=(out_errorParameter->minColorError)>>1;
            out_errorParameter->maxTableSize=16;
            return;
        }else if ((60<=colorQuality)&&(colorQuality<80)){
            out_errorParameter->minColorError=(int)(0.5f+(colorQuality-60)*(kSingleColorError80-kSingleColorError60)/(80-60)+kSingleColorError60);
            out_errorParameter->minColorError_optimize=(out_errorParameter->minColorError)>>1;
            out_errorParameter->maxTableSize=(int)(0.5f+(colorQuality-60)*(kTableSize80-kTableSize60)/(80-60)+kTableSize60);
            return;
        }else if ((0<=colorQuality)&&(colorQuality<60)){
            out_errorParameter->minColorError=(int)(0.5f+(colorQuality-0)*(kSingleColorError60-kSingleColorError0)/(60-0)+kSingleColorError0);
            out_errorParameter->minColorError_optimize=(out_errorParameter->minColorError)>>1;
            out_errorParameter->maxTableSize=(int)(0.5f+(colorQuality-0)*(kTableSize60-kTableSize0)/(60-0)+kTableSize0);
            return;
        }
        assert(false);
    }


    //////////////////

    void TColorTableZiper::TColorNode::uniteColor(const TColorNode& n){
        m_sumColor.add(n.m_sumColor);
        m_count+=n.m_count;
        TColor bestColor=m_sumColor; bestColor.divRound(m_count);
        if (getColorDistance(bestColor,n.m_color)<getColorDistance(bestColor,m_color))
            m_color=n.m_color;
    }

    TColorTableZiper::TColorTableZiper(float colorQuality,bool isMustFitColorTable):m_colorQuality(colorQuality){
        qualityToMinColorError(&m_errorParameter,m_colorQuality,isMustFitColorTable);
    }


    static void clearErrorColors(const TPixelsRefBase<TColorTableZiper::TErrorColor>& dst){
        const int w=dst.width;
        TColorTableZiper::TErrorColor* pline=dst.pColor;
        for (int y=0;y<dst.height;++y){
            for (int x=0;x<w;++x){
                pline[x].clear();
            }
            pline=dst.nextLine(pline);
        }
    }

    void TColorTableZiper::setImageSize(int imageWidth,int imageHeight){
        m_errorBuffer.resizeFast(imageWidth+2,imageHeight+1);
        clearErrorColors(m_errorBuffer.getRef());
    }
    
    //int _temp_nums[64+1]={0};
    bool TColorTableZiper::getBestColorTable(std::vector<Color24>& out_table,const TPixels32Ref& colors,const int maxTableSize)const{
        std::vector<TColorNode> colorSet; //color<-->count
        getColorSet(&colorSet,colors);
        //++_temp_nums[colorSet.size()];
        
        if (m_errorParameter.minColorError==0){
            if (colorSet.size()<=(TUInt)maxTableSize){
                //ok
                sortColorArrayForOut(colorSet);
                writeTable(out_table,colorSet);
                return true;
            }else{
                out_table.clear();
                return false;
            }
        }

        //delete color
        deleteColor(colorSet,maxTableSize,m_errorParameter);
        if (colorSet.size()>(TUInt)maxTableSize)
            return false; //fail

        sortColorArrayForOut(colorSet);
        writeTable(out_table,colorSet);//ok
        return true;
    }

    /////////////////
    
    class TColorsDistance{//算法复杂度现在O(N*N).
    public:
        inline TColorsDistance(std::vector<TColorTableZiper::TColorNode>& colorSet)
            :m_colorSet(colorSet),m_colorCount((int)colorSet.size()){ }
        inline ~TColorsDistance(){ m_colorSet.resize(m_colorCount); }
        void initColorSet(std::vector<TColorTableZiper::TColorNode>* colorSet);
        inline int getColorCount()const{ return m_colorCount; }
        TInt32 getMinColorDistance(TInt32* out_index0,TInt32* out_index1)const;
        void  deleteAColor(TInt32 index0,TInt32 index1);
    private:
        std::vector<TColorTableZiper::TColorNode>&    m_colorSet;
        int                         m_colorCount;

        inline static TInt32 getNodeColorDistance(const TColorTableZiper::TColorNode& na,const TColorTableZiper::TColorNode& nb){
            return getColorDistance(na.getColor(),nb.getColor())*std::min(na.getCount(),nb.getCount());
        }
    };

    TInt32 TColorsDistance::getMinColorDistance(TInt32* out_index0,TInt32* out_index1)const{
        assert(m_colorCount>=2);
        
        TInt32 curMinDistance=((TUInt32)1<<31)-1;
        TInt32 curMinIndex0=-1;
        TInt32 curMinIndex1=-1;
        for (int index0=0;index0<m_colorCount-1;++index0){
            for (int index1=index0+1;index1<m_colorCount;++index1){
                TInt32 distance=getNodeColorDistance(m_colorSet[index0],m_colorSet[index1]);
                if (distance<curMinDistance){
                    curMinDistance=distance;
                    curMinIndex0=index0;
                    curMinIndex1=index1;
                }
            }
        }
        
        *out_index0=curMinIndex0;
        *out_index1=curMinIndex1;
        return curMinDistance;
        
    }
    
    void TColorsDistance::deleteAColor(TInt32 index0,TInt32 index1){
        assert(index0<index1);
        //del color
        m_colorSet[index0].uniteColor(m_colorSet[index1]);
        m_colorSet.erase(m_colorSet.begin()+index1);
        --m_colorCount;
    }
  
    ///

    void TColorTableZiper::deleteColor(std::vector<TColorNode>& colorSet,int maxTableSize,const TColorErrorParameter& errorParameter){
        maxTableSize=std::min(maxTableSize,errorParameter.maxTableSize);
        TColorsDistance colorsDistance(colorSet);
        TInt32 curMinColorError=0;
        while (colorsDistance.getColorCount()>maxTableSize) {
            TInt32 index0,index1;
            curMinColorError=colorsDistance.getMinColorDistance(&index0,&index1);
            if ((!errorParameter.isMustFitColorTable)&&(curMinColorError>errorParameter.minColorError))
                return; //fail;
            colorsDistance.deleteAColor(index0,index1);
        }

        //succeed,continue optimize size

        const TInt32 kMinTableSize =2;
        while ((curMinColorError<errorParameter.minColorError_optimize)&&(colorsDistance.getColorCount()>kMinTableSize)){
            TInt32 index0,index1;
            curMinColorError=colorsDistance.getMinColorDistance(&index0,&index1);
            if (curMinColorError>errorParameter.minColorError_optimize)
                return; //finish optimize;
            colorsDistance.deleteAColor(index0,index1);
        }
    }

    template <class TCalcColor>
    static int getABestColorIndex(const Color24* table,TInt32 tableSize,const TCalcColor& color){
        TInt32 curMinDistance=(1<<30)-1;
        TInt32 curMinIndex=-1;
        for (int index=0;index<tableSize;++index){
            TInt32 distance=getColorDistance(TCalcColor(Color32(table[index].r,table[index].g,table[index].b)),color);
            if (distance<curMinDistance){
                curMinDistance=distance;
                curMinIndex=index;
            }
        }
        assert(curMinIndex>=0);
        return curMinIndex;
    }

    struct TIndextColors{
    public:
        typedef TColorTableZiper::TErrorColor TErrorColor;
       inline void toBeginLine() { }//m_rand.setSeed(0);  }
       inline void toNextLine()  {   }
        Color24 setColor(const TErrorColor& wantColor,Color24 srcColor) {
            //wantColor引入随机误差.
            const TErrorColor& newColor=wantColor;
            //newColor.r=randError(wantColor.r);
            //newColor.g=randError(wantColor.g);
            //newColor.b=randError(wantColor.b);

            int index=getABestColorIndex(m_table,m_tableSize,newColor);
            m_out_indexList.push_back(index);
            return m_table[index];
        }
        inline Color32 setColor(const TErrorColor& wantColor,Color32 srcColor) {
            Color24 rt=setColor(wantColor,Color24(srcColor.r,srcColor.g,srcColor.b));
            return Color32(rt.r,rt.g,rt.b);
        }

        TErrorColor  optimizeErrorColor(const TErrorColor& eColor) {
            TErrorColor rt=eColor;
            rt.r=rt.r*m_errorParameter.errorDiffuse_coefficient/(1<<kColorErrorDiffuseCoefficientIntFloatBit);
            rt.g=rt.g*m_errorParameter.errorDiffuse_coefficient/(1<<kColorErrorDiffuseCoefficientIntFloatBit);
            rt.b=rt.b*m_errorParameter.errorDiffuse_coefficient/(1<<kColorErrorDiffuseCoefficientIntFloatBit);
            rt.r=borderError(rt.r);
            rt.g=borderError(rt.g);
            rt.b=borderError(rt.b);
            return rt;
        }

    public:
       inline TIndextColors(std::vector<TByte>& out_indexList,const Color24* table,TInt32 tableSize,const TColorErrorParameter& errorParameter)
            :m_out_indexList(out_indexList),m_table(table),m_tableSize(tableSize),m_errorParameter(errorParameter){

        }
    private:
        std::vector<TByte>&  m_out_indexList;
        const Color24*  m_table;
        TInt32           m_tableSize;
        TColorErrorParameter m_errorParameter;
        //TRand          m_rand;
        /*must_inline int randError(int c){
            return c;
            const int kRandCoefficient_div=100;
            int randError=m_rand.next_inRange(-m_errorParameter.errorDiffuse_randError,m_errorParameter.errorDiffuse_randError+1);
            //作用不大！
            return c+c*randError/m_errorParameter.errorDiffuse_randError/kRandCoefficient_div;
        }*/
       inline int borderError(int c){
            if (c<=(-m_errorParameter.maxErrorDiffuseValue)){
                int half=(c+m_errorParameter.maxErrorDiffuseValue)/2;
                return -m_errorParameter.maxErrorDiffuseValue+half;
            }else if (c>=(m_errorParameter.maxErrorDiffuseValue)){
                int half=(c-m_errorParameter.maxErrorDiffuseValue)/2;
                return m_errorParameter.maxErrorDiffuseValue+half;
            }else{
                return c;
            }
        }
    };


    void TColorTableZiper::getBestColorIndex(std::vector<TByte>& out_indexList,const Color24* table,TInt32 tableSize,const TPixels32Ref& subColors,int subX0,int subY0){
        out_indexList.clear();
        TIndextColors dstIndexs(out_indexList,table,tableSize,m_errorParameter);
        TErrorDiffuse<TPixelsRefBase<TIndextColors::TErrorColor>,TIndextColors,TPixels32Ref>::errorDiffuse(
                                                            dstIndexs,subColors,m_errorBuffer.getRef(),subX0+1,subY0);

        /* not uses error diffuse for test
        out_indexList.resize(subColors.width*subColors.height);
        const Color32* pline=subColors.pColor;
        TByte* out_index=&out_indexList[0];
        for (int y=0;y<subColors.height;++y){
            for (int x=0;x<subColors.width;++x){
                out_index[x]=getABestColorIndex(table,tableSize,TErrorColor(pline[x]));
            }
            out_index+=subColors.width;
            pline=subColors.nextLine(pline);
        }
        //*/
    }

    ////////

    void TColorTableZiper::getColorSet(std::vector<TColorNode>* out_colorSet,const TPixels32Ref& colors){
        TFRG_map<TUInt32,TUInt32> colorSet;
        const Color32* pline=colors.pColor;
        for (int y=0;y<colors.height;++y){
            for (int x=0; x<colors.width; ++x) {
                TUInt32 RGB=pline[x].getBGR();
                if (pline[x].a!=0)
                    ++colorSet[RGB];
            }
            pline=colors.nextLine(pline);
        }
        pline=colors.pColor;
        for (int y=0;y<colors.height;++y){
            for (int x=0; x<colors.width; ++x) {
                TUInt32 RGB=pline[x].getBGR();
                if (colorSet.find(RGB)==colorSet.end()) {
                    colorSet[RGB]=1;
                }
            }
            pline=colors.nextLine(pline);
        }

        int colorCountsSize=(int)colorSet.size();
        out_colorSet->resize(colorCountsSize);
        TFRG_map<TUInt32,TUInt32>::const_iterator it(colorSet.begin());
        for (int i=0;i<colorCountsSize;++i,++it){
            (*out_colorSet)[i].setColor(*(const Color24*)(&it->first),it->second);
        }
    }


       inline static bool color_sort_by(const Color24& a,const Color24& b){
            //return ((a.b<<16)|(a.g<<8)|(a.r)) < ((b.b<<16)|(b.g<<8)|(b.r));
            return (a.b+a.g+a.r)<(b.b+b.g+b.r);
            //return (a.b*2+a.g*4+a.r*3)<(b.b*2+b.g*4+b.r*3);
        }
       inline static bool ncolor_sort_by(const TColorTableZiper::TColorNode& na,const TColorTableZiper::TColorNode& nb){
            return color_sort_by(na.asColor24(),nb.asColor24());
        }
       inline static void swap(TColorTableZiper::TColorNode& na,TColorTableZiper::TColorNode& nb){
            TColorTableZiper::TColorNode tmp=na;  na=nb;  nb=tmp;
        }
    void TColorTableZiper::sortColorArrayForOut(std::vector<TColorNode>& colorSet){
        std::sort(colorSet.begin(),colorSet.end(),ncolor_sort_by);
    }
    void TColorTableZiper::sortColorArrayForOut(std::vector<Color24>& colorSet){
        std::sort(colorSet.begin(),colorSet.end(),color_sort_by);
    }

    void TColorTableZiper::writeTable(std::vector<Color24>& out_table,std::vector<TColorNode>& colorSet){
        int colorSetSize=(int)colorSet.size();
        out_table.resize(colorSetSize);
        for (int i=0;i<colorSetSize; ++i) {
            out_table[i]=colorSet[i].asColor24();
        }
    }

}//end namespace frg
