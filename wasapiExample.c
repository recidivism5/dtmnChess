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
i8 sound[]={0,-10,-37,-94,-88,26,108,124,74,15,-31,-58,-39,-2,13,26,15,5,4,-24,-32,-13,13,29,34,23,10,-9,-25,-45,-49,-38,-18,8,26,23,21,29,28,26,9,0,-1,-5,-7,-11,-12,-12,-14,-14,-10,-5,2,7,11,15,15,12,9,6,3,2,1,-4,-9,-13,-15,-11,-2,7,12,12,6,-1,-5,-5,-4,-1,1,3,3,2,0,-2,-1,3,8,10,6,-3,-11,-15,-15,-12,-5,3,12,16,13,8,4,2,2,1,-1,-4,-7,-9,-8,-5,1,7,9,9,6,2,-1,-2,-1,-1,0,-1,-2,-3,-5,-7,-8,-7,-4,-2,0,1,2,4,4,4,2,-1,-3,-5,-5,-2,2,5,6,6,3,-2,-6,-9,-7,-4,-1,1,2,2,2,1,1,0,0,1,1,0,};
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