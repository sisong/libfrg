// frg_colorTableZiper.h
// for frg_writer
// 计算生成调色板.
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
#ifndef __LIBFRG_FRG_COLOR_TABLE_H_
#define __LIBFRG_FRG_COLOR_TABLE_H_
#include "frg_color_base.h"
#include "../calc_color.h"

namespace frg{

    //处理调色板.
    class TColorTableZiper{
    public:
        static TUInt32 qualityToMatchColorMask(float colorQuality);
    public:
        TColorTableZiper(float colorQuality,bool isMustFitColorTable);
        void setImageSize(int imageWidth,int imageHeight);

        bool getBestColorTable(std::vector<Color24>& out_table,const TPixels32Ref& colors,const int maxTableSize)const;//计算最佳调色板.
        void getBestColorIndex(std::vector<TByte>& out_indexList,const Color24* table,int tableSize,const TPixels32Ref& subColors,int subX0,int subY0);//计算像素在调色板中的序号.
    public:
        struct TColorErrorParameter{
            TUInt32      minColorError;             //最大允许误差距离  当调色板过大时，允许删除颜色而产生的最大误差.
            TUInt32      minColorError_optimize;    //最大允许误差距离-优化  当调色板大小已经合适，允许继续删除颜色而产生的最大误差.
            TInt32       errorDiffuse_coefficient;  //误差扩散系数.
            TInt32       maxErrorDiffuseValue;      //最大扩散值.
            TInt32       maxTableSize;
            bool         isMustFitColorTable;
        };
        
        struct TColorNode {
        public:
            typedef TCalcColor<Color24,8,TInt32> TColor;
        private:
            TColor  m_sumColor;
            TColor  m_color;
            int     m_count;
        public:
           inline void setColor(const Color24& rgb,int count){
                m_color.setWithMul(TColor(rgb), 1);
                m_sumColor.setWithMul(TColor(rgb), count);
                m_count=count;
            }
            void uniteColor(const TColorNode& n);
           inline const TColor& getColor()const{ return m_color; }
           inline int getCount()const { return m_count; }
           inline  Color24 asColor24()const { return m_color.asColor(0,255); }
        };
        
        enum{ kColorErrorIntFloatBit=8};
        typedef TCalcColor<Color32,kColorErrorIntFloatBit,TInt32> TErrorColor;
    private:
        float                   m_colorQuality;
        TColorErrorParameter    m_errorParameter;
        TPixelsBufferBase<TPixelsRefBase<TErrorColor> > m_errorBuffer;
    private:
        static void  getColorSet(std::vector<TColorNode>* out_colorCountsList,const TPixels32Ref& colors);
        static void  deleteColor(std::vector<TColorNode>& colorSet,int maxTableSize,const TColorErrorParameter& errorParameter);
        static void  sortColorArrayForOut(std::vector<TColorNode>& colorSet);
        static void  sortColorArrayForOut(std::vector<Color24>& colorSet);
        static void  writeTable(std::vector<Color24>& out_table,std::vector<TColorNode>& colorSet);
    };

}//end namespace frg

#endif //__LIBFRG_FRG_COLOR_TABLE_H_