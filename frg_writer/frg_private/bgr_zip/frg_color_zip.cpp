// frg_color_zip.cpp
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
#include "frg_color_zip.h"
#include "../frg_color_tools.h"
#include "frg_color_table.h"
#include "frg_match_image.h"
//#include "frg_match_index.h"
#include "frg_match_table.h"

namespace frg{

    struct TClipNode{
        TPixels32Ref    colors;         //本块像素
        int             subX0;          //本块在源图片中的位置x
        int             subY0;          //本块在源图片中的位置y
        frg_TClipType   type;           //本块当前判定的类型
        TByte           matchTableBit;  
        TInt32          forwardLength;//type==kFrg_ClipType_single_* 向前匹配位置.
        frg_TMatchType  matchType;
        TByte           directcolorTypeData;
        int             colorMatchIndex;
        std::vector<Color24>    sub_table;//块调色板
        
        
        inline bool isSingleColorType()const{//单色.
            return (type==kFrg_ClipType_single_bgr)||(type==kFrg_ClipType_single_bgra_w8);
        }
        inline bool isDirectColorType()const{//无损压缩.
            return (type==kFrg_ClipType_directColor);
        }
        inline bool isMatchTableType()const{//颜色表匹配.
            return (type==kFrg_ClipType_match_table)||(type==kFrg_ClipType_match_table_single_a_w8);
        }
        inline bool isIndexType()const{//使用序号和调色板.
            return (type==kFrg_ClipType_index)||(type==kFrg_ClipType_index_single_a_w8);
        }
        
        TByte packNodeInfo()const{
            switch (type) {
                //case kFrg_ClipType_match_index:
                case kFrg_ClipType_index:
                case kFrg_ClipType_index_single_a_w8:
                    return (type<<4)|(sub_table.size()-1);
                    break;
                case kFrg_ClipType_single_bgr:
                case kFrg_ClipType_single_bgra_w8:
                    return (type<<4)|forwardLength;
                    break;
                case kFrg_ClipType_directColor:
                    return (type<<4)| directcolorTypeData;
                    break;
                case kFrg_ClipType_match_table_single_a_w8:
                case kFrg_ClipType_match_table:
                    return (type<<4)|(matchTableBit-1);
                    break;
                case kFrg_ClipType_match_image:
                    return (type<<4)|(matchType);
                    break;
            }
            assert(false); 
            return 0;
        }
    };
    
    class TColorZip{
    public:
        explicit TColorZip(float colorQuality,bool isMustFitColorTable,int* tempMemoryByteSize_forDecode):m_colorQuality(colorQuality),m_tableZiper(colorQuality,isMustFitColorTable),m_tempMemoryByteSize_forDecode(*tempMemoryByteSize_forDecode){ }
        void saveTo(std::vector<TByte>& out_buf,const TPixels32Ref& ref){
            if (ref.getIsEmpty()) return;
            m_srcRef=ref;
            m_nodeWidth=(m_srcRef.width+kFrg_ClipWidth-1)/kFrg_ClipWidth;
            m_nodeHeight=(m_srcRef.height+kFrg_ClipHeight-1)/kFrg_ClipHeight;
            m_colorMatcher.initColorMatch(m_srcRef, kFrg_ClipWidth, kFrg_ClipHeight);
            m_tableZiper.setImageSize(m_srcRef.width,m_srcRef.height);
            m_tableMatcher.initSetColorMask(TColorTableZiper::getMatchColorMask(m_colorQuality));
            createColorTable();
            createIndexList();
            writeOut(out_buf);
        }
        
        void writeOut(std::vector<TByte>& out_buf){
            const TInt32 nodeCount=(TInt32)m_clipNodeList.size();
            pack32Bit(out_buf,nodeCount);
            const TInt32 tableSize=(TInt32)m_colorTable.size();
            pack32Bit(out_buf,tableSize);
            const TInt32 index2Size=(TInt32)m_indexList.size();
            pack32Bit(out_buf,index2Size);
            const TInt32 matchCount=(TInt32)m_matchXYList.size();
            pack32Bit(out_buf,matchCount);
            
            
            for (int i=0;i<nodeCount;++i){
                const TClipNode& node=m_clipNodeList[i];
                out_buf.push_back(node.packNodeInfo());
            }
            
            m_tempMemoryByteSize_forDecode=tableSize*sizeof(TBGRA32)+3; //+3 is for Align4
            for (int i=0;i<tableSize;++i) {
                out_buf.push_back(m_colorTable[i].b);
                out_buf.push_back(m_colorTable[i].g);
                out_buf.push_back(m_colorTable[i].r);
            }
            
            out_buf.insert(out_buf.end(), m_indexList.begin(),m_indexList.end());
            
            for (int i=0; i<matchCount; ++i) {
                writeUInt32(out_buf,m_matchXYList[i]);
            }
        }
        void createColorTable(){
            const int nodeCount=m_nodeWidth*m_nodeHeight;
            m_clipNodeList.resize(nodeCount);
            m_colorTable.clear();
            m_matchXYList.clear();
            //m_lastPushSubTableSize=0;
            
            
            TClipNode* cur_node=&m_clipNodeList[0];
            for (int ny=0; ny<m_nodeHeight;++ny) {
                for (int nx=0; nx<m_nodeWidth;++nx,++cur_node) {
                    cur_node->subX0=nx*kFrg_ClipWidth;
                    cur_node->subY0=ny*kFrg_ClipHeight;
                    cur_node->colors.pColor=&m_srcRef.pixels(cur_node->subX0,cur_node->subY0);
                    cur_node->colors.byte_width=m_srcRef.byte_width;
                    cur_node->colors.width=kFrg_ClipWidth;
                    if (cur_node->subX0+kFrg_ClipWidth>m_srcRef.width)
                        cur_node->colors.width=m_srcRef.width-cur_node->subX0;
                    cur_node->colors.height=kFrg_ClipHeight;
                    if (cur_node->subY0+kFrg_ClipHeight>m_srcRef.height)
                        cur_node->colors.height=m_srcRef.height-cur_node->subY0;
                    
                    cur_node->type=kFrg_ClipType_index;
                }
            }
            
            //预先计算局部调色板.
            cur_node=&m_clipNodeList[0];
            int nodeSize=(int)m_clipNodeList.size();
            getBestColorTables(m_tableZiper,cur_node,nodeSize); //sub_table.empty()表示需要无损压缩.
            
            //处理node颜色全匹配.
            cur_node=&m_clipNodeList[0];
            for (int ny=0; ny<m_nodeHeight;++ny) {
                for (int nx=0; nx<m_nodeWidth;++nx,++cur_node) {
                   std::vector<Color24>& sub_table=cur_node->sub_table;
                    //先处理单色.
                    bool isSingleColor=(sub_table.size()==1);
                    if (!isSingleColor){
                        //查找是否能匹配.
                        frg_TMatchType matchType;
                        TInt32  matchX0,matchY0;
                        if (findMatch(nx,ny,&matchX0,&matchY0,&matchType)){
                            //match ok
                            cur_node->type=kFrg_ClipType_match_image;
                            cur_node->sub_table.clear();
                            cur_node->matchType=matchType;
                            cur_node->colorMatchIndex=addAMatch(matchX0, matchY0);
                        }else if (sub_table.size()==0){
                            cur_node->type=kFrg_ClipType_directColor;
                            TByte _tmpAlpha;
                            if ((cur_node->colors.width==kFrg_ClipWidth)&&getIsSigleAlphaColor(cur_node->colors,&_tmpAlpha))
                                cur_node->directcolorTypeData=1;
                            else
                                cur_node->directcolorTypeData=0;
                        }
                    }else{
                        TByte _tmpAlpha;
                        if ((cur_node->colors.width==kFrg_ClipWidth)&&getIsSigleAlphaColor(cur_node->colors,&_tmpAlpha))
                            cur_node->type=kFrg_ClipType_single_bgra_w8;
                        else
                            cur_node->type=kFrg_ClipType_single_bgr;
                    }
                }
            }
            
            cur_node=&m_clipNodeList[0];
            for (int ni=0; ni<(int)m_clipNodeList.size();++ni,++cur_node) {
               std::vector<Color24>& sub_table=cur_node->sub_table;
                if (cur_node->type==kFrg_ClipType_match_image)
                    continue; //next node;
                
                if (cur_node->isDirectColorType()){//无损.
                    addNodeColorsToTable(cur_node->colors);
                    //m_lastPushSubTableSize=0;
                    continue; //next node;
                }
                
                bool isSingleColor=cur_node->isSingleColorType();//单色需要特别处理,只支持短匹配.
                
                int matchTableBit;
                int tableMatchIndex=m_tableMatcher.findMatch(m_colorTable,sub_table,&matchTableBit); //颜色表匹配.
                int curBestZipBytes=0;
                if (tableMatchIndex>=0){
                    int forwardLength=(int)m_colorTable.size()-tableMatchIndex;
                    const int indexBitOld=kFrg_SubTableSize_to_indexBit[sub_table.size()];
                    if (isSingleColor)
                        curBestZipBytes=(int)sub_table.size()*sizeof(Color24);
                    else
                        curBestZipBytes=(int)sub_table.size()*sizeof(Color24)-(kFrg_ClipWidth*kFrg_ClipHeight*(matchTableBit-indexBitOld)>>3);
                    if (forwardLength>kFrg_MaxForwardLength)
                        curBestZipBytes-=pack32BitOutSize(forwardLength);
                    assert(curBestZipBytes>0);
                }
                
                if (tableMatchIndex>=0){
                     cur_node->matchTableBit=matchTableBit;
                     int forwardLength=(int)m_colorTable.size()-tableMatchIndex;
                     //note: mybe forwardLength < sub_table.size()
                                          
                     if (!isSingleColor){
                         TByte _tmpAlpha;
                         if ((cur_node->colors.width==kFrg_ClipWidth)&&getIsSigleAlphaColor(cur_node->colors,&_tmpAlpha))
                             cur_node->type=kFrg_ClipType_match_table_single_a_w8;
                         else
                             cur_node->type=kFrg_ClipType_match_table;
                         cur_node->sub_table.clear();
                         cur_node->forwardLength=forwardLength;
                     }else{
                         assert(sub_table.size()==1);
                         //singleColor;
                         if (forwardLength<=kFrg_MaxForwardLength){
                             cur_node->forwardLength=forwardLength;
                         }else{
                             cur_node->forwardLength=0;
                             addColorsToTable(sub_table);
                         }
                     }
                 }else{
                     if (!isSingleColor){
                         TByte _tmpAlpha;
                         if ((cur_node->colors.width==kFrg_ClipWidth)&&getIsSigleAlphaColor(cur_node->colors,&_tmpAlpha))
                             cur_node->type=kFrg_ClipType_index_single_a_w8;
                         else
                             cur_node->type=kFrg_ClipType_index;
                         addColorsToTable(sub_table);
                     }else{
                         assert(sub_table.size()==1);
                         //singleColor;
                         cur_node->forwardLength=0;
                         addColorsToTable(sub_table);
                     }
                 }
            }
        }
        void createIndexList(){
            m_indexList.clear();
            TInt32 nodeCount=m_nodeWidth*m_nodeHeight;
            
            //get indexList
            std::vector<TByte> sub_indexList; sub_indexList.reserve(kFrg_ClipWidth*kFrg_ClipHeight);
            const Color24* cur_table=&m_colorTable[0];
            const Color24* table_end=cur_table+m_colorTable.size();
            for (int i=0;i<nodeCount;++i){
                TClipNode& cur_node=m_clipNodeList[i];
                switch (cur_node.type) {
                    case kFrg_ClipType_index:
                    case kFrg_ClipType_index_single_a_w8:
                    case kFrg_ClipType_match_table:
                    case kFrg_ClipType_match_table_single_a_w8:{
                        const Color24* subTable;
                        const bool isMatchTable=cur_node.isMatchTableType();
                        if (isMatchTable){
                            subTable=cur_table-cur_node.forwardLength;
                        }else{ //index
                            subTable=cur_table;
                            cur_table+=cur_node.sub_table.size();
                        }
                                                
                        sub_indexList.clear();
                        int bit;
                        if (isMatchTable)
                            bit=cur_node.matchTableBit;
                        else
                            bit=kFrg_SubTableSize_to_indexBit[cur_node.sub_table.size()];
                        TInt32 subTableSafeSize=(1<<bit);
                        if (subTableSafeSize>(table_end-subTable))
                            subTableSafeSize=(TInt32)(table_end-subTable);
                        m_tableZiper.getBestColorIndex(sub_indexList, subTable,subTableSafeSize,cur_node.colors,cur_node.subX0,cur_node.subY0);
                        packIndexs(sub_indexList,bit);
                        
                        std::vector<TByte> ctrlData;
                        if (isMatchTable){//match table
                            int tableForwardLength=cur_node.forwardLength;
                            cur_node.forwardLength=0;
                            pack32Bit(ctrlData,tableForwardLength);
                        }
                        addIndexsToList(ctrlData);
                        addIndexsToList(sub_indexList);
                        
                    } break;
                    case kFrg_ClipType_directColor:{
                        cur_table+=cur_node.colors.width*cur_node.colors.height;
                    } break;
                    case kFrg_ClipType_single_bgra_w8:
                    case kFrg_ClipType_single_bgr:{
                        if (cur_node.forwardLength==0)
                            cur_table+=1;
                    } break;
                    case kFrg_ClipType_match_image:{
                        //cur_table+=0;
                    } break;
                    default:{
                        assert(false);
                    } break;
                }
            }
        }
        
        bool findMatch(TInt32 cur_nx,TInt32 cur_ny,TInt32* out_matchX0,TInt32* out_matchY0,frg_TMatchType* out_matchType){
            const int curNodeIndex=cur_ny*m_nodeWidth+cur_nx;
            const TClipNode& curNode=m_clipNodeList[curNodeIndex];
            const int subWidth=curNode.colors.width;
            const int subHeight=curNode.colors.height;
            if ((subWidth==kFrg_ClipWidth)&&(subHeight==kFrg_ClipHeight)){
                return m_colorMatcher.findMatch(cur_nx*kFrg_ClipWidth,cur_ny*kFrg_ClipHeight,subWidth,subHeight,out_matchX0,out_matchY0,out_matchType);
            }else{
                if (cur_nx>0){
                    const TClipNode& curNodeLeft=m_clipNodeList[curNodeIndex-1];
                    if (curNodeLeft.type==kFrg_ClipType_match_image){
                        *out_matchX0=unpackMatchX(m_matchXYList[curNodeLeft.colorMatchIndex])+kFrg_ClipWidth;
                        *out_matchY0=unpackMatchY(m_matchXYList[curNodeLeft.colorMatchIndex])+kFrg_ClipHeight;
                        if (m_colorMatcher.isMatchAt(cur_nx*kFrg_ClipWidth,cur_ny*kFrg_ClipHeight,subWidth,subHeight,*out_matchX0,*out_matchY0,out_matchType))
                            return true;
                    }
                }
                
                if (cur_ny>0){
                    const TClipNode& curNodeTop=m_clipNodeList[curNodeIndex-m_nodeWidth];
                    if (curNodeTop.type==kFrg_ClipType_match_image){
                        *out_matchX0=unpackMatchX(m_matchXYList[curNodeTop.colorMatchIndex])+kFrg_ClipWidth;
                        *out_matchY0=unpackMatchY(m_matchXYList[curNodeTop.colorMatchIndex])+kFrg_ClipHeight;
                        if (m_colorMatcher.isMatchAt(cur_nx*kFrg_ClipWidth,cur_ny*kFrg_ClipHeight,subWidth,subHeight,*out_matchX0,*out_matchY0,out_matchType))
                            return true;
                    }                    
                }
                
                return false;
            }
        }
        
        inline void addColorsToTable(const std::vector<Color24>& sub_table){
            //m_lastPushSubTableSize=(int)sub_table.size();
            m_colorTable.insert(m_colorTable.end(), sub_table.begin(),sub_table.end());
        }
        void addNodeColorsToTable(const TPixels32Ref& node_colors){
            const Color32* pline=node_colors.pColor;
            for (int y=0;y<node_colors.height;++y){
                for (int x=0;x<node_colors.width;++x){
                    m_colorTable.push_back(Color24(pline[x].r,pline[x].g,pline[x].b));
                }
                pline=node_colors.nextLine(pline);
            }
        }
        
        static void packIndexs(std::vector<TByte>& sub_indexList,int ibit){
            int size=(int)sub_indexList.size();
            for (int i=0; i<size; ++i) assert(sub_indexList[i]<(1<<ibit));
            switch (ibit) {
                case 4:{
                    if ((size&1)!=0){
                        sub_indexList.push_back(0);
                        ++size;
                    }
                    int newSize=size>>1;
                    for (int i=0;i<newSize;++i){
                        sub_indexList[i]=sub_indexList[i*2]|(sub_indexList[i*2+1]<<4);
                    }
                    sub_indexList.resize(newSize);
                } break;
                case 3:{
                    int newSize=0;
                    int curValue=0;
                    int curBit=0;
                    for (int i=0;i<size;++i){
                        curValue|=(sub_indexList[i]<<curBit);
                        curBit+=3;
                        if (curBit>=8){
                            sub_indexList[newSize]=curValue;
                            ++newSize;
                            curValue>>=8;
                            curBit-=8;
                        }
                    }
                    if (curBit>0){
                        sub_indexList[newSize]=curValue;
                        ++newSize;
                    }
                    sub_indexList.resize(newSize);
                } break;
                case 2:{
                    while ((size&3)!=0){
                        sub_indexList.push_back(0);
                        ++size;
                    }
                    int newSize=size>>2;
                    for (int i=0;i<newSize;++i){
                        sub_indexList[i]=sub_indexList[i*4]|(sub_indexList[i*4+1]<<2)
                                        |(sub_indexList[i*4+2]<<4)|(sub_indexList[i*4+3]<<6);
                    }
                    sub_indexList.resize(newSize);
                } break;
                case 1:{
                    while ((size&7)!=0){
                        sub_indexList.push_back(0);
                        ++size;
                    }
                    int newSize=size>>3;
                    for (int i=0;i<newSize;++i){
                        sub_indexList[i]=sub_indexList[i*8]|(sub_indexList[i*8+1]<<1)
                                        |(sub_indexList[i*8+2]<<2)|(sub_indexList[i*8+3]<<3)
                                        |(sub_indexList[i*8+4]<<4)|(sub_indexList[i*8+5]<<5)
                                        |(sub_indexList[i*8+6]<<6)|(sub_indexList[i*8+7]<<7);
                    }
                    sub_indexList.resize(newSize);
                } break;
                case 0:{
                    sub_indexList.resize(0);
                } break;
                default:{
                    assert(false);
                } break;
            }
        }
        
        inline void addIndexsToList(const std::vector<TByte>& sub_indexList){
            m_indexList.insert(m_indexList.end(), sub_indexList.begin(),sub_indexList.end());
        }
        
       inline int addAMatch(TUInt32 matchX0,TUInt32 matchY0){
            return addAMatch(packMatchXY(matchX0, matchY0));
        }
       inline int addAMatch(TUInt32 matchPos){
            int colorMatchIndex=(int)m_matchXYList.size();
            m_matchXYList.push_back(matchPos);
            return colorMatchIndex;
        }
        
        static void  getBestColorTables(const TColorTableZiper& tableZiper,TClipNode* nodes,int nodeSize);
        
    private:
        float                   m_colorQuality;
        TColorTableZiper        m_tableZiper;
        TColorMatch             m_colorMatcher;
        TTableMatch             m_tableMatcher;
        //int                   m_lastPushSubTableSize;
        //TIndexMatch           m_indexMatcher;
        TPixels32Ref            m_srcRef;
        TInt32                  m_nodeWidth;
        TInt32                  m_nodeHeight;
        std::vector<TClipNode>  m_clipNodeList;
        std::vector<TByte>      m_indexList;
        std::vector<Color24>    m_colorTable;       //总调色板.
        std::vector<TUInt32>    m_matchXYList;      //帧内匹配列表.
        int&                    m_tempMemoryByteSize_forDecode;
    };
    
    //int _temp_nums[16+1]={0};
    
    void  TColorZip::getBestColorTables(const TColorTableZiper& tableZiper,TClipNode* nodes,int nodeSize){
        //#pragma omp parallel for
        for (int i=0; i<nodeSize;++i){
            tableZiper.getBestColorTable(nodes[i].sub_table,nodes[i].colors);
            //++_temp_nums[nodes[i].sub_table.size()-1];
        }
    }

    
    ///
    
    void TColorZiper::saveTo(std::vector<TByte>& out_buf,const TPixels32Ref& ref,float colorQuality,bool isMustFitColorTable,int* tempMemoryByteSize_forDecode){
        *tempMemoryByteSize_forDecode=0;
        //Int32 pos=out_buf.size(); colorQuality=100; //for test
        if (ref.getIsEmpty()) return;
        TColorZip colorZip(colorQuality,isMustFitColorTable,tempMemoryByteSize_forDecode);
        colorZip.saveTo(out_buf,ref);
    }

}//end namespace frg
