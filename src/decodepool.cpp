#include"decodepool.h"

decodepool::decodepool()
{
	weith = 0;
	pthread_mutex_init(&l_ock, NULL);
}
decodepool::~decodepool()
{
	av_free(b_buf);
	pthread_mutex_destroy(&l_ock);
}
void decodepool::init()
{
	if (weith == 720 || weith == 704)
	{
		for (int i = 0; i < 16; i++)
			b_buf[i] = new uint8_t[1024 * 300];
		bufsize1 = 1024 * 300;
	}
	else if (weith == 1920)
	{
		for (int i = 0; i < 16; i++)
			b_buf[i] = new uint8_t[1024 * 1024];
		bufsize1 = 1024 * 1024;
	}
	r_addr = 0;
	w_addr = 0;
	
	numberinde = 0;
	
}

/*int decodepool::g_et_queue(uint8_t *buf, int size, mfxU64* TimeStamp)
{
	pthread_mutex_lock(&l_ock);
	int wrap = 0;
	int pos = w_addr;
	if (pos == r_addr)
	{
		pthread_mutex_unlock(&l_ock);
		return 1;
	}
	if (pos < r_addr)
	{
		pos += bufsize1;
		if (size + r_addr > bufsize1)
			wrap = 1;
	}
	if ((r_addr + size) > pos)
	{
		pthread_mutex_unlock(&l_ock);
		return 1;
	}
	if (wrap == 1)
	{
		memcpy(buf, b_buf + r_addr, (bufsize1 - r_addr));
		memcpy(buf + bufsize1 - r_addr, b_buf, size + r_addr - bufsize1);
	}
	else
	{
		memcpy(buf, b_buf + r_addr, size);

	}
	r_addr = (r_addr + size) % bufsize1;

	pthread_mutex_unlock(&l_ock);
	return 0;
}*/

int decodepool::g_et_queue(uint8_t *buf, int size, mfxU64* TimeStamp)
{
	pthread_mutex_lock(&l_ock);
	if (r_addr == w_addr)
	{
		pthread_mutex_unlock(&l_ock);
		return 1;
	}

	int size1;
	memcpy(&size1, b_buf[r_addr], 4);
	if (size1>bufsize1 || size1>size)
	{
		r_addr++;
		pthread_mutex_unlock(&l_ock);
		return 1;
	}
	
	memcpy(TimeStamp, b_buf[r_addr] + 4, 8);
	memcpy(buf, b_buf[r_addr] + 12, size1);
	r_addr++;
	if (r_addr == 16)
		r_addr = 0;

	numberinde = 0;
	pthread_mutex_unlock(&l_ock);
	return size1;
}
int decodepool::r_ead_data(uint8_t *buf, int buf_size, mfxU64* TimeStamp,int i)
{
	int size = buf_size;
	int ret;
	do {
		ret = g_et_queue(buf, size, TimeStamp);
		if (numberinde >= 3000)
		{
			printf("%d demux not send data\n",i);
			numberinde = 0;
		}
		if (ret == 1)
		{
			Sleep(10);
			numberinde++;
		}
	} while (ret==1);
	return ret;
}
void decodepool::putts(uint8_t *buf, int size, int64_t TimeStamp)
{
	pthread_mutex_lock(&l_ock);
	if (size > bufsize1)
	{
		printf("size out \n");
		pthread_mutex_unlock(&l_ock);
		return;
	}


	memcpy(b_buf[w_addr], &size, 4);
	memcpy(b_buf[w_addr] + 4, &TimeStamp, 8);
	memcpy(b_buf[w_addr] + 12, buf, size);
	w_addr++;
	if (w_addr == 16)
		w_addr = 0;
	pthread_mutex_unlock(&l_ock);
}
/*void decodepool::putts(uint8_t *buf, int size, int64_t TimeStamp)
{

	
	if (w_addr + size > bufsize1)
	{
		memcpy(b_buf + w_addr, buf, (bufsize1 - w_addr));
		memcpy(b_buf, buf + (bufsize1 - w_addr), size + w_addr - bufsize1);
	}
	else {
		memcpy(b_buf + w_addr, buf, size);
	}
	w_addr = (w_addr + size) % bufsize1;
	pthread_mutex_unlock(&l_ock);

}*/