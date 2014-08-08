//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "MediaStreamer.h"
#include <exception>
#include <wchar.h>
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
#include "pch.h"
using namespace Windows::ApplicationModel;
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#include <wrl\wrappers\corewrappers.h>
#include <ppltasks.h>

using namespace Microsoft::WRL;
using namespace Windows::Storage;
using namespace Windows::Storage::FileProperties;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;
using namespace Concurrency;

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
	((uint32)(byte)(ch0) | ((uint32)(byte)(ch1) << 8) | \
	((uint32)(byte)(ch2) << 16) | ((uint32)(byte)(ch3) << 24))
#endif /* defined(MAKEFOURCC) */
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
HANDLE mfLibrary = nullptr;
#endif

static inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
		// Set a breakpoint on this line to catch DirectX API errors
		throw Platform::Exception::CreateException(hr);
#else
		// TODO create an HRESULT exception
		throw std::exception();
#endif
	}
}


MediaStreamer::MediaStreamer()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	m_reader = nullptr;
	m_audioType = nullptr;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	m_offset = 0;
#endif

	ZeroMemory(&m_waveFormat, sizeof(m_waveFormat));

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	m_installedLocation = Package::Current->InstalledLocation;
	m_installedLocationPath = Platform::String::Concat(m_installedLocation->Path, "\\Assets\\Resources\\")->Data();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	m_installedLocationPath = L"";
#endif

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


#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)

	IMFMediaType* outputMediaType;
	IMFMediaType* mediaType;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	// Load needed functions from Media Framework (MFStartup, MFCreateSourceReaderFromURL)
	//MFStartup
	//MFCreateSourceReaderFromURL
	//MFCreateMediaType
	//MFCreateWaveFormatExFromMFMediaType
	//

#endif

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

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	Platform::Array<byte>^ data = ReadData(ref new Platform::String(filePath));
	UINT32 length = data->Length;
	const byte * dataPtr = data->Data;
	UINT32 offset = 0;

	DWORD riffDataSize = 0;

	auto ReadChunk = [&length, &offset, &dataPtr, &riffDataSize](DWORD fourcc, DWORD& outChunkSize, DWORD& outChunkPos) -> HRESULT
	{
		while (true)
		{
			if (offset + sizeof(DWORD)* 2 >= length)
			{
				return E_FAIL;
			}

			// Read two DWORDs.
			DWORD chunkType = *reinterpret_cast<const DWORD *>(&dataPtr[offset]);
			DWORD chunkSize = *reinterpret_cast<const DWORD *>(&dataPtr[offset + sizeof(DWORD)]);
			offset += sizeof(DWORD)* 2;

			if (chunkType == MAKEFOURCC('R', 'I', 'F', 'F'))
			{
				riffDataSize = chunkSize;
				chunkSize = sizeof(DWORD);
				outChunkSize = sizeof(DWORD);
				outChunkPos = offset;
			}
			else
			{
				outChunkSize = chunkSize;
				outChunkPos = offset;
			}

			offset += chunkSize;

			if (chunkType == fourcc)
			{
				return S_OK;
			}
		}
	};

	// Locate riff chunk, check the file type.
	DWORD chunkSize = 0;
	DWORD chunkPos = 0;

	ThrowIfFailed(ReadChunk(MAKEFOURCC('R', 'I', 'F', 'F'), chunkSize, chunkPos));
	if (*reinterpret_cast<const DWORD *>(&dataPtr[chunkPos]) != MAKEFOURCC('W', 'A', 'V', 'E')) ThrowIfFailed(E_FAIL);

	// Locate 'fmt ' chunk, copy to WAVEFORMATEXTENSIBLE.
	ThrowIfFailed(ReadChunk(MAKEFOURCC('f', 'm', 't', ' '), chunkSize, chunkPos));
	ThrowIfFailed((chunkSize <= sizeof(m_waveFormat)) ? S_OK : E_FAIL);
	CopyMemory(&m_waveFormat, &dataPtr[chunkPos], chunkSize);

	// Locate the 'data' chunk and copy its contents to a buffer.
	ThrowIfFailed(ReadChunk(MAKEFOURCC('d', 'a', 't', 'a'), chunkSize, chunkPos));
	m_data.resize(chunkSize);
	CopyMemory(m_data.data(), &dataPtr[chunkPos], chunkSize);

	m_offset = 0;

#endif
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
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
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

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	UINT32 toCopy = m_data.size() - m_offset;
	if (toCopy > maxBufferSize) toCopy = maxBufferSize;

	CopyMemory(buffer, m_data.data(), toCopy);
	*bufferLength = toCopy;

	m_offset += toCopy;
	if (m_offset > m_data.size()) m_offset = m_data.size();

#endif
}

void MediaStreamer::Restart()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	if (m_reader == nullptr)
	{
		return;
	}

	PROPVARIANT var = { 0 };
	var.vt = VT_I8;

	ThrowIfFailed(
		m_reader->SetCurrentPosition(GUID_NULL, var)
		);

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	m_offset = 0;

#endif
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) 
Platform::Array<byte>^ MediaStreamer::ReadData(_In_ Platform::String^ filename)
{
	CREATEFILE2_EXTENDED_PARAMETERS extendedParams = { 0 };
	extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
	extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
	extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
	extendedParams.lpSecurityAttributes = nullptr;
	extendedParams.hTemplateFile = nullptr;

	Wrappers::FileHandle file(
		CreateFile2(
		filename->Data(),
		GENERIC_READ,
		FILE_SHARE_READ,
		OPEN_EXISTING,
		&extendedParams
		)
		);
	if (file.Get() == INVALID_HANDLE_VALUE)
	{
		throw ref new Platform::FailureException();
	}

	FILE_STANDARD_INFO fileInfo = { 0 };
	if (!GetFileInformationByHandleEx(
		file.Get(),
		FileStandardInfo,
		&fileInfo,
		sizeof(fileInfo)
		))
	{
		throw ref new Platform::FailureException();
	}

	if (fileInfo.EndOfFile.HighPart != 0)
	{
		throw ref new Platform::OutOfMemoryException();
	}

	Platform::Array<byte>^ fileData = ref new Platform::Array<byte>(fileInfo.EndOfFile.LowPart);

	if (!ReadFile(
		file.Get(),
		fileData->Data,
		fileData->Length,
		nullptr,
		nullptr
		))
	{
		throw ref new Platform::FailureException();
	}

	return fileData;
}
#endif