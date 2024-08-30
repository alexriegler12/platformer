#include "audiosys.h"
#include "2badpcm.h"
#define TSF_IMPLEMENTATION
#include "tsf.h"

#define TML_IMPLEMENTATION
#include "tml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct audiovoice{
    AudioSample  *sample;
    unsigned int position;
    unsigned int active;
    unsigned int age;
    unsigned char volume;
}AudioVoice;
int currentVoiceAge=0;
AudioVoice audioVoices[NUM_VOICES];
void audio_getsamples(signed short* buf,int num){
	int i,j;
	signed int mixvar=0;
	for(i=0;i<num;i++){
		mixvar=0;
		for(j=0;j<NUM_VOICES;j++){
			if(audioVoices[j].active==1){
				mixvar+=((audioVoices[j].sample->data[audioVoices[j].position])*audioVoices[j].volume)/255;
                audioVoices[j].position++;
                if(audioVoices[j].position>=audioVoices[j].sample->length){
                    audioVoices[j].active=0;
                }
				/*printf("mixvar=%i\n",mixvar);*/
			}
		}
		if(mixvar>32767){
			mixvar=32767;
		}else if(mixvar<-32768){
			mixvar=-32768;
		}
		buf[i]=mixvar;
	}
	
}
unsigned int audio_playsample(AudioSample  *sample,unsigned char volume){
    int i;
	char voice=-1;
	for(i=0;i<NUM_VOICES;i++){
		if(audioVoices[i].active==0){
			voice=i;
			break;
		}
		
	}
	if(voice==-1){/*Note Steal*/
		if(voice==-1){
			unsigned int minagechan=0;
			for(i=0;i<NUM_VOICES;i++){
				if(audioVoices[i].age<audioVoices[minagechan].age){
					minagechan=i;
				}	
		
			}
			voice=minagechan;
		}
		
	}
    audioVoices[voice].age=currentVoiceAge++;
    audioVoices[voice].sample=sample;
    audioVoices[voice].position=0;
    audioVoices[voice].active=1;
    audioVoices[voice].volume=volume;
    return voice;
}
AudioSample* audio_load2BitAdpcmFromFS(char* filename){
    FILE *in;
    unsigned int size;
    unsigned char inc;
    AudioSample* smp;
    bit2_Context ctx;
	bit2_init(&ctx);
    smp=(AudioSample*)malloc(sizeof(AudioSample));
    memset(smp,0,sizeof(AudioSample));
    in=fopen(filename,"rb");
    fseek(in, 0, SEEK_END);
	size = ftell(in);
    fseek(in, 0, SEEK_SET);
    smp->data=malloc(4*size*2);
    smp->length=4*size;
    printf("\n%d\n",smp->length);
    for(int i=0;i<size;i++){
			short *outc=&(smp->data[i*4]);
            fread(&inc,1,1,in);
			outc[0]=bit2_decode(&ctx,inc&0x3);
			outc[1]=bit2_decode(&ctx,(inc&0xC)>>2);
			outc[2]=bit2_decode(&ctx,(inc&0x30)>>4);
			outc[3]=bit2_decode(&ctx,(inc&0xC0)>>6);
			
	}
    fclose(in);
    return smp;
}
unsigned int audio_stillplaying(unsigned int voicenum){
    return audioVoices[voicenum].active;
}





static tsf* g_TinySoundFont;


static double g_Msec;               
static tml_message* g_MidiMessage;  
static tml_message* g_FirstMidiMessage;  

void audio_initmidi(){
    g_TinySoundFont = tsf_load_filename("soundbank.sf2");
    tsf_set_output(g_TinySoundFont, TSF_MONO, 22050, 0.0f);
}
void audio_loadmidi(char* filename){
    g_FirstMidiMessage = tml_load_filename(filename);
    g_MidiMessage=g_FirstMidiMessage;  
    g_Msec=0;
}
void audio_getmidisamples(signed short *stream, int len)
{
	

		//Loop through all MIDI messages which need to be played up until the current playback time
		for (g_Msec += (len) * (1000.0 / 22050.0); g_MidiMessage && g_Msec >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next)
		{
            if (g_MidiMessage==NULL){
                g_MidiMessage=g_FirstMidiMessage;  
                g_Msec=0;
                continue;
            }
			switch (g_MidiMessage->type)
			{
				case TML_PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
					tsf_channel_set_presetnumber(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->program, (g_MidiMessage->channel == 9));
					break;
				case TML_NOTE_ON: //play a note
					tsf_channel_note_on(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
					break;
				case TML_NOTE_OFF: //stop a note
					tsf_channel_note_off(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key);
					break;
				case TML_PITCH_BEND: //pitch wheel modification
					tsf_channel_set_pitchwheel(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
					break;
				case TML_CONTROL_CHANGE: //MIDI controller messages
					tsf_channel_midi_control(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
					break;
			}
		}

		// Render the block of audio samples in float format
		tsf_render_short(g_TinySoundFont, (signed short *)stream, len, 0);
	
}