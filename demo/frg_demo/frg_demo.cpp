//
//  frg_demo.cpp
//  for libfrg
//
//  Created by housisong on 2012-12-04.
//

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h> // malloc free
#include <vector>
#include "../../writer/frg_writer.h"
#include "../../reader/frg_reader.h"
typedef unsigned char TByte;

void writeFile(const std::vector<TByte>& srcData,const char* dstFileName){
    FILE* file=fopen(dstFileName, "wb");
    assert(file!=0);
    int dataSize=(int)srcData.size();
    if (dataSize>0)
        fwrite(srcData.data(),1,dataSize,file);
    fclose(file);
}

void loadPixelsImage(frg::TFrgPixels32Ref& out_image){
    const int width=1024;
    const int height=768;
    assert(out_image.pColor==0);
    out_image.pColor=(frg::TFrgBGRA32*)malloc(width*height*sizeof(frg::TFrgBGRA32));
    out_image.width=width;
    out_image.height=height;
    out_image.byte_width=width*sizeof(frg::TFrgBGRA32);

    frg::TFrgBGRA32* colorLine=out_image.pColor;
    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            colorLine[x].b=x;
            colorLine[x].g=y;
            colorLine[x].r=x+y;
            colorLine[x].a=y-x+127;
        }
        colorLine=(frg::TFrgBGRA32*)((TByte*)colorLine+out_image.byte_width); //next line
    }
}

bool decodeFrgImage(const TByte* frgCode,const TByte* frgCode_end,frg::TFrgPixels32Ref& out_image){
    frg_TFrgImageInfo frgInfo;
    if (!readFrgImageInfo(frgCode, frgCode_end, &frgInfo))
        return false;
    assert(out_image.pColor==0);
    size_t pixelsCount=(size_t)frgInfo.imageWidth*(size_t)frgInfo.imageHeight;
    out_image.pColor=(frg::TFrgBGRA32*)malloc(pixelsCount*sizeof(frg::TFrgBGRA32));
    out_image.width=frgInfo.imageWidth;
    out_image.height=frgInfo.imageHeight;
    out_image.byte_width=frgInfo.imageWidth*sizeof(frg::TFrgBGRA32);
    
    TByte* tempMemory=(TByte*)malloc(frgInfo.decoder_tempMemoryByteSize);
    frg_TPixelsRef bmpImage;
    assert(sizeof(frg::TFrgBGRA32)==kFrg_outColor_size);
    *(frg::TFrgPixels32Ref*)&bmpImage=out_image;
    bmpImage.colorType=kFrg_ColorType_32bit_A8R8G8B8;
    frg_BOOL isok=readFrgImage(frgCode,frgCode_end,&bmpImage,
                               tempMemory,tempMemory+frgInfo.decoder_tempMemoryByteSize,0);
    free(tempMemory);
    return frg_FALSE!=isok;
}

int pixels2FrgDemo(){
    int result=0;
    frg::TFrgPixels32Ref srcImage; srcImage.pColor=0;
    loadPixelsImage(srcImage);
    size_t pixelsCount=(size_t)srcImage.width*(size_t)srcImage.height;
    std::cout << "image pixels           : "<<srcImage.width<<" * "<<srcImage.height<<"\n";
    std::cout << "argb32 image byte size : "<<pixelsCount*sizeof(frg::TFrgBGRA32)<<"\n";

    //encode
    std::vector<TByte> frgImageCode;
    frg::TFrgParameter frg_encode_parameter(80,25);  //default
    //frg::TFrgParameter frg_encode_parameter(100,0); //test lossless and minsize
    writeFrgImage(frgImageCode,srcImage,frg_encode_parameter);
    std::cout << "frg image byte size    : "<<frgImageCode.size()<<"\n";
    //writeFile(frgImageCode,"frg_demo_temp.frg");

    //decode
    frg::TFrgPixels32Ref dstImage; dstImage.pColor=0;
    if (!decodeFrgImage(frgImageCode.data(),frgImageCode.data()+frgImageCode.size(),dstImage)){
        std::cout << "read frg image error!\n";
        result=1;
    }

    if (dstImage.pColor) free(dstImage.pColor);
    if (srcImage.pColor) free(srcImage.pColor);
    std::cout << "\ndone!\n";
    return result;
}

int main(int argc, const char * argv[]){
    return pixels2FrgDemo();
}
