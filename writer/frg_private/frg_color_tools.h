//  frg_color_tools.h
//  for frg_writer
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
#ifndef _LIBFRG_frg_color_tools_h
#define _LIBFRG_frg_color_tools_h

#include "frg_writer_base.h"

namespace frg{

struct PACKED TBGRA32 {
    TByte b;
    TByte g;
    TByte r;
    TByte a;
    inline TBGRA32(){}
    inline TBGRA32(const TBGRA32& v):b(v.b),g(v.g),r(v.r),a(v.a){}
    inline TBGRA32(TByte _r,TByte _g,TByte _b,TByte _a=255):b(_b),g(_g),r(_r),a(_a){}
    inline TUInt32 getBGRA()const{ return getBGR()|(a<<24); }
    inline TUInt32 getBGR()const{ return b|(g<<8)|(r<<16); }
};

template <class _TColorType>
struct TPixelsRefBase{
    typedef _TColorType     TColor;
    typedef _TColorType*    TPLineColor;
    
    TColor*     pColor;
    TInt32      width;
    TInt32      height;
    TInt32      byte_width;
    
    inline TPixelsRefBase():pColor(0),width(0),height(0),byte_width(0){}
    inline TColor&    pixels(int x,int y)const{ return getLinePixels(y)[x]; }
    inline bool getPixels(int x,int y,TColor* outColor)const{
        if ((x<0)||(y<0)||(x>=width)||(y>=height)) return false;
        *outColor=pixels(x,y);
        return true;
    }
    inline TColor* getLinePixels(int y)const{
        TByte* py=(TByte*)pColor+y*byte_width;
        return (TColor*)py;
    }
    TColor* nextLine(const TColor* pline)const { return (TColor*)((TByte*)pline+byte_width); }
    TColor* prevLine(const TColor* pline)const { return (TColor*)((TByte*)pline-byte_width); }
    inline bool getIsEmpty()const{
        return (width<=0)||(height<=0);
    }
};
    
    typedef TPixelsRefBase<TBGRA32> TPixels32Ref;
    
    void pixelsCopy(const TPixels32Ref& dst,const TPixels32Ref& src);
    bool getIsSigleRGBColor(const TPixels32Ref& src,TBGRA32* out_BGR);
    bool getIsSigleAlphaColor(const TPixels32Ref& src,TByte* out_Alpha);
    void delEmptyColor(const TPixels32Ref& dst);//alpha==0
    void pixelsFill(const TPixels32Ref& dst,TBGRA32 color);
    
/////

template <class TPixelsRefType>
class TPixelsBufferBase{
public:
    typedef typename TPixelsRefType::TColor TColor;
    
    inline TPixelsBufferBase(){ }
    inline TPixelsBufferBase(int width,int height){ resizeFast(width,height); }
    inline ~TPixelsBufferBase() { clear(); }
    void resizeFast(int width,int height) {
        assert((width>=0)&&(height>=0));
        if ((width==m_ref.width)&&(height==m_ref.height)) return;
        clear();
        m_ref.width=width;
        m_ref.height=height;
        m_ref.byte_width=width*sizeof(TColor);
        if (width*height>0)
            m_ref.pColor=new TColor[width*height];
    }
    inline const TPixelsRefType& getRef()const{ return m_ref; }
    inline void clear(){
        if (m_ref.pColor!=0) delete[] m_ref.pColor;
        memset(&m_ref,0,sizeof(m_ref));
    }
private:
    TPixelsRefType m_ref;
};
typedef TPixelsBufferBase<TPixels32Ref> TPixels32Buffer;

}//end namespace frg
    
#endif
