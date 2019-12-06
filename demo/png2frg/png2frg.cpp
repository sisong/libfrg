//
//  png2frg.cpp
//  demo for libfrg
//
//  Created by housisong on 2013-06-10.
//  rewrite by housisong on 2019-12-04.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h> // malloc free
#include <assert.h>
#include <string.h>
#include <vector>
#include <stdexcept>  //std::runtime_error
#include "../../writer/frg_writer.h"
#include "../../reader/frg_reader.h"

typedef unsigned char TByte;

#define check(v) do{ if (!(v)) { throw std::runtime_error(#v); } }while(0)

double  clock_s();
void    writeFile(const std::vector<TByte>& srcData,const char* dstFileName);
void    readFile(std::vector<TByte>& out_data,const char* srcFileName);

void    decodeFrgImage(const TByte* frgCode,const TByte* frgCode_end,frg::TFrgPixels32Ref& out_image);
#define encodeFrgImage  frg::writeFrgImage
void    decodePngImage(const TByte* pngCode,const TByte* pngCode_end,frg::TFrgPixels32Ref& out_image);
void    encodePngImage(std::vector<TByte>& out_frgCode,const frg::TFrgPixels32Ref& image);

struct TAutoFileClose {
    inline explicit TAutoFileClose(FILE* _file):file(_file){}
    inline ~TAutoFileClose() { if (file) fclose(file); }
private:
    FILE* file;
};

struct TAutoMemFree {
    inline explicit TAutoMemFree(void* _pmem_from_malloc):pmem(_pmem_from_malloc){}
    inline ~TAutoMemFree() { if (pmem) free(pmem); }
private:
    void* pmem;
};

struct TAutoPixelsFree {
    inline explicit TAutoPixelsFree(frg::TFrgPixels32Ref* _ppixels):ppixels(_ppixels){}
    inline ~TAutoPixelsFree() { if (ppixels->pColor) free(ppixels->pColor); }
private:
    frg::TFrgPixels32Ref* ppixels;
};

enum TConvertType {
    cv_unknown = 0,
    cv_png2frg,
    cv_frg2png
};

#define RunSpeedTest(time,runner)   \
    double time=0;  \
    {   double time0=clock_s();     \
        double time1=time0;         \
        long runLoopCount=0;        \
        while (time1-time0<0.3) {   \
            runner; \
            ++runLoopCount;         \
            time1=clock_s();        \
        }           \
       time=(time1-time0)/runLoopCount; \
    }

void convertImageType(const char* srcFile,const char* dstFile,
                      TConvertType cvType,float quality,float compressSize){
    const bool         isPngSrc=(cvType==cv_png2frg);
    const char*        srcType=isPngSrc?"png":"frg";
    const char*        dstType=isPngSrc?"frg":"png";
    frg::TFrgParameter frg_encode_parameter(quality,compressSize);
    //load src
    frg::TFrgPixels32Ref srcImage; srcImage.pColor=0;
    TAutoPixelsFree _autoSrcPixelsFree(&srcImage);
    std::vector<TByte> srcCode; readFile(srcCode,srcFile);
    printf("%s byte size  : %d \n",srcType,(int)srcCode.size());
    RunSpeedTest(decodeSrcTime_s,isPngSrc?
                decodePngImage(srcCode.data(),srcCode.data()+srcCode.size(),srcImage)
               :decodeFrgImage(srcCode.data(),srcCode.data()+srcCode.size(),srcImage));
    printf("%s decode time: %.3f ms\n",srcType,decodeSrcTime_s*1000);
    printf("image pixels  : %d * %d\n",srcImage.width,srcImage.height);
    
    //encode
    std::vector<TByte> dstCode;
    RunSpeedTest(encodeDstTime_s,isPngSrc?
                 encodePngImage(dstCode,srcImage)
                :encodeFrgImage(dstCode,srcImage,frg_encode_parameter));
    printf("%s byte size  : %d \n",dstType,(int)dstCode.size());
    printf("%s encode time: %.3f ms\n",dstType,encodeDstTime_s*1000);
    writeFile(dstCode,dstFile); //save dst
    
    //decode
    frg::TFrgPixels32Ref dstImage; dstImage.pColor=0;
    TAutoPixelsFree _autoDstPixelsFree(&dstImage);
    RunSpeedTest(decodeDstTime_s,isPngSrc?
                 decodeFrgImage(dstCode.data(),dstCode.data()+dstCode.size(),dstImage)
                :decodePngImage(dstCode.data(),dstCode.data()+dstCode.size(),dstImage));
    printf("%s decode time: %.3f ms\n",dstType,decodeDstTime_s*1000);
}

void printUsage(){
    printf("usage: png2frg src.png dst.frg quality(0.0--100.0] compressSize[0.0--100.0]\n"
           "    ( png image file convert to frg; if quality set 100 means lossless. )\n"
           "usage: png2frg src.png dst.frg\n"
           "    ( same as: png2frg src.png dst.frg 80 25 )\n"
           "usage: png2frg src.frg dst.png\n"
           "    ( frg image file convert to png. )\n");
}

bool isFileType(const char* fileName,const char* type){
    size_t tlen=strlen(type);
    size_t flen=strlen(fileName);
    if (flen<tlen) return false;
    const char* suffix=fileName+flen-tlen;
    for (size_t i=0;i<tlen; ++i) {
        if (tolower(suffix[i])!=tolower(type[i]))
            return false;
    }
    return true;
}

int main(int argc, const char * argv[]){
    if ((argc<3)||(argc>5)) {
        int result=(argc==1)?0:1;
        if (result!=0) printf("parameter count error!\n\n");
        printUsage(); return result;
    }
    const char* srcFile=argv[1];
    const char* dstFile=argv[2];
    float quality=80;
    float compressSize=25;
    TConvertType cvType=cv_unknown;
    if (isFileType(srcFile,".png")&&isFileType(dstFile,".frg")){
        cvType=cv_png2frg;
        if(argc>=4) quality=(float)atof(argv[3]);
        if(argc>=5) compressSize=(float)atof(argv[4]);
    }else if (isFileType(srcFile,".frg")&&isFileType(dstFile,".png")){
        cvType=cv_frg2png;
        if (argc!=3) { printf("parameter count error!\n\n"); printUsage(); return 2; }
    }else{
        printf("unknown image file type error!\n\n"); printUsage(); return 3;
    }
    
    convertImageType(srcFile,dstFile,cvType,quality,compressSize);
    return 0;
}

//  #include <time.h>
//  double clock_s(){ return clock()*1.0/CLOCKS_PER_SEC; }
#ifdef _WIN32
#include <windows.h>
double clock_s(){ return GetTickCount()/1000.0; }
#else
//Unix-like system
#include <sys/time.h>
#include <assert.h>
double clock_s(){
    struct timeval t={0,0};
    int ret=gettimeofday(&t,0);
    assert(ret==0);
    if (ret==0)
        return t.tv_sec + t.tv_usec/1000000.0;
    else
        return 0;
}
#endif

void writeFile(const std::vector<TByte>& srcData,const char* dstFileName){
    FILE* file=fopen(dstFileName, "wb");
    check(file!=0);
    TAutoFileClose _autoFileClose(file);
    if (!srcData.empty())
        check(srcData.size()==fwrite(srcData.data(),1,srcData.size(),file));
}

void readFile(std::vector<TByte>& out_data,const char* srcFileName){
    FILE* file=fopen(srcFileName, "rb");
    check(file!=0);
    TAutoFileClose _autoFileClose(file);
    check(0==fseek(file,0,SEEK_END));
    long file_length = (long)ftell(file);
    assert((file_length>=0)&&((size_t)file_length)==(unsigned long)file_length);
    check(0==fseek(file,0,SEEK_SET));
    out_data.resize((size_t)file_length);
    if (!out_data.empty())
        check(out_data.size()==fread(out_data.data(),1,out_data.size(),file));
}


void decodeFrgImage(const TByte* frgCode,const TByte* frgCode_end,frg::TFrgPixels32Ref& out_image){
    frg_TFrgImageInfo frgInfo;
    check(readFrgImageInfo(frgCode,frgCode_end,&frgInfo));
    check(out_image.pColor==0);
    size_t pixelsCount=(size_t)frgInfo.imageWidth*(size_t)frgInfo.imageHeight;
    out_image.pColor=(frg::TFrgBGRA32*)malloc(pixelsCount*sizeof(frg::TFrgBGRA32));
    check((out_image.pColor!=0)||(pixelsCount==0));
    out_image.width=frgInfo.imageWidth;
    out_image.height=frgInfo.imageHeight;
    out_image.byte_width=frgInfo.imageWidth*sizeof(frg::TFrgBGRA32);
    
    TByte* tempMemory=(TByte*)malloc(frgInfo.decoder_tempMemoryByteSize);
    check((tempMemory!=0)||(frgInfo.decoder_tempMemoryByteSize==0));
    TAutoMemFree _autoMemFree(tempMemory);
    frg_TPixelsRef bmpImage;
    assert(sizeof(frg::TFrgBGRA32)==kFrg_outColor_size);
    *(frg::TFrgPixels32Ref*)&bmpImage=out_image;
    bmpImage.colorType=kFrg_ColorType_32bit_A8R8G8B8;
    check(readFrgImage(frgCode,frgCode_end,&bmpImage,
                       tempMemory,tempMemory+frgInfo.decoder_tempMemoryByteSize,0));
}

//png
//#include "png.h" //
void decodePngImage(const TByte* pngCode,const TByte* pngCode_end,frg::TFrgPixels32Ref& out_image){
    //todo:
    check(0);
}

void encodePngImage(std::vector<TByte>& out_pngCode,const frg::TFrgPixels32Ref& image){
    //todo:
    check(0);
}
