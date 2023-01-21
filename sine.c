#include <stdint.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef uint32_t u32;
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <Ksmedia.h>
DEFINE_GUID(CLSID_MMDeviceEnumerator,0xBCDE0395,0xE52F,0x467C,0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E);
DEFINE_GUID(IID_IMMDeviceEnumerator,0xA95664D2,0x9614,0x4F35,0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6);
DEFINE_GUID(IID_IAudioClient,0x1CB9AD4C,0xDBFA,0x4c32,0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2);
DEFINE_GUID(IID_IAudioRenderClient,0xF294ACFC,0x3146,0x4483,0xA7,0xBF,0xAD,0xDC,0xA7,0xC2,0x60,0xE2);
i8 sound[]={0,-4,-15,-37,-34,10,42,48,29,5,-13,-23,-16,-1,5,10,5,2,1,-9,-13,-5,5,11,13,9,3,-4,-10,-18,-20,-15,-7,3,10,9,8,11,11,10,3,0,-1,-2,-3,-5,-5,-5,-6,-6,-4,-2,0,2,4,6,5,4,3,2,1,0,0,-2,-4,-5,-6,-5,-1,3,5,4,2,-1,-2,-2,-2,-1,0,1,1,0,0,-1,-1,1,3,4,2,-1,-5,-6,-6,-5,-2,1,4,6,5,3,1,1,0,0,-1,-2,-3,-4,-3,-2,0,2,3,3,2,0,-1,-1,-1,-1,0,-1,-1,-2,-2,-3,-3,-3,-2,-1,0,0,0,1,1,1,0,-1,-1,-2,-2,-1,0,2,2,2,1,-1,-3,-4,-3,-2,-1,0,};
int main(){
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    WAVEFORMATEX *pwfx;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
    BYTE* pData;
    DWORD flags = 0;
    CoInitialize(NULL);
    CoCreateInstance(&CLSID_MMDeviceEnumerator,NULL,CLSCTX_ALL,&IID_IMMDeviceEnumerator,&pEnumerator);
    pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator,eRender,eConsole,&pDevice);
    pDevice->lpVtbl->Activate(pDevice,&IID_IAudioClient,CLSCTX_ALL,NULL,&pAudioClient);
    pAudioClient->lpVtbl->GetMixFormat(pAudioClient,&pwfx);
    UINT32 fmt;
    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE){
        if (((WAVEFORMATEXTENSIBLE*)pwfx)->SubFormat.Data1 == WAVE_FORMAT_PCM) fmt = WAVE_FORMAT_PCM;
        else if (((WAVEFORMATEXTENSIBLE*)pwfx)->SubFormat.Data1 == WAVE_FORMAT_IEEE_FLOAT) fmt = WAVE_FORMAT_IEEE_FLOAT;
        else {
            puts("Audio endpoint not supported.");
            goto EXIT;
        }
    } else if (pwfx->wFormatTag == WAVE_FORMAT_PCM) fmt = WAVE_FORMAT_PCM;
    else if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) fmt = WAVE_FORMAT_IEEE_FLOAT;
    else {
        puts("Audio endpoint not supported.");
        goto EXIT;
    }
    pAudioClient->lpVtbl->Initialize(pAudioClient,AUDCLNT_SHAREMODE_SHARED,0,10000000,0,pwfx,NULL);
    pAudioClient->lpVtbl->GetService(pAudioClient,&IID_IAudioRenderClient,&pRenderClient);
    unsigned int frameCount = sizeof(sound)*pwfx->nSamplesPerSec/8000;
    pRenderClient->lpVtbl->GetBuffer(pRenderClient,frameCount,&pData);
    /*
    8000 hz means 1 frame every 1/8000 sec,
    48000 hz means 1 frame every 1/48000 sec,
    48000/8000 = 6 frames of 48000hz = 1 frame of 8000 hz
    */
    if (fmt == WAVE_FORMAT_PCM){
        if (pwfx->wBitsPerSample == 8){
            for (u32 i = 0; i < frameCount; i++){
                for (u32 c = 0; c < pwfx->nChannels; c++){
                    ((i8*)pData)[i*pwfx->nChannels + c] = sound[i*8000/pwfx->nSamplesPerSec];
                }
            }
        } else if (pwfx->wBitsPerSample == 16){
            for (u32 i = 0; i < frameCount; i++){
                for (u32 c = 0; c < pwfx->nChannels; c++){
                    ((i16*)pData)[i*pwfx->nChannels + c] = sound[i*8000/pwfx->nSamplesPerSec];
                }
            }
        } else if (pwfx->wBitsPerSample == 32){
            for (u32 i = 0; i < frameCount; i++){
                for (u32 c = 0; c < pwfx->nChannels; c++){
                    ((i32*)pData)[i*pwfx->nChannels + c] = sound[i*8000/pwfx->nSamplesPerSec];
                }
            }
        } else {
            puts("Audio endpoint not supported");
            goto EXIT;
        }
    } else {
        if (pwfx->wBitsPerSample == 32){
            for (u32 i = 0; i < frameCount; i++){
                for (u32 c = 0; c < pwfx->nChannels; c++){
                    ((float*)pData)[i*pwfx->nChannels + c] = (float)sound[i*8000/pwfx->nSamplesPerSec] / 127.0f;
                }
            }
        } else {
            puts("Audio endpoint not supported");
            goto EXIT;
        }
    }
    pRenderClient->lpVtbl->ReleaseBuffer(pRenderClient,sizeof(sound),flags);
    pAudioClient->lpVtbl->Start(pAudioClient);
    Sleep(sizeof(sound) * 1000 / 8000);
    pAudioClient->lpVtbl->Stop(pAudioClient);
    EXIT:
    CoTaskMemFree(pwfx);
    pEnumerator->lpVtbl->Release(pEnumerator);
    pDevice->lpVtbl->Release(pDevice);
    pAudioClient->lpVtbl->Release(pAudioClient);
    pRenderClient->lpVtbl->Release(pRenderClient);
    printf("end\n");
    return 0;
}