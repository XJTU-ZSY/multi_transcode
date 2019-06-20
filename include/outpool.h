#pragma once
#define HAVE_STRUCT_TIMESPEC
#include<pthread.h>
#include"mfxcommon.h"
extern"C"
{
#include"libavformat/avformat.h"
}
typedef unsigned char BYTE;
typedef unsigned char* PBYTE;
typedef struct tagsample
{
	mfxU64 ITimeStamp;
	mfxU32 ISampleLength;
	unsigned char *abySample;
}SAMPLE,*PSAMPLE;


class outpool
{
public:
	outpool(int weith);
	virtual ~outpool(void);
	bool Get(uint8_t* Vsample, mfxU64 *ITimeStamp,mfxU32 *ISampleLength);
	bool Get(uint8_t* Vsample, mfxU32 *ISampleLength, mfxU64 *ITimeStamp);
	bool Write(mfxBitstream *pMfxBitstream);
	void Write(AVPacket* audio_pkt);
	int GetSamplecount();
	void writecontex(int i);
	bool readcontx();
private:
	PBYTE m_pabyBuffer[16];
	PBYTE m_pabyBuffer_A[1000];
	volatile int m_Read_A;
	volatile int m_Wead_A;
	volatile unsigned long m_Read;
	volatile unsigned long m_Wead;
	unsigned long m_IBufferSize;
	pthread_mutex_t m_mutex;
	pthread_mutex_t m_mutex1;
	pthread_mutex_t lockcontex;
	int Samplecount;
	int contex;
	//PSAMPLE pSample;
};