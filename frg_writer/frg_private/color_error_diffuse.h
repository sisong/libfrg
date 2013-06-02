//color_error_diffuse.h
//误差扩散算法的一个实现. 
/*
 Copyright (c) 2012-2013 HouSisong All Rights Reserved.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef WH_COLOR_ERROR_DIFFUSE_H_
#define WH_COLOR_ERROR_DIFFUSE_H_
#include "calc_color.h"

namespace frg{
    
    
    //Floyd-Steinberg扩散模版的实现. 
    //  * 7
    //3 5 1   /16  =>
    
    //  type TSrcColors{
    //      type TColor;
    //      type TLineColor;
    //      value width;
    //      value height;
    //      TLineColor beginLine();
    //      TLineColor nextLine(TLineColor);
    //  };
    
    //  type TDstColors{
    //      type TErrorColor{
    //          TErrorColor();
    //          void clear();
    //          void add(TErrorColor);
    //          void add(TSrcColors::TColor);
    //          void sub(TSrcColors::TColor);
    //          template <int divValue> void div();
    //          template <int mulValue> void addWithMul(TErrorColor);
    //          template <int mulValue> void setWithMul(TErrorColor);
    //      };
    //      void toBeginLine();
    //      void toNextLine();
    //      TSrcColors::TColor setColor(TErrorColor wantColor,Color24 srcColor);
    //      TErrorColor  optimizeErrorColor(TErrorColor);
    //  };
    //>> TDstColors::TErrorColor can use TCalcColor<>
    
    template<class TPixelsRef_ErrorColor,class TDstColors,class TSrcColors>
    class TErrorDiffuse{
    public:
        typedef typename TDstColors::TErrorColor    TErrorColor;
        typedef typename TSrcColors::TColor         TSrcColor;
        typedef typename TSrcColors::TPLineColor    TSrcPLineColor;
        static void  errorDiffuse(TDstColors& dst,const  TSrcColors &  src){
            TErrorColor* _HLineErr=new TErrorColor[(src.width + 2)*2];
            TErrorColor*  HLineErr0 =&_HLineErr[ 1 ];
            TErrorColor*  HLineErr1 =&_HLineErr[ 1 + (src.width + 2) ];
            for (int i=0;i<src.width;++i)
                HLineErr0[i].clear();
            
            TSrcPLineColor pSrc=src.beginLine();
            dst.toBeginLine();
            for  ( int  y = 0 ;y < src.height; ++ y){
                _errorDiffuse_Line(dst,pSrc,src.width,HLineErr0,HLineErr1);
                TErrorColor* _tmp=HLineErr0; HLineErr0=HLineErr1; HLineErr1=_tmp; //swap
                pSrc= src.nextLine(pSrc);
                dst.toNextLine();
            }
            delete []_HLineErr;
        }
        static void  errorDiffuse(TDstColors& dst,const  TSrcColors &  src,const TPixelsRef_ErrorColor& tempErrorRef,int errorX0,int errorY0){
            assert(errorX0>=1);
            assert(errorY0>=0);
            assert(tempErrorRef.width-errorX0>=src.width+1);
            assert(tempErrorRef.height-errorY0>=src.height+1);
            TSrcPLineColor pSrc=src.pColor;
            TErrorColor*  HLineErr0 =&tempErrorRef.pixels(errorX0,errorY0);
            TErrorColor*  HLineErr1 =tempErrorRef.nextLine(HLineErr0);
            dst.toBeginLine();
            for  ( int  y = 0 ;y < src.height; ++ y){
                _errorDiffuse_Line(dst,pSrc,src.width,HLineErr0,HLineErr1);
                HLineErr0=HLineErr1; HLineErr1=tempErrorRef.nextLine(HLineErr1);
                pSrc= src.nextLine(pSrc);
                dst.toNextLine();
            }
        }
    private:
        static void  _errorDiffuse_Line(TDstColors& dst,const TSrcPLineColor pSrc,int  width,TErrorColor*  PHLineErr0,TErrorColor*  PHLineErr1){
            TErrorColor HErr;
            HErr.clear();
            PHLineErr1[ - 1 ].clear();
            PHLineErr1[ 0 ].clear();
            for  (int  x = 0 ;x < width; ++ x) {
                TErrorColor& wantColor=HErr;
                wantColor.add(PHLineErr0[x]);
                wantColor.add(pSrc[x]);
                TSrcColor resultColor=dst.setColor(wantColor,pSrc[x]);
                
                //HErr=wantColor;
                HErr.sub(resultColor); //get error
                HErr=dst.optimizeErrorColor(HErr);
                HErr.div(16);
                PHLineErr1[x + 1]=HErr;   //1/16
                PHLineErr1[x    ].addWithMul(HErr,5);//5/16
                PHLineErr1[x - 1].addWithMul(HErr,3); //3/16
                HErr.setWithMul(HErr,7);//7/16
            }
        }

    };
    

}//end namespace

#endif //WH_COLOR_ERROR_DIFFUSE_H_
