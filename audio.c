#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include "micromod.h"
static char *fbuf;
static int fsize;

void getMusic(short* output,int count){
	micromod_get_audio( output, count );
}

void loadMusic(char* filename){
	FILE* mod=fopen(filename,"rb");
	fseek(mod,0,SEEK_END);
	fsize=ftell(mod);
	fseek(mod,0,SEEK_SET);
	fbuf=malloc(fsize);
	fread(fbuf,fsize,1,mod);
	fclose(mod);
	
	micromod_initialise( fbuf, 22050 );
	
}