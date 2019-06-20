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
class udprecv
{
public:
	udprecv();
	~udprecv();
	void init();
	void put_queue(uint8_t *buf, int size);
	int get_queue(uint8_t *buf, int size);
	bool getss(unsigned char *tsHeader);
	int indexnumber;
private:
     pthread_mutex_t lock;
	 volatile int waddr;
	 volatile int raddr;
	 uint8_t *q_buf;
	 int bufsize;
	 
};