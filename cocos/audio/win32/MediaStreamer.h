//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef _MEDIA_STREAMER_H_
#define _MEDIA_STREAMER_H_

#include "CCStdC.h"
#include <string>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class MediaStreamer
{
private:
    WAVEFORMATEX                        m_waveFormat;
    UINT32                              m_maxStreamLengthInBytes;
	std::wstring m_installedLocationPath;

	IMFSourceReader* m_reader;
    IMFMediaType* m_audioType;
	bool GetNextBuffer(UINT8* buffer, UINT32 maxBufferSize, UINT32* bufferLength);	

public:
    MediaStreamer();
    virtual ~MediaStreamer();

    WAVEFORMATEX& GetOutputWaveFormatEx()
    {
        return m_waveFormat;
    }

    UINT32 GetMaxStreamLengthInBytes()
    {
        return m_maxStreamLengthInBytes;
    }

    void Initialize(_In_ const WCHAR* url); 
    void ReadAll(UINT8* buffer, UINT32 maxBufferSize, UINT32* bufferLength); 
    void Restart();
};

#endif