#include "hpdf.h"
#include <cstring>
#include <cstdio>
#include <dlfcn.h>
#include <qrencode.h>
#include <dmtx.h>
#include "pdf417lib.h"

int hpdf_dmtx(HPDF_Page page, const char* encode, double startx, double starty, double cellsize){
    //dlopen("libdmtx.so"RTLD_GLOBAL);
    DmtxEncode *enc = dmtxEncodeCreate();
    int result;
    result = dmtxEncodeSetProp(enc, DmtxPropModuleSize, 1);
    result = dmtxEncodeSetProp(enc, DmtxPropMarginSize, 0);
    result = dmtxEncodeSetProp(enc, DmtxPropPixelPacking, DmtxPack8bppK);
    result = dmtxEncodeDataMatrix(enc, strlen(encode), (unsigned char*)encode);
    int height = enc->region.symbolRows;
    int width = enc->region.symbolCols;
    int module;
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            module = dmtxSymbolModuleStatus(enc->message, enc->region.sizeIdx, y, x);
            if (module & DmtxModuleOn){
                HPDF_Page_Rectangle(page, startx + x * cellsize, starty + y * cellsize, cellsize, cellsize);
                HPDF_Page_Fill(page);
            }
        }
    }
    dmtxEncodeDestroy(&enc);
    return 0;
}
// this one currently doesn't scan
int hpdf_pdf417(HPDF_Page page, const char* encode, double startx, double starty, double cellsize){
    pdf417param p;
    pdf417init(&p);
    p.text = (char*)encode;
    paintCode(&p);
    int result;
    int height = p.codeRows;
    int width = p.bitColumns;
    int cols = p.bitColumns / 8 + 1;
    int x = 0, y = 0;
    //printf("Height: %d\tWidth: %d\tBit Columns: %d\n", p.codeRows, p.codeColumns, p.bitColumns);
    for(int idx = 0; idx < p.lenBits; idx++){
        for(int bit = 0; bit < 8; bit++){
            if((p.outBits[idx] >> bit) & 0x1){
                x = (idx % cols) * 8 + bit;
                y = idx / cols;
                int finalX = startx + x * cellsize;
                int finalY = starty + y * cellsize * 3;
                HPDF_Page_Rectangle(page, finalX, finalY, cellsize, cellsize * 3);
                HPDF_Page_Fill(page);
            }
        }
    }
    pdf417free(&p);
    return 0;
}
int hpdf_qrcode(HPDF_Page page, const char* encode, double startx, double starty, double cellsize){
    QRcode *qrcode = QRcode_encodeString(encode, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
    int qrwidth = qrcode->width;
    unsigned char *data = qrcode->data;
    for(int y = 0; y < qrwidth; y++){
        for(int x = 0; x < qrwidth; x++){
            if ((*data & 1) == 1){
                HPDF_Page_Rectangle(page, startx + x * cellsize, starty + (qrwidth - y) * cellsize, cellsize, cellsize);
                HPDF_Page_Fill(page);
            }
            data++;
        }
    }
    QRcode_free(qrcode);
    return 0;
}
int hpdf_qrdots(HPDF_Page page, const char* encode, double startx, double starty, double cellsize){
    QRcode *qrcode = QRcode_encodeString(encode, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
    int qrwidth = qrcode->width;
    unsigned char *data = qrcode->data;
    for(int y = 0; y < qrwidth; y++){
        for(int x = 0; x < qrwidth; x++){
            if ((*data & 1) == 1){
                HPDF_Page_Rectangle(page, startx + x * cellsize, starty + (qrwidth - y) * cellsize, cellsize, cellsize/2);
                HPDF_Page_Fill(page);
            }
            data++;
        }
    }
    QRcode_free(qrcode);
    return 0;
}
int hpdf_microqr(HPDF_Page page, const char* encode, double startx, double starty, double cellsize){
    QRcode *qrcode = QRcode_encodeStringMQR(encode, 2, QR_ECLEVEL_L, QR_MODE_8, 1);
    int qrwidth = qrcode->width;
    unsigned char *data = qrcode->data;
    for(int y = 0; y < qrwidth; y++){
        for(int x = 0; x < qrwidth; x++){
            if ((*data & 1) == 1){
                HPDF_Page_Rectangle(page, startx + x * cellsize, starty + (qrwidth - y) * cellsize, cellsize, cellsize);
                HPDF_Page_Fill(page);

            }
            data++;
        }
    }
    QRcode_free(qrcode);
    return 0;
}
