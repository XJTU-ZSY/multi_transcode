#pragma once
#define HAVE_STRUCT_TIMESPEC
#include"pthread.h"
#include"stdint.h"
#include "mfxplugin.h"
#include "mfxplugin++.h"
#include "mfxvideo.h"
#include "mfxvideo++.h"
#include"windows.h"

extern "C"
{
#include <libavformat/avformat.h>
}
class decodepool
{
public:
	decodepool();
	~decodepool();
	void init();
	void  putts(uint8_t *buf, int size ,int64_t TimeStamp);
	int r_ead_data(uint8_t *buf, int buf_size, mfxU64* TimeStamp,int i);
	int g_et_queue(uint8_t *buf, int size, mfxU64* TimeStamp);
	int weith;
private:
	volatile int w_addr;
	volatile int r_addr;
	pthread_mutex_t l_ock;
	//uint8_t *b_buf;
	uint8_t* b_buf[16];
	int bufsize1 ;
	int numberinde;
	bool sendd;
};