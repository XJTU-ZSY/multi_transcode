#pragma once
#define HAVE_STRUCT_TIMESPEC
#include"rtmp.h"
#include"pthread.h"
#define AUDIOPACKETARRAYSIZE 10000
//constexpr auto AUDIOPACKETARRAYSIZE = 10000;
typedef unsigned char   BYTE;
typedef unsigned char* PBYTE;
typedef unsigned long   ULONG;
typedef unsigned int    UINT;
typedef unsigned short WORD;
//typedef unsigned int DWORD;
//typedef DWORD* PDWORD;
#define FILEBUFSIZE (1024 * 1024 * 10)
//constexpr auto FILEBUFSIZE = (1024 * 1024 * 10);
enum
{
	FLV_CODECID_H264 = 7,
};
typedef struct _NaluUnit
{
	int type;
	int size;
	unsigned char *data;
}NaluUnit;
typedef struct _RTMPMetadata
{
	unsigned int nWidth;
	unsigned int nHeigth;
	unsigned int nFrameRate;
	unsigned int nVideoDataRate;
	unsigned int nSpsLen;
	unsigned char Sps[1024];
	unsigned int nPpsLen;
	unsigned char Pps[1024];
	//audio
	bool bHasAudio;
	unsigned int nAudioSampleRate;
	unsigned int nAudioSampleSize;
	unsigned int nAudioChannels;
	char pAudioSpecCfg;
	unsigned int nAudioSpecCfgLen;
}RTMPMetadata, *LPRTMPMetadata;

typedef struct tagAudioPacketData
{
	BYTE m_abyAudioData[2048];
	int m_nDataSize;
	unsigned int m_dwPTS;
}AUDIO_PACKET_DATA, *PAUDIO_PACKET_DATA;

class CRtmpClient
{
public:
	CRtmpClient();
	virtual ~CRtmpClient();
public:
	int Initialize();
	bool Connect();
	//int Send(PBYTE pbuf, int nBufLen, int type, unsigned int dwTimeStamp, unsigned int nCTOffset = 0);
	int Send(PBYTE pbuf, int nBufLen, int type, unsigned int dwTimeStamp, unsigned int nCTOffset=0);
	int Close();
	void Reset();
	void SetVideoParam(int nVideoWidth, int nVideoHeight, int nVideoRate);
	void SetAudioParam(int nFreq);
	void VideoRateChanged(int nVideoRate);
	void SetAudioDelay(unsigned int dwAudioDelay);
	void SetURL(const char *strURL);
	void FreeRtmp();
	bool m_bHaveSendVideoMetaData;
	bool SendH264Packet(unsigned char *data, unsigned int size, bool bIsKeyFrame, unsigned int nTimeStamp, unsigned int nCTOffset);
	bool m_bRtmpEnable1;
	bool m_bIsEncoderEnable;
private:
	// 从缓存中读取一个NALU包
	bool ReadOneNaluFromBuf(NaluUnit &nalu);
	int ReadOneNaluFromBuf2(NaluUnit &nalu, BYTE *pByte, int nBufLen, unsigned int dwVideoTimeStamp,unsigned int dwCTOffset);
	//bool ReadOneNaluFromBuf(NaluUnit &nalu);
	void FillMetaData(NaluUnit &nalu);
	int SendRTMPPacket(PBYTE pbuf, int nBufLen, int type, unsigned int timestamp);
	int SendMetaData(LPRTMPMetadata pMetaData);
	int SendAACPacket(PBYTE pBuf, unsigned int size, ULONG dwAudioTimeStamp);
	int SetChunkSize();
	unsigned int keyframepts;
private:
	RTMP *m_pRtmp;
	bool m_bConnected;
	bool m_bHaveSendAudioInfo;
	
	ULONG pts;
	unsigned char* m_pFileBuf;
	unsigned int  m_nFileBufSize;
	unsigned int  m_nCurPos;
	bool m_bsps;
	bool m_bpps;
	RTMPMetadata m_metaData;

	unsigned int m_dwAudioDelay;
	char  m_strURL[100];
	unsigned int m_dwCTOffset;

	// mwb 2014.12.08 add  支持从播出端控制AudioDelay
	AUDIO_PACKET_DATA m_aAudioPacketData[AUDIOPACKETARRAYSIZE];
	int m_nWirteAudioPacketIndex;
	int m_nReadAudioPacketIndex;

	int m_nCacheAudioSampleCount;
	int m_nBufferAudioSampleCount;
	volatile bool m_bSendAudioPacket;
	ULONG tps;
public:
	int m_nVideoWidth;
	int m_nVideoHeight;
	int m_nVideoFrameRate;

	int m_nAudioFrameDur;
	int m_nVideoFrameDur;
};
