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
#ifndef __COLOR_ERROR_DIFFUSE_H_
#define __COLOR_ERROR_DIFFUSE_H_
#include "calc_color.h"

namespace frg{
    
    //  type TSrcColors{
    //      type    TColor;
    //      type    TLineColor;
    //      intType width;
    //      intType height;
    //      TLineColor beginLine();
    //      TLineColor nextLine(TLineColor);
    //  };
    
    //  type TDstColors{
    //      type TErrorColor{  //TDstColors::TErrorColor can use TCalcColor<>
    //          TErrorColor();
    //          void clear();
    //          void add(TErrorColor);
    //          void add(TSrcColors::TColor);
    //          void sub(TSrcColors::TColor);
    //          void div(int divValue);
    //          void addWithMul(TErrorColor,int mulValue);
    //          void setWithMul(TErrorColor,int mulValue); //clear + addWithMul
    //      };
    //      void toBeginLine();
    //      void toNextLine();
    //      TSrcColors::TColor setColor(TErrorColor wantColor,Color24 srcColor);
    //      TErrorColor  optimizeErrorColor(TErrorColor);
    //  };
    
    
    //Floyd-Steinberg扩散模版的实现.
    //  * 7
    //3 5 1   /16  =>
    template<class TPixelsRef_ErrorColor,class TDstColors,class TSrcColors>
    class TErrorDiffuse{
    public:
        typedef typename TDstColors::TErrorColor    TErrorColor;
        typedef typename TSrcColors::TColor         TSrcColor;
        typedef typename TSrcColors::TPLineColor    TSrcPLineColor;
    private:
        struct TAutoPointer{
            inline TAutoPointer(TInt size):pointer(0){ pointer=new TErrorColor[size]; }
            inline ~TAutoPointer(){ if (pointer!=0) delete []pointer; }
            TErrorColor* pointer;
        };
    public:
        static void  errorDiffuse(TDstColors& dst,const  TSrcColors &  src){
            TAutoPointer _autoPointer((src.width+2)*2);
            TErrorColor* _HLineErr=_autoPointer.pointer;
            TErrorColor*  PrevHLineErr =&_HLineErr[1];
            TErrorColor*  NextHLineErr =&_HLineErr[1 + (src.width+2)];
            for (int i=0;i<src.width;++i)
                PrevHLineErr[i].clear();
            
            TSrcPLineColor pSrc=src.beginLine();
            dst.toBeginLine();
            for  (int y=0; y<src.height; ++y){
                NextHLineErr[-1].clear();
                NextHLineErr[ 0].clear();
                TErrorColor HErrCur; HErrCur.clear();
                errorDiffuse_Line(dst,pSrc,src.width,PrevHLineErr,NextHLineErr,&HErrCur);
                TErrorColor* _tmp=PrevHLineErr; PrevHLineErr=NextHLineErr; NextHLineErr=_tmp; //swap pointer
                pSrc= src.nextLine(pSrc);
                dst.toNextLine();
            }
        }
    public:
        static void  errorDiffuse_Line(TDstColors& dst,const TSrcPLineColor pSrc,int  width,TErrorColor*  PrevHLineErr,TErrorColor*  NextHLineErr,TErrorColor* HErrCur){
            TErrorColor& HErr=*HErrCur;
            for  (int  x = 0 ;x < width; ++ x) {
                TErrorColor& wantColor=HErr;
                wantColor.add(PrevHLineErr[x]);
                wantColor.add(pSrc[x]);
                TSrcColor resultColor=dst.setColor(wantColor,pSrc[x]);
                
                //HErr=wantColor;
                HErr.sub(resultColor); //get error
                HErr=dst.optimizeErrorColor(HErr);
                HErr.div(16);
                NextHLineErr[x + 1]=HErr;   //1/16
                NextHLineErr[x    ].addWithMul(HErr,5);//5/16
                NextHLineErr[x - 1].addWithMul(HErr,3); //3/16
                HErr.setWithMul(HErr,7);//7/16
            }
        }

    };
    

}//end namespace

#endif //__COLOR_ERROR_DIFFUSE_H_
