#include <stdint.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <Ksmedia.h>
#include <xaudio2.h>
/*
https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2-createsourcevoice:
XAudio2 supports the following PCM formats:
8-bit (unsigned) integer PCM
16-bit integer PCM (optimal format for XAudio2)
20-bit integer PCM (either in 24 or 32 bit containers)
24-bit integer PCM (either in 24 or 32 bit containers)
32-bit integer PCM
32-bit float PCM (preferred format after 16-bit integer)
*/
u8 sound[]={128,118,90,31,38,155,240,255,205,143,96,68,87,126,141,155,143,133,132,104,95,115,141,158,163,152,138,119,103,82,77,89,109,137,155,152,150,158,157,155,137,128,127,123,120,117,116,116,113,113,118,123,130,135,139,144,143,140,138,134,131,130,129,124,119,115,112,116,126,136,141,140,134,127,123,122,124,127,129,131,131,130,128,126,127,131,137,138,134,125,116,112,113,116,122,131,141,144,142,136,132,130,130,129,127,124,121,119,120,123,130,135,138,137,134,130,127,126,127,127,128,127,126,125,123,121,120,121,124,126,128,129,130,132,132,132,130,127,125,123,123,126,130,133,135,134,131,126,122,119,120,124,127,129,130,130,130,129,129,128,128,129,129,128,};
int main(){
    CoInitialize(NULL);
    IXAudio2* pXAudio2 = NULL;
    XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    IXAudio2MasteringVoice *pMasterVoice = NULL;
    pXAudio2->lpVtbl->CreateMasteringVoice(pXAudio2,&pMasterVoice,XAUDIO2_DEFAULT_CHANNELS,XAUDIO2_DEFAULT_SAMPLERATE,0,NULL,NULL,AudioCategory_GameEffects);
    IXAudio2SourceVoice *pSourceVoice = NULL;
    WAVEFORMATEX wfx = {WAVE_FORMAT_PCM,1,8000,8000,1,8,0};
    pXAudio2->lpVtbl->CreateSourceVoice(pXAudio2,&pSourceVoice,&wfx,0,XAUDIO2_DEFAULT_FREQ_RATIO,NULL,NULL,NULL);
    XAUDIO2_BUFFER buf = {0,sizeof(sound),sound,0,0,0,0,0,NULL};
    pSourceVoice->lpVtbl->SubmitSourceBuffer(pSourceVoice,&buf,NULL);
    pSourceVoice->lpVtbl->Start(pSourceVoice,0,XAUDIO2_COMMIT_NOW);
    Sleep(1000);
    return 0;
}