#include"udprecv.h"
#define BUF_SIZE 1024
//constexpr auto BUF_SIZE = 1024;
udprecv::udprecv()
{}

udprecv::~udprecv()
{
	av_free(q_buf);
	pthread_mutex_destroy(&lock);
}
bool udprecv::getss(unsigned char *tsHeader)
{
	bool a = false;

		while (waddr == 0 && raddr == 0)
			Sleep(10);
		pthread_mutex_lock(&lock);
		memset(tsHeader, 0, 188);
		if (waddr > raddr)
		{
			if (raddr + 188 < waddr)
			{
				memcpy(tsHeader, q_buf + raddr, 188);
				raddr += 188;
				a = true;
			}

		}
		else
		{
			if ((bufsize - raddr) > 188)
			{
				memcpy(tsHeader, q_buf + raddr, 188);
				raddr += 188;
				a = true;
			}
			else if ((bufsize - raddr + waddr) > 188)
			{
				memcpy(tsHeader, q_buf + raddr, bufsize - raddr);
				memcpy(tsHeader + bufsize - raddr, q_buf, 188 + raddr - bufsize);
				raddr = 188 + raddr - bufsize;
				a = true;
			}
		}
		pthread_mutex_unlock(&lock);
		return a;
	}

void udprecv::init()
{
	pthread_mutex_init(&lock, NULL);
	q_buf = (uint8_t*)av_mallocz(sizeof(uint8_t) * 1024 * 128*10);
	waddr = 0;
	raddr = 0;
	bufsize = 1024 * 128*10;
	indexnumber = 0;
}

void udprecv:: put_queue(uint8_t *buf, int size)
{
	pthread_mutex_lock(&lock);
	/*if (size != 752)
	{
		pthread_mutex_unlock(&lock);
		return;
	}*/
	/*else 
	{
		int i = 1;
		unsigned char p = buf[i];
		while (i<752)
		{
			if((p>>7)==1)
			{
				pthread_mutex_unlock(&lock);
				return;
			}
			i = i + 188;
			p = buf[i];
		}
	}*/
	if (waddr + size > bufsize)
	{
		memcpy(q_buf + waddr, buf, (bufsize - waddr));
		memcpy(q_buf, buf + (bufsize - waddr), size - bufsize + waddr);
	}
	else
	{
		memcpy(q_buf + waddr, buf, size);
	}
	waddr = (waddr + size) % bufsize;
	
	pthread_mutex_unlock(&lock);
}


int udprecv::get_queue(uint8_t *buf, int size)
{
	pthread_mutex_lock(&lock);
	int wrap = 0;
	int pos = waddr;
	if (pos < raddr)
	{
		pos += bufsize;
		if (size + raddr > bufsize)
			wrap = 1;
	}
	if ((raddr + size) >= pos)
	{
		pthread_mutex_unlock(&lock);
		indexnumber++;
		return 1;
	}
	if (wrap == 1)
	{
		memcpy(buf, q_buf + raddr, (bufsize - raddr));
		memcpy(buf + bufsize - raddr, q_buf, size - bufsize + raddr);
	}
	else
	{memcpy(buf, q_buf + raddr, size);
     }
	raddr = (raddr + size) % bufsize;
	indexnumber = 0;
	pthread_mutex_unlock(&lock);
	
	return 0;
}