//clac_color.h
//计算过程中的颜色.
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
#ifndef __CALC_COLOR_H_
#define __CALC_COLOR_H_

namespace frg{

    template<class TBaseColor,int _kIntFloatBit,class TValueType=int>
    struct TCalcColor{
    public:
        enum { kIntFloatBit=_kIntFloatBit };
        typedef TCalcColor<TBaseColor,kIntFloatBit,TValueType> SelfType;
        
        TValueType b;
        TValueType g;
        TValueType r;
        inline TCalcColor(){}
        inline explicit TCalcColor(const TBaseColor& rgb) {
            r=fromCalcColorValue<0>(rgb.r);
            g=fromCalcColorValue<0>(rgb.g);
            b=fromCalcColorValue<0>(rgb.b);
        }
        inline TCalcColor(const SelfType& clColor):b(clColor.b),g(clColor.g),r(clColor.r){}
        template<int color_IntFloatBit>
        inline explicit TCalcColor(const TCalcColor<TBaseColor,color_IntFloatBit>& clColor){
            fromCalcColor<color_IntFloatBit>(clColor); }
        inline void clear() { r=0; g=0;b=0; }
        
        inline TBaseColor asColor()const{ return TBaseColor(toFitColor8(r),toFitColor8(g),toFitColor8(b)); }
        template <int color_IntFloatBit>
        inline TCalcColor<TBaseColor,color_IntFloatBit> asCalcColor()const{
            return TCalcColor<TBaseColor,color_IntFloatBit>(toCalcColor<color_IntFloatBit>(r),
                                                            toCalcColor<color_IntFloatBit>(g),toCalcColor<color_IntFloatBit>(b));
        }
        template <int color_IntFloatBit>
        inline void fromCalcColor(const SelfType& clColor){
            r=fromCalcColorValue<color_IntFloatBit>(clColor.r);
            g=fromCalcColorValue<color_IntFloatBit>(clColor.g);
            b=fromCalcColorValue<color_IntFloatBit>(clColor.b);
        }
    public:
        inline void add(const SelfType& clColor) { r+=clColor.r; g+=clColor.g; b+=clColor.b; }
        inline void add(const TBaseColor& rgb) {
            r+=rgb.r*(1<<kIntFloatBit); g+=rgb.g*(1<<kIntFloatBit); b+=rgb.b*(1<<kIntFloatBit); }
        inline void sub(const TBaseColor& rgb) {
            r-=rgb.r*(1<<kIntFloatBit); g-=rgb.g*(1<<kIntFloatBit); b-=rgb.b*(1<<kIntFloatBit); }
        inline void mul(int mulValue) { r*=mulValue; g*=mulValue; b*=mulValue;  }
        inline void div(int divValue) { r/=divValue; g/=divValue; b/=divValue;  }
        inline void divRound(int divValue) {
            const int half=divValue>>1; r=(r+half)/divValue; g=(g+half)/divValue; b=(b+half)/divValue;  }
        inline void sar(int sarValue) { r>>=sarValue; g>>=sarValue; b>>=sarValue;  }
        inline void addWithMul(const SelfType& clColor,int mulValue) {
            r+=clColor.r*mulValue; g+=clColor.g*mulValue; b+=clColor.b*mulValue; }
        inline void setWithMul(const SelfType& clColor,int mulValue) {
            r=clColor.r*mulValue; g=clColor.g*mulValue; b=clColor.b*mulValue; }
        
        inline void min(const TBaseColor& rgb) {
            if (rgb.r*(1<<kIntFloatBit)<r) r=rgb.r*(1<<kIntFloatBit);
            if (rgb.g*(1<<kIntFloatBit)<g) g=rgb.r*(1<<kIntFloatBit);
            if (rgb.b*(1<<kIntFloatBit)<b) b=rgb.b*(1<<kIntFloatBit); }
        inline void max(const TBaseColor& rgb) {
            if (rgb.r*(1<<kIntFloatBit)>r) r=rgb.r*(1<<kIntFloatBit);
            if (rgb.g*(1<<kIntFloatBit)>g) g=rgb.r*(1<<kIntFloatBit);
            if (rgb.b*(1<<kIntFloatBit)>b) b=rgb.b*(1<<kIntFloatBit); }
    public:
        inline static int toFitColor8(TValueType selfColor) {
            selfColor>>=kIntFloatBit;
            if (selfColor<=0)
                return 0;
            else if (selfColor>=255)
                return 255;
            else
                return selfColor;
        }
        template <int out_color_IntFloatBit>
        inline static TValueType toCalcColor(TValueType selfColor) {
            if (out_color_IntFloatBit<=kIntFloatBit)
                return sar(selfColor,kIntFloatBit-out_color_IntFloatBit);
            else
                return shl(selfColor,out_color_IntFloatBit-kIntFloatBit);
        }
        template <int src_color_IntFloatBit>
        inline static TValueType fromCalcColorValue(TValueType srcColor) {
            if (src_color_IntFloatBit<=kIntFloatBit)
                return shl(srcColor,kIntFloatBit-src_color_IntFloatBit);
            else
                return sar(srcColor,src_color_IntFloatBit-kIntFloatBit);
        }
    private:
        inline static TValueType shl(TValueType srcColor,int bit) {
            return srcColor<<bit;
        }
        inline static TValueType sar(TValueType srcColor,int bit) {
            return srcColor>>bit;
        }
    };
    
    
    template<int kIntFloatBit,class TValueType=int>
    struct TCalcGray{
    public:
        //enum { kIntFloatBit=_kIntFloatBit };
        typedef TCalcGray<kIntFloatBit,TValueType> SelfType;
        
        TValueType gray;
        inline TCalcGray(){}
        inline explicit TCalcGray(TValueType gray_intFloat):gray(gray_intFloat){}
        inline TCalcGray(const SelfType& clColor):gray(clColor.gray){}
        template<int color_IntFloatBit>
        inline explicit TCalcGray(const TCalcGray<color_IntFloatBit>& clColor){ fromCalcColor<color_IntFloatBit>(clColor); }
        inline void clear() { gray=0; }
        
        inline unsigned int asGray8()const { return TCalcColor<int,kIntFloatBit>::toFitColor8(gray); }
        template <int color_IntFloatBit>
        inline TCalcGray<color_IntFloatBit> asCalcColor()const{
            return TCalcGray<color_IntFloatBit>(TCalcColor<int,kIntFloatBit>::toCalcColor<color_IntFloatBit>(gray));
        }
        template <int color_IntFloatBit>
        inline void fromCalcColor(const TCalcGray<color_IntFloatBit>& clColor){
            gray=TCalcColor<int,kIntFloatBit>::fromCalcColorValue<color_IntFloatBit>(clColor.gray);
        }
    public:
        inline void add(const SelfType& clColor) { gray+=clColor.gray; }
        inline void add(const unsigned int gray8) { gray+=gray8*(1<<kIntFloatBit); }
        inline void sub(const unsigned int gray8) { gray-=gray8*(1<<kIntFloatBit); }
        inline void div(int divValue) { gray/=divValue; }
        inline void sar(int sarValue) { gray>>=sarValue; }
        inline void addWithMul(const SelfType& clColor,int mulValue) { gray+=clColor.gray*mulValue; }
        inline void setWithMul(const SelfType& clColor,int mulValue) { gray=clColor.gray*mulValue; }
        inline void min(const unsigned int gray8) { if (gray8*(1<<kIntFloatBit)<gray) gray=gray8*(1<<kIntFloatBit); }
        inline void max(const unsigned int gray8) { if (gray8*(1<<kIntFloatBit)>gray) gray=gray8*(1<<kIntFloatBit); }
    };


}//end namespace

#endif //__CALC_COLOR_H_
