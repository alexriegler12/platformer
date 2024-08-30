#include <stdio.h>
#include <string.h>
#include "2badpcm.h"
static unsigned char clip(unsigned short in){
	if(in>255){
		return 255;
	}else{
		return (unsigned char)in;
	}
	
}
int main(int argc, char** argv){
	FILE *in,*out;
	bit2_Context ctx;
	bit2_init(&ctx);
	in=fopen(argv[2],"rb");
	out=fopen(argv[3],"wb");
	if(!strcmp(argv[1],"c")){
		signed short inc[4];
		while(fread(inc,1,8,in)){
			unsigned char temp;
			temp = bit2_encode(&ctx,inc[0]);
			temp |= bit2_encode(&ctx,inc[1]) << 2;
			temp |= bit2_encode(&ctx,inc[2]) << 4;
			temp |= bit2_encode(&ctx,inc[3]) << 6;
			fwrite(&temp,1,1,out);
		}
	}else if(!strcmp(argv[1],"d")){
		unsigned char inc;
		while(fread(&inc,1,1,in)){
			unsigned short outc[4];
			outc[0]=bit2_decode(&ctx,inc&0x3);
			outc[1]=bit2_decode(&ctx,(inc&0xC)>>2);
			outc[2]=bit2_decode(&ctx,(inc&0x30)>>4);
			outc[3]=bit2_decode(&ctx,(inc&0xC0)>>6);
			fwrite(outc,1,8,out);
		}
		
		
	}
	fclose(in);
	fclose(out);
	return 0;
}