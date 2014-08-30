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
    
    const int kMaxMatchLength=1024*16;//增大该值可能增大匹配机率(从而增大压缩率),但可能会降低压缩速度.
    
    static inline TUInt colorMatchHash(const Color24& color,TUInt32 colorMask){
        const TUInt32 v4=color.getBGR()&colorMask;
        TUInt hash=hash_value((const char*)&v4,sizeof(v4));
        //if (sizeof(TUInt)>sizeof(TUInt32)) hash^=(hash>>32);
        hash^=(hash>>16);
        hash^=(hash>>8);
        return ((TUInt)1)<<(hash & (sizeof(TUInt)*8-1) );
    }
    
    static void cacheFastMatch(std::vector<TUInt>& fastMatch,int matchSize,const std::vector<Color24>&  colorTable,TUInt oldColorTableSize,TUInt32 colorMask){
        TInt tabSize=colorTable.size();
        TInt oldFastMatchSize=fastMatch.size();
        fastMatch.resize(oldFastMatchSize+(tabSize-oldColorTableSize));
        for (TInt i=oldColorTableSize,j=oldFastMatchSize;i<tabSize;++i,++j){
            TUInt v=0;
            for (TInt h=i; (h>=0)&&(i-h+1<=matchSize);--h) {
                v|=colorMatchHash(colorTable[h],colorMask);
            }
            fastMatch[j]=v;
        }
        
        if (fastMatch.size()>=(TUInt)(kMaxMatchLength*2+kFrg_MaxSubTableSize)){
            fastMatch.erase(fastMatch.begin(),fastMatch.end()-(kMaxMatchLength+kFrg_MaxSubTableSize));
        }
    }
    
    static inline void setBit(TByte* bitVector,TUInt i,bool value){
        TByte& v8=bitVector[i>>3];
        if (value)
            v8|=1<<(i&7);
        else
            v8&=~(1<<(i&7));
    }
    
    static inline bool getBit(const TByte* bitVector,TUInt i){
        const TByte v8=bitVector[i>>3];
        return ( v8&(1<<(i&7)) )!=0;
    }

    int TTableMatch::_findMatch(const std::vector<Color24>& subTable,int windowTableSize){
        const TInt subSize=subTable.size();
        const TInt tabSize=m_colorTable.size();
        assert(subSize>1);
        TUInt sub_v=0;
        for (TInt h=0;h<subSize;++h)
            sub_v|=colorMatchHash(subTable[h],m_colorMask);
        
        TInt last_mhi=tabSize-kMaxMatchLength;
        if (last_mhi<0) last_mhi=0;
        TInt mi=-1;
        for (TInt hmi=tabSize-1,fastMatchi=(TInt)m_fastMatch4bitCache.size()-1; hmi>=last_mhi; --hmi,--fastMatchi) {
            if ((m_fastMatch4bitCache[fastMatchi]&sub_v)!=sub_v)
                continue;
            
            TInt min_mi=hmi-windowTableSize+1;
            if (min_mi<0) min_mi=0;
            //init set
            for (TInt i=min_mi;i<=hmi;++i){
                setBit(&m_subColorSets[0],m_colorTable[i].getBGR()&m_colorMask,true);
            }
            
            bool isMatch=true;
            for (TInt i=0;i<subSize;++i){
                if (!getBit(&m_subColorSets[0],subTable[i].getBGR()&m_colorMask)){
                    isMatch=false;
                    break;
                }
            }
            
            //reset set
            for (TInt i=min_mi;i<=hmi;++i)
                setBit(&m_subColorSets[0],m_colorTable[i].getBGR()&m_colorMask,false);
            
            if (isMatch){
                //find best mi
                for (TInt i=0;i<subSize;++i)
                    setBit(&m_subColorSets[0],subTable[i].getBGR()&m_colorMask,true);
                
                for (TInt i=hmi;i>=min_mi;--i){
                    int bindex=m_colorTable[i].getBGR()&m_colorMask;
                    if (!getBit(&m_subColorSets[0],bindex)) continue;
                    setBit(&m_subColorSets[0],bindex,false);
                    mi=i;
                }
                
                break; //ok
            }
        }
        return (int)mi;
    }
    
    int TTableMatch::findMatch(const std::vector<Color24>& subTable,int* out_matchTableBit){
        const int kWindowTableSize_4bit=(1<<4);  assert(kWindowTableSize_4bit==kFrg_MaxSubTableSize);
        const int kWindowTableSize_3bit=(1<<3);
        const int kWindowTableSize_2bit=(1<<2);
        const int kWindowTableSize_1bit=(1<<1);

        TInt subSize=(TInt)subTable.size();
        assert(subSize>0);
        TInt tabSize=(TInt)m_colorTable.size();
        if (tabSize>m_oldColorTableSize){
            cacheFastMatch(m_fastMatch4bitCache,kWindowTableSize_4bit,m_colorTable,m_oldColorTableSize,m_colorMask);
            m_oldColorTableSize=tabSize;
        }
        
        if (subSize==1){
            //只用支持较短的前匹配.
            Color24 singleColor=subTable[0];
            TInt mi=(TInt)m_colorTable.size()-1;
            for (TInt i=1;(mi>=0)&&(i<=kFrg_MaxShortForwardLength);--mi,++i){
                if (m_colorTable[mi].getBGR()==singleColor.getBGR()){
                    return (int)mi;
                }
            }
            return -1;
        }
        
        //匹配窗口.
        switch (subSize) {
            case 16:
            case 15:
            case 14:
            case 13:
            case 12:
            case 11:
            case 10:
            case  9:{
                *out_matchTableBit=4;
                return _findMatch(subTable,kWindowTableSize_4bit);
            } break;
            case 8:
            case 7:
            case 6:
            case 5:{
                *out_matchTableBit=3;
                int matchIndex=_findMatch(subTable,kWindowTableSize_3bit);
                if (matchIndex>=0)
                    return matchIndex;
                *out_matchTableBit=4; //还可能有利可图,继续增大匹配窗口.
                return _findMatch(subTable,kWindowTableSize_4bit);
            } break;
            case 4:
            case 3:{
                *out_matchTableBit=2;
                return _findMatch(subTable,kWindowTableSize_2bit);
            } break;
            case 2:{
                *out_matchTableBit=1;
                return _findMatch(subTable,kWindowTableSize_1bit);
            } break;
            default:{
                assert(false);
                return -1;
            } break;
        }
    }

}//end namespace frg
