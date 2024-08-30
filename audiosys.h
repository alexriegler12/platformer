#define NUM_VOICES 16
typedef struct audiosample{
    short *data;
    unsigned int length;

}AudioSample;
void audio_getsamples(signed short* buf,int num);
unsigned int audio_playsample(AudioSample  *sample,unsigned char volume);
AudioSample* audio_load2BitAdpcmFromFS(char* filename);
unsigned int audio_stillplaying(unsigned int voicenum);

void audio_initmidi();
void audio_loadmidi(char* filename);
void audio_getmidisamples(signed short *stream, int len);