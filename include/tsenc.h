#pragma once
#define HAVE_STRUCT_TIMESPEC
#include"pthread.h"
#define AUDIO_ES_ARRAY_SIZE		400
#define RTP_HEADER_SIZE			12
#define TS_PACKET_ARRAY_COUNT	4096
#define TS_PACKET_LEN			188
#define TS_PACKET_SEND_ONCE     7
#define TS_PCR_DELAY 9000
/*
constexpr auto AUDIO_ES_ARRAY_SIZE = 400;
constexpr auto RTP_HEADER_SIZE = 12;
constexpr auto TS_PACKET_ARRAY_COUNT = 4096;
constexpr auto TS_PACKET_LEN = 188;
constexpr auto TS_PACKET_SEND_ONCE = 7;
constexpr auto TS_PCR_DELAY = 9000;
*/
typedef unsigned char       BYTE;
typedef unsigned long       DWORD;
typedef unsigned short WORD;
typedef unsigned char* PBYTE;
typedef unsigned int        mfxU32;
typedef int                int32_t;
typedef unsigned long ULONG;
typedef long long          int64_t;
// 保存接收到的音频数据

typedef struct epginfo
{
	unsigned char pdname_size;
	unsigned char pdname[100];
	unsigned char data[2];
	BYTE start_time[2];
	int idnum;
	unsigned char size;
	BYTE name[256];
	//unsigned char index;
	
}epginfo;
class tsenc
{
public:
	tsenc();
	~tsenc();
	void init();
	void ts_write(PBYTE buf, mfxU32 len,ULONG dts,ULONG pts,int type,int roadnum);
	unsigned char* Write_PTS_DTS(PBYTE p, BYTE fb, ULONG pts);
	unsigned char* Fill_Stuff(unsigned char* pes_body_end,PBYTE packet, int body_size, mfxU32 in_size);
	bool CheckIsIDRFrame(BYTE *pByte, mfxU32 nBufLen);
	unsigned char* Write_PCR(unsigned char* p, int64_t pcr);
	int w_addr;
	int r_addr;
	PBYTE ts_buf[TS_PACKET_ARRAY_COUNT];
	void writeinfo(epginfo* ss);
	void writeaudio(unsigned char* p, ULONG pts, int len);
	void readaudio(ULONG pts,int roadnum);

private:
	int pindex;
	bool epginfostart;
	unsigned char* infostor;
	unsigned char* audiobuf[100];
	bool CheckIsPFrame(BYTE *pByte, mfxU32 nBufLen);
	ULONG audiopts[100];
	int audiolen[100];
	epginfo fuinfo[14];
	ULONG perdts;
	ULONG lastIpts;
	unsigned char numindex;
	BYTE m_byVideoContinueIndex;
	BYTE m_byAudioContinueIndex;
	BYTE m_byPATPMTContinue;
	int m_nTSEncapsuCount;
	pthread_mutex_t sccd;
	pthread_mutex_t audiolock;
	int audio_waddr;
	int audio_raddr;
};