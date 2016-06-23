/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SPRD_VPX_DECODER_H_
#define SPRD_VPX_DECODER_H_

#include <SprdSimpleOMXComponent.h>
#include <MemoryHeapIon_SPRD.h>

#include "vpx_dec_api.h"

#define SPRD_ION_DEV "/dev/ion"

#define VP8_DECODER_INTERNAL_BUFFER_SIZE  (0x200000)
#define ONEFRAME_BITSTREAM_BFR_SIZE	(1500*1024)  //for bitstream size of one encoded frame.

namespace android {

struct SPRDVPXDecoder : public SprdSimpleOMXComponent {
    SPRDVPXDecoder(const char *name,
                   const OMX_CALLBACKTYPE *callbacks,
                   OMX_PTR appData,
                   OMX_COMPONENTTYPE **component);

protected:
    virtual ~SPRDVPXDecoder();

    virtual OMX_ERRORTYPE internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR params);

    virtual OMX_ERRORTYPE internalUseBuffer(
            OMX_BUFFERHEADERTYPE **buffer,
            OMX_U32 portIndex,
            OMX_PTR appPrivate,
            OMX_U32 size,
            OMX_U8 *ptr,
            BufferPrivateStruct* bufferPrivate=NULL);

    virtual OMX_ERRORTYPE allocateBuffer(
        OMX_BUFFERHEADERTYPE **header,
        OMX_U32 portIndex,
        OMX_PTR appPrivate,
        OMX_U32 size);

    virtual OMX_ERRORTYPE freeBuffer(
        OMX_U32 portIndex,
        OMX_BUFFERHEADERTYPE *header);

    virtual void onQueueFilled(OMX_U32 portIndex);
    virtual void onPortFlushCompleted(OMX_U32 portIndex);
    virtual void onPortFlushPrepare(OMX_U32 portIndex);
    virtual void onPortEnableCompleted(OMX_U32 portIndex, bool enabled);
    virtual OMX_ERRORTYPE getExtensionIndex(
        const char *name, OMX_INDEXTYPE *index);

private:
    enum {
        kNumBuffers = 4
    };

    enum EOSStatus {
        INPUT_DATA_AVAILABLE,
        INPUT_EOS_SEEN,
        OUTPUT_FRAMES_FLUSHED,
    };

    tagVPXHandle *mHandle;

    size_t mInputBufferCount;

    int32_t mWidth;
    int32_t mHeight;

    int32 mMaxWidth, mMaxHeight;
    int mSetFreqCount;

    bool mSignalledError;

    bool mIOMMUEnabled;
    uint8_t *mPbuf_inter;

    sp<MemoryHeapIon> mPmem_stream;
    unsigned char* mPbuf_stream_v;
    int32 mPbuf_stream_p;
    int32 mPbuf_stream_size;

    sp<MemoryHeapIon> mPmem_extra;
    unsigned char*  mPbuf_extra_v;
    int32  mPbuf_extra_p;
    int32  mPbuf_extra_size;

    OMX_BOOL iUseAndroidNativeBuffer[2];

    EOSStatus mEOSStatus;
    void* mLibHandle;
    FT_VPXGetBufferDimensions mVPXGetBufferDimensions;
    FT_VPXGetCodecCapability mVPXGetCodecCapability;
    FT_VPXDecSetCurRecPic mVPXDecSetCurRecPic;
    FT_VPXDecInit mVPXDecInit;
    FT_VPXDecDecode mVPXDecDecode;
    FT_VPXDecRelease mVPXDecRelease;
    FT_VPXDecReleaseRefBuffers  mVPXDecReleaseRefBuffers;
    FT_VPXDecGetLastDspFrm mVPXDecGetLastDspFrm;

    static int32_t BindFrameWrapper(void *aUserData, void *pHeader, int flag);
    static int32_t UnbindFrameWrapper(void *aUserData, void *pHeader, int flag);

    int VSP_bind_cb(void *pHeader,int flag);
    int VSP_unbind_cb(void *pHeader,int flag);

    enum {
        NONE,
        AWAITING_DISABLED,
        AWAITING_ENABLED
    } mOutputPortSettingsChange;

    void initPorts();
    status_t initDecoder();
    bool drainAllOutputBuffers();
    void updatePortDefinitions();
    bool openDecoder(const char* libName);
    void set_ddr_freq(const char* freq_in_khz);
    void change_ddr_freq();

    DISALLOW_EVIL_CONSTRUCTORS(SPRDVPXDecoder);
};

}  // namespace android

#endif  // SPRD_VPX_DECODER_H_