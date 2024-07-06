#include "imageloader.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"
#include "tjpgd.h"
#include <stdio.h>
SDL_Surface* loadQoi(char* filename){
	qoi_desc desc;
	char* pixels;
	SDL_Surface* image;
	pixels=qoi_read(filename,&desc, 4);
	image=SDL_CreateRGBSurface(0,desc.width,desc.height,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
	memcpy(image->pixels,pixels,desc.width*desc.height*4);
	free(pixels);
	return image;
	
}
static char* jpgpix;
static int jpgw;
int output_func (	/* 1:Ok, 0:Aborted */
	JDEC* jd,		/* Decompression object */
	void* bitmap,	/* Bitmap data to be output */
	JRECT* rect		/* Rectangular region to output */
)
{
	int nx, ny, xc, wd;
	char *src,*dst;
	


	nx = rect->right - rect->left + 1;
	ny = rect->bottom - rect->top + 1;	/* Number of lines of the rectangular */
	src = (uint8_t*)bitmap;				/* RGB bitmap to be output */

	wd = jd->width*3;							/* Number of bytes a line of the frame buffer */
	dst = (char*)jpgpix + rect->top * wd + rect->left * 3;	/* Left-top of the destination rectangular in the frame buffer */

	do {	/* Copy the rectangular to the frame buffer */
		xc = nx;
		do {
			
							/* RGB888 output */
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
			
		} while (--xc);
		dst += wd - nx * 3;
	} while (--ny);

	return 1;	/* Continue to decompress */
	
}


size_t input_func (	/* Returns number of bytes read (zero on error) */
	JDEC* jd,		/* Decompression object */
	uint8_t* buff,	/* Pointer to the read buffer (null to remove data) */
	size_t ndata	/* Number of bytes to read/skip */
)
{
	FILE *f;
	int res;
	f=(FILE*)jd->device;
	if (buff) {
		return fread(buff,1,ndata,f);
	}else{
		return fseek(f, ndata, SEEK_CUR) ? 0 : ndata;
	}
	
}


SDL_Surface* loadJpeg(char* filename){
	JDEC jd;
	JRESULT rc;
	void *jdwork;
	char* pixels;
	SDL_Surface* image;
	FILE* jpeg;
	jdwork=malloc(32768);
	jpeg=fopen(filename,"rb");
	rc = jd_prepare(&jd, input_func, jdwork, 32768, jpeg);
	image=SDL_CreateRGBSurface(0,jd.width,jd.height,24,0x000000FF,0x0000FF00,0x00FF0000,0x00000000);
	jpgpix=(char*)image->pixels;
	//jpgw=jd.width;
	
	
	rc = jd_decomp(&jd, output_func, 0);
	if (rc == JDR_OK) {
		printf("decoded jpeg");
	}else{
		printf("decode jpeg error");
	}
	
	fclose(jpeg);
	free(jdwork);
	return image;
	
}