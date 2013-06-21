//
//  frg_demo.cpp
//  for frg
//
//  Created by housisong on 12-12-4.
//

#include <iostream>
#include "../writer/frg_writer.h"
#include "../reader/frg_reader.h"

void readFile(std::vector<unsigned char>& dstData,const char* srcFileName){
    FILE	* file=fopen(srcFileName, "rb");
    
	fseek(file,0,SEEK_END);
	int file_length = (int)ftell(file);
	fseek(file,0,SEEK_SET);
    
    dstData.resize(file_length);
    if (file_length>0)
        fread(&dstData[0],1,file_length,file);
    
    fclose(file);
}

void writeFile(const std::vector<unsigned char>& srcData,const char* dstFileName){
    FILE	* file=fopen(dstFileName, "wb");
    
    int dataSize=(int)srcData.size();
    if (dataSize>0)
        fwrite(&srcData[0], 1,dataSize, file);
    
    fclose(file);
}

void loadBmpImage(frg::TFrgPixels32Ref& image){
    const int width=800;
    const int height=600;
    image.pColor=new frg::TFrgBGRA32[width*height];
    image.width=width;
    image.height=height;
    image.byte_width=width*sizeof(frg::TFrgBGRA32);
    
    frg::TFrgBGRA32* colorLine=image.pColor;
    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            colorLine[x].a=y-x+127;
            colorLine[x].b=x;
            colorLine[x].g=y;
            colorLine[x].r=x+y;
        }
        (unsigned char*&)colorLine+=image.byte_width; //next line
    }    
}

void bmpImage_encodeTo_frgImage(const frg::TFrgPixels32Ref& bmpImage,std::vector<unsigned char>& out_frgImageCode){
    frg::TFrgParameter frg_encode_parameter(80,50);
    //frg::TFrgParameter frg_encode_parameter(100,0); //lossless and minsize
    writeFrgImage(out_frgImageCode, bmpImage, frg_encode_parameter);
}

bool frgImage_decodeTo_bmpImage(const unsigned char* frgCode,const unsigned char* frgCode_end,frg_TPixelsRef& bmpImage){
    frg_TFrgImageInfo frgInfo;
    if (!readFrgImageInfo(frgCode, frgCode_end, &frgInfo))
        return false;
    unsigned char *tempMemory=new unsigned char[frgInfo.decoder_tempMemoryByteSize];
    
    bmpImage.byte_width=frgInfo.imageWidth*kFrg_outColor32_size;
    bmpImage.pColor=new unsigned char[frgInfo.imageWidth*bmpImage.byte_width];
    bmpImage.width=frgInfo.imageWidth;
    bmpImage.height=frgInfo.imageHeight;
    bmpImage.colorType=kFrg_ColorType_32bit_A8R8G8B8;
    bool isok=frg_FALSE!=readFrgImage(frgCode,frgCode_end,&bmpImage,tempMemory,tempMemory+frgInfo.decoder_tempMemoryByteSize);
    
    delete []tempMemory;
    return isok;
}

int main(int argc, const char * argv[]){
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
    frg_TPixelsRef dstImage; dstImage.pColor=0;
    if (!frgImage_decodeTo_bmpImage(&frgImageCode[0],&frgImageCode[0]+frgImageCode.size(),dstImage)){
        std::cout << "read frg image error!\n";
        result=1;
    }

    delete [](unsigned char*)dstImage.pColor;
    delete []srcImage.pColor;
    std::cout << "\ndone!\n";
    return result;
}

