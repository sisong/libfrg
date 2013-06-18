// frg_match_table.cpp
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
#include "frg_match_table.h"

namespace frg{
    
    static inline TUInt colorMatchHash(const Color24& color,TUInt32 colorMask){
        const TUInt32 v4=color.getBGR()&colorMask;
        TUInt32 hash=(TUInt32)hash_value((const char*)&v4,sizeof(v4));
        hash^=(hash>>16);
        hash^=(hash>>8);
        return ((TUInt)1)<<(hash & (sizeof(TUInt)*8-1) );
    }
    
    
    static void createFastMatch(std::vector<TUInt>& fastMatch,int matchSize,const std::vector<Color24>&  colorTable,int oldColorTableSize,TUInt32 colorMask){
        int tabSize=(int)colorTable.size();
        fastMatch.resize(tabSize);
        for (int i=oldColorTableSize;i<tabSize;++i){
            TUInt v=0;
            for (int h=i; (h>=0)&&(i-h+1<=matchSize);--h) {
                v|=colorMatchHash(colorTable[h],colorMask);
            }
            fastMatch[i]=v;
        }
    }

    int TTableMatch::_findMatch(const std::vector<Color24>&  colorTable,const std::vector<Color24>& subTable,const std::vector<TUInt>& fashMatch,int windowTableSize){
        const int subSize=(int)subTable.size();
        const int tabSize=(int)colorTable.size();
        assert(subSize>1);
        TUInt sub_v=0;
        for (int h=0;h<subSize;++h)
            sub_v|=colorMatchHash(subTable[h],m_colorMask);
        
        const int kMaxMatchLength=1024*64;//增大该值可能增大匹配机率(从而增大压缩率),但会降低压缩速度.
        int last_mhi=tabSize-kMaxMatchLength;
        if (last_mhi<0) last_mhi=0;
        int mi=-1;
        for (int hmi=tabSize-1; hmi>=last_mhi; --hmi) {
            if ((fashMatch[hmi]&sub_v)!=sub_v)
                continue;
            
            int min_mi=hmi-windowTableSize+1;
            if (min_mi<0) min_mi=0;
            //init set
            for (int i=min_mi;i<=hmi;++i){
                m_subColors[colorTable[i].getBGR()&m_colorMask]=1;
            }
            
            bool isMatch=true;
            for (int i=0;i<subSize;++i){
                if (m_subColors[subTable[i].getBGR()&m_colorMask]==0){
                    isMatch=false;
                    break;
                }
            }
            
            //reset set
            for (int i=min_mi;i<=hmi;++i)
                m_subColors[colorTable[i].getBGR()&m_colorMask]=0;
            
            if (isMatch){
                //find best mi
                for (int i=0;i<subSize;++i)
                    m_subColors[subTable[i].getBGR()&m_colorMask]=1;
                
                for (int i=hmi;i>=min_mi;--i){
                    TByte& ct=m_subColors[colorTable[i].getBGR()&m_colorMask];
                    if (ct==0) continue;
                    ct=0;
                    mi=i;
                }
                
                break; //ok
            }
        }
        return mi;
    }
    
    int TTableMatch::findMatch(const std::vector<Color24>&  colorTable,const std::vector<Color24>& subTable,int* out_matchTableBit){
        const int kWindowTableSize_4bit=kFrg_MaxSubTableSize; //16;
        const int kWindowTableSize_3bit=(1<<3);
        const int kWindowTableSize_2bit=(1<<2);
        const int kWindowTableSize_1bit=(1<<1);

        int subSize=(int)subTable.size();
        assert(subSize>0);
        int tabSize=(int)colorTable.size();
        if (tabSize>m_oldColorTableSize){
            createFastMatch(m_fashMatch4bit,kWindowTableSize_4bit,colorTable,m_oldColorTableSize,m_colorMask);
            //createFastMatch(m_fashMatch3bit,kWindowTableSize_3bit,colorTable,m_oldColorTableSize,m_colorMask);
            //createFastMatch(m_fashMatch2bit,kWindowTableSize_2bit,colorTable,m_oldColorTableSize,m_colorMask);
            //createFastMatch(m_fashMatch1bit,kWindowTableSize_1bit,colorTable,m_oldColorTableSize,m_colorMask);
            m_oldColorTableSize=tabSize;
        }
        
        if (subSize==1){
            //只用支持较短的前匹配.
            Color24 singleColor=subTable[0];
            int mi=(int)colorTable.size()-1;
            for (int i=1;(mi>=0)&&(i<=kFrg_MaxForwardLength);--mi,++i){
                if (colorTable[mi].getBGR()==singleColor.getBGR()){
                    return mi;
                }
            }
            return -1;
        }
        
        //匹配窗口.
        switch (subSize) {
            case 8:
            case 7:
            case 6:
            case 5:{
                *out_matchTableBit=3;
                int matchIndex=_findMatch(colorTable,subTable,m_fashMatch4bit,kWindowTableSize_3bit);
                if (matchIndex>=0)
                    return matchIndex;
                *out_matchTableBit=4; //还可能有利可图,继续增大匹配窗口.
                return _findMatch(colorTable,subTable,m_fashMatch4bit,kWindowTableSize_4bit);
            } break;
            case 4:
            case 3:{
                *out_matchTableBit=2;
                return _findMatch(colorTable,subTable,m_fashMatch4bit,kWindowTableSize_2bit);
            } break;
            case 2:{
                *out_matchTableBit=1;
                return _findMatch(colorTable,subTable,m_fashMatch4bit,kWindowTableSize_1bit);
            } break;
            default:{ //9..16
                assert((subSize>kWindowTableSize_3bit)&&(subSize<=kWindowTableSize_4bit));
                *out_matchTableBit=4;
                return _findMatch(colorTable,subTable,m_fashMatch4bit,kWindowTableSize_4bit);
            } break;
        }
    }

}//end namespace frg
