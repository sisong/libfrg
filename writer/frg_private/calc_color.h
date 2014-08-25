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
            r=rgb.r*(1<<kIntFloatBit);
            g=rgb.g*(1<<kIntFloatBit);
            b=rgb.b*(1<<kIntFloatBit);
        }
        inline TCalcColor(const SelfType& clColor):b(clColor.b),g(clColor.g),r(clColor.r){}
        inline void clear() { r=0; g=0;b=0; }
        
        inline TBaseColor asColor(TValueType minColor,TValueType maxColor)const{
            return TBaseColor(toFitColor(r,minColor,maxColor),toFitColor(g,minColor,maxColor),toFitColor(b,minColor,maxColor)); }
    public:
        inline void add(const SelfType& clColor) { r+=clColor.r; g+=clColor.g; b+=clColor.b; }
        inline void add(const TBaseColor& rgb) { add(SelfType(rgb)); }
        inline void sub(const SelfType& clColor) { r-=clColor.r; g-=clColor.g; b-=clColor.b; }
        inline void sub(const TBaseColor& rgb) { sub(SelfType(rgb)); }
        inline void mul(int mulValue) { r*=mulValue; g*=mulValue; b*=mulValue;  }
        inline void div(int divValue) { r/=divValue; g/=divValue; b/=divValue;  }
        inline void divUseRound(int divValue) { int half=divValue>>1; r+=half; g+=half; b+=half; div(divValue); }
        inline void sar(int sarBit) { r>>=sarBit; g>>=sarBit; b>>=sarBit;  }
        inline void addWithMul(const SelfType& clColor,int mulValue) {
            r+=clColor.r*mulValue; g+=clColor.g*mulValue; b+=clColor.b*mulValue; }
        inline void setWithMul(const SelfType& clColor,int mulValue) {
            r=clColor.r*mulValue; g=clColor.g*mulValue; b=clColor.b*mulValue; }
        
        inline void min(const SelfType& clColor) {
            if (clColor.r<r) r=clColor.r;
            if (clColor.g<g) g=clColor.r;
            if (clColor.b<b) b=clColor.b; }
        inline void min(const TBaseColor& rgb) { min(SelfType(rgb)); }
        inline void max(const SelfType& clColor) {
            if (clColor.r>r) r=clColor.r;
            if (clColor.g>g) g=clColor.r;
            if (clColor.b>b) b=clColor.b; }
        inline void max(const TBaseColor& rgb) { max(SelfType(rgb)); }
    public:
        inline static int toFitColor(TValueType selfColor,TValueType minColor,TValueType maxColor) {
            selfColor>>=kIntFloatBit;
            if (selfColor<=minColor)
                return minColor;
            else if (selfColor>=maxColor)
                return maxColor;
            else
                return selfColor;
        }
    };

}//end namespace

#endif //__CALC_COLOR_H_
