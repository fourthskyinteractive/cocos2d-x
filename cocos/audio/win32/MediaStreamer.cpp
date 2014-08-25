//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#define INITGUID 1

#include "MediaStreamer.h"
#include <exception>
#include <wchar.h>
#include <mutex>


#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

// Media Framework helper objects
static GUID _GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

HMODULE mfPlatLibrary = nullptr;
typedef HRESULT(STDAPICALLTYPE *MFStartupFunc)(ULONG Version, DWORD dwFlags/* = MFSTARTUP_FULL*/);
typedef HRESULT(STDAPICALLTYPE *MFCreateMediaTypeFunc)(IMFMediaType**  ppMFType);
typedef HRESULT(STDAPICALLTYPE *MFCreateWaveFormatExFromMFMediaTypeFunc)(IMFMediaType* pMFType, WAVEFORMATEX** ppWF, UINT32* pcbSize, UINT32 Flags/* = MFWaveFormatExConvertFlag_Normal*/);
MFStartupFunc _MFStartup;
MFCreateMediaTypeFunc _MFCreateMediaType;
MFCreateWaveFormatExFromMFMediaTypeFunc _MFCreateWaveFormatExFromMFMediaType;

HMODULE mfReadWriteLibrary = nullptr;
typedef HRESULT(STDAPICALLTYPE *MFCreateSourceReaderFromURLFunc)(LPCWSTR pwszURL, IMFAttributes *pAttributes, IMFSourceReader **ppSourceReader);
MFCreateSourceReaderFromURLFunc _MFCreateSourceReaderFromURL;


std::mutex init_mutex;

static inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// TODO create an HRESULT exception
		throw std::exception();
	}
}

MediaStreamer::MediaStreamer()
{
	m_reader = nullptr;
	m_audioType = nullptr;

	ZeroMemory(&m_waveFormat, sizeof(m_waveFormat));

	// TODO Obtain install location
	m_installedLocationPath = L"";

}

MediaStreamer::~MediaStreamer()
{
	if (m_reader != nullptr) {
		m_reader->Release();
	}

	if (m_audioType != nullptr) {
		m_audioType->Release();
	}
}

void MediaStreamer::Initialize(__in const WCHAR* url)
{
	WCHAR filePath[MAX_PATH] = { 0 };
	if ((wcslen(url) > 1 && url[1] == ':'))
	{
		// path start with "x:", is absolute path
		wcscat_s(filePath, url);
	}
	else if (wcslen(url) > 0
		&& (L'/' == url[0] || L'\\' == url[0]))
	{
		// path start with '/' or '\', is absolute path without driver name
		wcscat_s(filePath, m_installedLocationPath.data());
		// remove '/' or '\\'
		wcscat_s(filePath, (const WCHAR*)url[1]);
	}
	else
	{
		wcscat_s(filePath, m_installedLocationPath.data());
		wcscat_s(filePath, url);
	}

	// Init Media Framework
	{
		std::lock_guard<std::mutex> lock(init_mutex);

		if (mfReadWriteLibrary == nullptr || mfReadWriteLibrary == nullptr)
		{
			mfPlatLibrary = LoadLibrary(TEXT("MFPlat.dll"));
			if (mfPlatLibrary == nullptr)
			{
				CCLOG("Could not find MFPlay library");
				return;
			}
			_MFStartup = (MFStartupFunc)GetProcAddress(mfPlatLibrary, "MFStartup");
			_MFCreateMediaType = (MFCreateMediaTypeFunc)GetProcAddress(mfPlatLibrary, "MFCreateMediaType");
			_MFCreateWaveFormatExFromMFMediaType = (MFCreateWaveFormatExFromMFMediaTypeFunc)GetProcAddress(mfPlatLibrary, "MFCreateWaveFormatExFromMFMediaType");

			mfReadWriteLibrary = LoadLibrary(TEXT("MFReadWrite.dll"));
			if (mfReadWriteLibrary == nullptr)
			{
				CCLOG("Could not find MFReadWrite library");
				return;
			}
			_MFCreateSourceReaderFromURL = (MFCreateSourceReaderFromURLFunc) GetProcAddress(mfReadWriteLibrary, "MFCreateSourceReaderFromURL");

		}
	}

	IMFMediaType* outputMediaType;
	IMFMediaType* mediaType;

	ThrowIfFailed(
		_MFStartup(MF_VERSION, MFSTARTUP_FULL)
		);

	ThrowIfFailed(
		_MFCreateSourceReaderFromURL(filePath, nullptr, &m_reader)
		);

	// Set the decoded output format as PCM
	// XAudio2 on Windows can process PCM and ADPCM-encoded buffers.
	// When using MF, this sample always decodes into PCM.

	ThrowIfFailed(
		_MFCreateMediaType(&mediaType)
		);

	ThrowIfFailed(
		mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)
		);

	ThrowIfFailed(
		mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)
		);

	ThrowIfFailed(
		m_reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, mediaType)
		);

	// Get the complete WAVEFORMAT from the Media Type
	ThrowIfFailed(
		m_reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outputMediaType)
		);

	UINT32 formatSize = 0;
	WAVEFORMATEX* waveFormat;
	ThrowIfFailed(
		_MFCreateWaveFormatExFromMFMediaType(outputMediaType, &waveFormat, &formatSize, MFWaveFormatExConvertFlag_Normal)
		);
	CopyMemory(&m_waveFormat, waveFormat, sizeof(m_waveFormat));
	CoTaskMemFree(waveFormat);

	// Get the total length of the stream in bytes
	PROPVARIANT var;
	ThrowIfFailed(
		m_reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var)
		);
	LONGLONG duration = var.uhVal.QuadPart;
	double durationInSeconds = (duration / static_cast<double>(10000000)); // duration is in 100ns units, convert to seconds
	m_maxStreamLengthInBytes = static_cast<unsigned int>(durationInSeconds * m_waveFormat.nAvgBytesPerSec);

	// Round up the buffer size to the nearest four bytes
	m_maxStreamLengthInBytes = (m_maxStreamLengthInBytes + 3) / 4 * 4;

	outputMediaType->Release();
	mediaType->Release();
}

bool MediaStreamer::GetNextBuffer(UINT8* buffer, UINT32 maxBufferSize, UINT32* bufferLength)
{
	IMFSample* sample;
	IMFMediaBuffer* mediaBuffer;

	BYTE *audioData = nullptr;
	DWORD sampleBufferLength = 0;
	DWORD flags = 0;

	*bufferLength = 0;

	if (m_reader == nullptr)
	{
		return false;
	}

	ThrowIfFailed(
		m_reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, &sample)
		);

	if (sample == nullptr)
	{
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	ThrowIfFailed(
		sample->ConvertToContiguousBuffer(&mediaBuffer)
		);

	ThrowIfFailed(
		mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength)
		);

	// If buffer isn't large enough, dump sample
	if (sampleBufferLength <= maxBufferSize)
	{
		CopyMemory(buffer, audioData, sampleBufferLength);
		*bufferLength = sampleBufferLength;
	}
	else
	{
#if defined(COCOS2D_DEBUG)
		OutputDebugString(L"Sample buffer dumped");
#endif
	}

	sample->Release();
	mediaBuffer->Release();

	if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MediaStreamer::ReadAll(UINT8* buffer, UINT32 maxBufferSize, UINT32* bufferLength)
{
	UINT32 valuesWritten = 0;
	UINT32 sampleBufferLength = 0;

	if (m_reader == nullptr)
	{
		return;
	}

	*bufferLength = 0;
	// If buffer isn't large enough, return
	if (maxBufferSize < m_maxStreamLengthInBytes)
	{
		return;
	}

	while (!GetNextBuffer(buffer + valuesWritten, maxBufferSize - valuesWritten, &sampleBufferLength))
	{
		valuesWritten += sampleBufferLength;
	}

	*bufferLength = valuesWritten + sampleBufferLength;

}

void MediaStreamer::Restart()
{
	if (m_reader == nullptr)
		return;

	PROPVARIANT var = { 0 };
	var.vt = VT_I8;

	ThrowIfFailed(
		m_reader->SetCurrentPosition(_GUID_NULL, var)
		);

}