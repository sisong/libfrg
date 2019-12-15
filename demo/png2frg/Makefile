
FRG_PATH := ../..
READER_OBJ := \
    $(FRG_PATH)/reader/frg_reader.o \
    $(FRG_PATH)/reader/frg_draw.o

WRITER_OBJ := \
    $(FRG_PATH)/writer/frg_writer.o \
    $(FRG_PATH)/writer/frg_private/bytes_rle.o \
    $(FRG_PATH)/writer/frg_private/frg_color_tools.o \
    $(FRG_PATH)/writer/frg_private/bgr_zip/frg_color_table.o \
    $(FRG_PATH)/writer/frg_private/bgr_zip/frg_color_zip.o \
    $(FRG_PATH)/writer/frg_private/bgr_zip/frg_match_image.o \
    $(FRG_PATH)/writer/frg_private/bgr_zip/frg_match_table.o

# https://github.com/lz4/lz4 
LZ4_PATH := ../../../lz4/lib
LZ4_OBJ := \
    $(LZ4_PATH)/lz4.o \
    $(LZ4_PATH)/lz4hc.o 

CFLAGS     += -O3 -DNDEBUG -I'../../../lz4/lib'
CXXFLAGS   += -O3 -DNDEBUG -I'../../../lz4/lib'


# http://www.libpng.org  https://github.com/glennrp/libpng
# http://zlib.net  https://github.com/madler/zlib
LIB_LINK := -lz -lpng16

.PHONY: all install clean

all: libfrg.a png2frg

libfrg.a: $(READER_OBJ) $(WRITER_OBJ) $(LZ4_OBJ)
	$(AR) rcs $@ $^

png2frg: 
	$(CXX) png2frg.cpp libfrg.a $(LIB_LINK) -o png2frg

RM := rm -f

clean:
	$(RM) libfrg.a png2frg $(READER_OBJ) $(WRITER_OBJ) $(LZ4_OBJ)

