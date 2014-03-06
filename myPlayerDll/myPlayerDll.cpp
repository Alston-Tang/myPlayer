// myPlayerDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "myPlayerDll.h"
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <time.h>

using namespace std;

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"winmm.lib")

#pragma pack(2)
struct riff
{
	char chunkID[4];
	INT32 chunkSize;
	char format[4];
};
struct fmt
{
	char subChunkID[4];
	INT32 subChunkSize;
	INT16 audioFormat;
	INT16 numChannels;
	INT32 sampleRate;
	INT32 byteRate;
	INT16 blockAlign;
	INT16 bitPerSample;
	INT16 extraParamSize;
	char extraParam[32];
};
struct data
{
	char subChunkID[4];
	INT32 subChunkSize;
	char* content;
};
#pragma pack()

int test()
{
	return 10;
}
LPDIRECTSOUND device = NULL;
LPDIRECTSOUNDBUFFER buffer = NULL;
HMMIO waveFile;
int totalTime;
struct riff myRiff;
struct fmt myFmt;
struct data myData;


int init()
{
	if (DirectSoundCreate(NULL, &device, NULL) != DS_OK) return -3;
	if (device->SetCooperativeLevel(GetDesktopWindow(), DSSCL_NORMAL) != DS_OK) return -4;

	return 0;
}

int loadFile(wchar_t* filePath)
{
	waveFile = mmioOpen(filePath, NULL, MMIO_READ);
	if (waveFile == NULL) return -1;
	
	mmioRead(waveFile, (char*)&myRiff, 12);
	mmioRead(waveFile, (char*)&myFmt, 8);
	mmioRead(waveFile, (char*)&myFmt.audioFormat, myFmt.subChunkSize);
	mmioRead(waveFile, (char*)&myData, 8);

	totalTime = myData.subChunkSize / myFmt.byteRate+1;

	DSBUFFERDESC bufferDes;
	WAVEFORMATEX waveFormat;

	waveFormat.cbSize = 0;
	waveFormat.nAvgBytesPerSec = myFmt.byteRate;
	waveFormat.nBlockAlign = myFmt.blockAlign;
	waveFormat.nChannels = myFmt.numChannels;
	waveFormat.nSamplesPerSec = myFmt.sampleRate;
	waveFormat.wBitsPerSample = myFmt.bitPerSample;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;

	bufferDes.dwBufferBytes = 3*myFmt.byteRate;
	bufferDes.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS | DSBCAPS_GLOBALFOCUS;
	bufferDes.dwReserved = 0;
	bufferDes.dwSize = sizeof(DSBUFFERDESC);
	bufferDes.lpwfxFormat = &waveFormat;

	if (buffer != NULL)
	{
		buffer->Stop();
		buffer->Restore();
		buffer = NULL;
	}

	if (FAILED(device->CreateSoundBuffer(&bufferDes, &buffer, NULL))) return -5;

	return 0;
}

int play()
{
	LPVOID writePointer1 = NULL;
	LPVOID writePointer2 = NULL;
	DWORD byteCount1 = 0;
	DWORD byteCount2 = 0;
	DWORD writeCount = 0;
	DWORD playCusor = 0;
	DWORD writeCusor = 0;
	DWORD freeBuffer = 3*myFmt.byteRate;
	DWORD remainData = 0;
	DWORD bufferSize = 3*myFmt.byteRate;

	myData.content = new char[3*myFmt.byteRate];
	
	long rLength = mmioRead(waveFile, myData.content, 3 * myFmt.byteRate);
	if (rLength != 3 * myFmt.byteRate) return -2;

	if (buffer->Lock(0, 3*myFmt.byteRate, &writePointer1, &byteCount1, &writePointer2, &byteCount2, 0L) != DS_OK) return -6;
	memcpy(writePointer1, myData.content, 3*myFmt.byteRate);
	if (FAILED(buffer->Unlock(writePointer1, byteCount1, writePointer2, byteCount2))) return -6;
	writeCount = 3*myFmt.byteRate;

	buffer->Play(0, 0, DSBPLAY_LOOPING);

	while (1)
	{
		if (writeCount == bufferSize) writeCount = 0;
		buffer->GetCurrentPosition(&playCusor, &writeCusor);
		if (writeCount <= writeCusor) remainData = bufferSize - writeCusor + writeCount;
		else remainData = writeCount - writeCusor;
		freeBuffer = bufferSize - writeCount;
		

		if (remainData < myFmt.byteRate)
		{
			rLength = mmioRead(waveFile, myData.content, myFmt.byteRate);
			if (rLength != myFmt.byteRate) return -2;
			
			if (buffer->Lock(writeCount, myFmt.byteRate, &writePointer1, &byteCount1, &writePointer2, &byteCount2, 0L) != DS_OK) return -6;
			memcpy(writePointer1, myData.content, byteCount1);
			writeCount += myFmt.byteRate;

			if (writePointer2 != NULL)
			{
				memcpy(writePointer2, &myData.content[byteCount1 + 1], byteCount2);
				writeCount = byteCount2;
			}
			if (FAILED(buffer->Unlock(writePointer1, byteCount1, writePointer2, byteCount2))) return -6;
		}
		Sleep(10);

	}
	return 0;
}

int stop()
{
	buffer->Stop();
	return 0;
}