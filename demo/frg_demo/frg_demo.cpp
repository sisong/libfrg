//
//  frg_demo.cpp
//  for frg
//
//  Created by housisong on 12-12-4.
//

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h> // malloc free
#include "../../writer/frg_writer.h"
#include "../../reader/frg_reader.h"

static void writeFile(const std::vector<unsigned char>& srcData,const char* dstFileName){
    FILE* file=fopen(dstFileName, "wb");
    assert(file!=0);
    int dataSize=(int)srcData.size();
    if (dataSize>0)
        fwrite(&srcData[0], 1,dataSize, file);
    fclose(file);
}

void loadBmpImage(frg::TFrgPixels32Ref& out_image){
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
        colorLine=(frg::TFrgBGRA32*)((unsigned char*)colorLine+out_image.byte_width); //next line
    }
}

void bmpImage_encodeTo_frgImage(const frg::TFrgPixels32Ref& bmpImage,
                                std::vector<unsigned char>& out_frgImageCode){
    frg::TFrgParameter frg_encode_parameter(80,25);  //default
    //frg::TFrgParameter frg_encode_parameter(100,0); //test lossless and minsize
    writeFrgImage(out_frgImageCode, bmpImage, frg_encode_parameter);
}

bool frgImage_decodeTo_bmpImage(const unsigned char* frgCode,const unsigned char* frgCode_end,
                                frg::TFrgPixels32Ref& out_image){
    frg_TFrgImageInfo frgInfo;
    if (!readFrgImageInfo(frgCode, frgCode_end, &frgInfo))
        return false;
    unsigned char* tempMemory=(unsigned char*)malloc(frgInfo.decoder_tempMemoryByteSize);

    frg_TPixelsRef bmpImage;
    bmpImage.pColor=(unsigned char*)malloc(frgInfo.imageWidth*frgInfo.imageHeight*kFrg_outColor_size);
    bmpImage.width=frgInfo.imageWidth;
    bmpImage.height=frgInfo.imageHeight;
    bmpImage.byte_width=frgInfo.imageWidth*kFrg_outColor_size;
    bmpImage.colorType=kFrg_ColorType_32bit_A8R8G8B8;
    frg_BOOL isok=readFrgImage(frgCode,frgCode_end,&bmpImage,
                               tempMemory,tempMemory+frgInfo.decoder_tempMemoryByteSize,0);
    free(tempMemory);
    
    assert(out_image.pColor==0);
    assert(sizeof(frg::TFrgBGRA32)==kFrg_outColor_size);
    out_image.pColor=(frg::TFrgBGRA32*)bmpImage.pColor;
    out_image.width=bmpImage.width;
    out_image.height=bmpImage.height;
    out_image.byte_width=bmpImage.byte_width;
    return frg_FALSE!=isok;
}

int pixelsToFrgDemo(){
    int result=0;
    frg::TFrgPixels32Ref srcImage; srcImage.pColor=0;
    loadBmpImage(srcImage);
    std::cout << "argb32 bmp image size:"<<srcImage.width*srcImage.height*4<<"\n";

    //encode
    std::vector<unsigned char> frgImageCode;
    bmpImage_encodeTo_frgImage(srcImage,frgImageCode);
    std::cout << "frg image size:"<<frgImageCode.size()<<"\n";
    //writeFile(frgImageCode,"frg_demo_temp.frg");

    //decode
    frg::TFrgPixels32Ref dstImage; dstImage.pColor=0;
    if (!frgImage_decodeTo_bmpImage(frgImageCode.data(),frgImageCode.data()+frgImageCode.size(),dstImage)){
        std::cout << "read frg image error!\n";
        result=1;
    }

    if (dstImage.pColor) free(dstImage.pColor);
    if (srcImage.pColor) free(srcImage.pColor);
    std::cout << "\ndone!\n";
    return result;
}

int main(int argc, const char * argv[]){
    return pixelsToFrgDemo();
}
