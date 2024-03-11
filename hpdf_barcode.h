#ifndef __HPDF_BARCODE
#define __HPDF_BARCODE

int hpdf_dmtx(HPDF_Page page, const char* encode, double x, double y, double cellsize);
/* int hpdf_pdf417(HPDF_Page page, const char* encode, double x, double y, double cellsize); */ // doesn't scan
int hpdf_qrcode(HPDF_Page page, const char* encode, double x, double y, double cellsize);
int hpdf_qrdots(HPDF_Page page, const char* encode, double x, double y, double cellsize);
int hpdf_microqr(HPDF_Page page, const char* encode, double x, double y, double cellsize);

#endif __HPDF_BARCODE
