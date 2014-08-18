//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "MediaStreamer.h"
#include <exception>
#include <wchar.h>

HANDLE mfLibrary = nullptr;

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

	IMFMediaType* outputMediaType;
	IMFMediaType* mediaType;

	// Load needed functions from Media Framework (MFStartup, MFCreateSourceReaderFromURL)
	//MFStartup
	//MFCreateSourceReaderFromURL
	//MFCreateMediaType
	//MFCreateWaveFormatExFromMFMediaType
	//

	ThrowIfFailed(
		MFStartup(MF_VERSION)
		);

	ThrowIfFailed(
		MFCreateSourceReaderFromURL(filePath, nullptr, &m_reader)
		);

	// Set the decoded output format as PCM
	// XAudio2 on Windows can process PCM and ADPCM-encoded buffers.
	// When using MF, this sample always decodes into PCM.

	ThrowIfFailed(
		MFCreateMediaType(&mediaType)
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
		MFCreateWaveFormatExFromMFMediaType(outputMediaType, &waveFormat, &formatSize)
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
		m_reader->SetCurrentPosition(GUID_NULL, var)
		);

}