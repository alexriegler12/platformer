
/*const signed char indexTable[16] = {-1, -1, -1, -1, 2, 4, 6, 8,-1, -1, -1, -1, 2, 4, 6, 8};*/
const signed char indexTable[4] = {-1, 1, -1, 1};
const unsigned short stepsizeTable[89] = {7, 8, 9, 10, 11, 12, 13, 14,
16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60,
66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209,
230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878,
2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871,
5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635,
13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
32767};
typedef struct bit2_ctx{
	signed long predicted;
	signed char stepindex;
	signed short stepsize;
}bit2_Context;



void bit2_init(bit2_Context *ctx){
	ctx->predicted=0;
	ctx->stepindex=0;
	ctx->stepsize=7;
}



unsigned char bit2_encode(bit2_Context *ctx, signed short sample){
	signed long difference=0;
	/*char mask;*/
	signed long tempStepsize;
	int i;
	unsigned char newSample=0;
	difference=sample-ctx->predicted;
	if (difference >= 0) 
	{
		newSample = 0;
	}else{
		newSample = 2; 
		difference = -difference; 
	}
	/*mask = 1;*/
	/*tempStepsize = ctx->stepsize;*/
	/*for (i = 0; i < 3 ; i++ ) 
	{*/
		if (difference >= ctx->stepsize)
		{ 
			newSample |= 1; 
			/*difference -= tempStepsize; */
		}
		/*tempStepsize >>=1; */
		/*mask >>=1; */
	/*}*/
	
	difference = 0; 
	if (newSample & 1) 
		difference += ctx->stepsize;
	/*if (newSample & 2)
		difference += ctx->stepsize >> 1;
	if (newSample & 1)
		difference += ctx->stepsize >> 2;*/
	difference += ctx->stepsize >> 1;

	if (newSample & 2) 
		difference = -difference;

	ctx->predicted += difference;
	if (ctx->predicted > 32767) 
		ctx->predicted = 32767;
	else if (ctx->predicted < -32768)
		ctx->predicted = -32768;

	ctx->stepindex += indexTable[newSample];
	if (ctx->stepindex < 0) 
		ctx->stepindex = 0;
	else if (ctx->stepindex > 88) 
		ctx->stepindex = 88;
	ctx->stepsize = stepsizeTable[ctx->stepindex]; 
	return newSample;
}
signed short bit2_decode(bit2_Context *ctx, unsigned char sample){
	signed long difference=0;
	
	difference = 0; 
	if (sample & 1) 
		difference += ctx->stepsize;
	/*if (sample & 2)
		difference += ctx->stepsize >> 1;
	if (sample & 1)
		difference += ctx->stepsize >> 2;*/
	difference += ctx->stepsize >> 1;
	
	if (sample & 2) 
		difference = -difference;
	
	ctx->predicted += difference;
	if (ctx->predicted > 32767) 
		ctx->predicted = 32767;
	else if (ctx->predicted < -32768)
		ctx->predicted = -32768;
	
	ctx->stepindex += indexTable[sample];
	if (ctx->stepindex < 0) 
		ctx->stepindex = 0;
	else if (ctx->stepindex > 88) 
		ctx->stepindex = 88;
	ctx->stepsize = stepsizeTable[ctx->stepindex]; 
	
	return ctx->predicted;
}	
