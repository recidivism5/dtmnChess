#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#define REFTIMES_PER_SEC  5000000
#define REFTIMES_PER_MILLISEC  10000
DEFINE_GUID(CLSID_MMDeviceEnumerator,0xBCDE0395,0xE52F,0x467C,0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E);
DEFINE_GUID(IID_IMMDeviceEnumerator,0xA95664D2,0x9614,0x4F35,0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6);
DEFINE_GUID(IID_IAudioClient,0x1CB9AD4C,0xDBFA,0x4c32,0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2);
DEFINE_GUID(IID_IAudioRenderClient,0xF294ACFC,0x3146,0x4483,0xA7,0xBF,0xAD,0xDC,0xA7,0xC2,0x60,0xE2);
int main(){
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
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
    pAudioClient->lpVtbl->Initialize(pAudioClient,AUDCLNT_SHAREMODE_SHARED,0,hnsRequestedDuration,0,pwfx,NULL);
    WAVEFORMATEXTENSIBLE format;
    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) format = *(WAVEFORMATEXTENSIBLE*)pwfx;
    else{
        puts("Error: Unsupported MixFormat");
        goto EXIT;
    }
    UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID(&format.SubFormat);
    if (formatTag != WAVE_FORMAT_IEEE_FLOAT){
        puts("Error: Unsupported MixFormat");
        goto EXIT;
    }
    pAudioClient->lpVtbl->GetBufferSize(pAudioClient,&bufferFrameCount);
    pAudioClient->lpVtbl->GetService(pAudioClient,&IID_IAudioRenderClient,&pRenderClient);
    pRenderClient->lpVtbl->GetBuffer(pRenderClient,bufferFrameCount,&pData);
    float *d = pData, rad = 0;
    for(UINT32 i = 0; i < bufferFrameCount; i++){
        for (int j = 0; j < format.Format.nChannels; j++)
            d[i*format.Format.nChannels + j] = sinf(rad);
        rad += 1200 * 2 * M_PI / format.Format.nSamplesPerSec;
    }
    pRenderClient->lpVtbl->ReleaseBuffer(pRenderClient,bufferFrameCount,flags);
    hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;
    pAudioClient->lpVtbl->Start(pAudioClient);
    Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);
    pAudioClient->lpVtbl->Stop(pAudioClient);
    EXIT:
    CoTaskMemFree(pwfx);
    pEnumerator->lpVtbl->Release(pEnumerator);
    pDevice->lpVtbl->Release(pDevice);
    pAudioClient->lpVtbl->Release(pAudioClient);
    pRenderClient->lpVtbl->Release(pRenderClient);
    return 0;
}