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

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	#include "pch.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	#include <mfapi.h>
	#include <mfidl.h>
	#include <mfreadwrite.h>
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	#include <xaudio2.h>
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
ref
#endif
class MediaStreamer
{
private:
    WAVEFORMATEX                        m_waveFormat;
    UINT32                              m_maxStreamLengthInBytes;
	std::wstring m_installedLocationPath;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
    Windows::Storage::StorageFolder^    m_installedLocation;
#endif
	
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	bool GetNextBuffer(UINT8* buffer, UINT32 maxBufferSize, UINT32* bufferLength);	
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
    IMFSourceReader* m_reader;
    IMFMediaType* m_audioType;

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	std::vector<byte> m_data;
    UINT32            m_offset;
	
	Platform::Array<byte>^ ReadData(_In_ Platform::String^ filename);
#endif

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