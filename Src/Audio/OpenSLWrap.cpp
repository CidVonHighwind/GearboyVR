#include <assert.h>
#include <string.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <Kernel/OVR_LogUtils.h>

// engine interfaces
static SLObjectItf engineObject;
static SLEngineItf engineEngine;
static SLObjectItf outputMixObject;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLVolumeItf bqPlayerVolume;

#define BUFFER_SIZE 10000

// Double buffering.
static short buffer[3][BUFFER_SIZE];
static short empytBuffer[10000];
SLuint32 bufferSize[3];

static int curBuffer = 0;

int lastBuffer = 0;

void StartPlaying() {
    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, empytBuffer, 4410 * 2);
}

void SetBuffer(unsigned short *audio, unsigned sampleCount) {
    //LOG("copy to %i", lastBuffer);

    // drop data if the buffer is not big enough
    if (sampleCount > 10000)
        sampleCount = 10000;

    bufferSize[lastBuffer] = sampleCount;
    memcpy(&buffer[lastBuffer], audio, sampleCount * 2);

    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                                    buffer[lastBuffer], bufferSize[lastBuffer] * 2);

    // lastBuffer ^= 1;
    lastBuffer++;
    if(lastBuffer > 2)
        lastBuffer = 0;
}

// create the engine and output mix objects
void OpenSLWrap_Init() {
    // audioCallback = cb;

    SLresult result;
    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    assert(SL_RESULT_SUCCESS == result);
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                2, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    //result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    //assert(SL_RESULT_SUCCESS == result);
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);

    // Render and enqueue a first buffer. (or should we just play the buffer empty?)
    curBuffer = 0;
    // audioCallback(buffer[curBuffer], BUFFER_SIZE_IN_SAMPLES);

    bufferSize[0] = 1476;
    bufferSize[1] = 1476;
    bufferSize[2] = 1476;

    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer[curBuffer],
                                             bufferSize[curBuffer] * 2);
    if (SL_RESULT_SUCCESS != result) {
        //  return false;
    }
}