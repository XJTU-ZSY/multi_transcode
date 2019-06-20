#include"outpool.h"
#define MSDK_SAFE_DELETE(P)                      {if (P) {delete P; P = NULL;}}
#include"windows.h"

outpool::outpool(int weith)
{   
	if(weith==720 || weith==704)
	m_IBufferSize = 1024*300;
	else if(weith == 1920)
		m_IBufferSize = 1024 * 1024;
	for (int i = 0; i < 16; i++)
	{
		m_pabyBuffer[i] = new BYTE[m_IBufferSize];
		memset(m_pabyBuffer[i], 0, m_IBufferSize);
	}
	for (int i = 0; i < 1000; i++)
	{
		m_pabyBuffer_A[i] = new BYTE[1024];
		memset(m_pabyBuffer_A[i], 0, 1024);
	}
	m_Read_A = 0;
	m_Wead_A = 0;
	m_Read = 0;
	m_Wead = 0;
	Samplecount = 0;
	contex = 0;
	pthread_mutex_init(&m_mutex, NULL);
	pthread_mutex_init(&m_mutex1, NULL);
	pthread_mutex_init(&lockcontex, NULL);
}
outpool::~outpool(void)
{
	for (int i = 0; i < 16; i++)
	{
		if (m_pabyBuffer[i])
		{
			delete m_pabyBuffer[i];
			m_pabyBuffer[i] = NULL;
		}
	}
	for (int i = 0; i < 1000; i++)
	{
		if (m_pabyBuffer_A[i])
		{
			delete m_pabyBuffer_A[i];
			m_pabyBuffer_A[i] = NULL;
		}
	}
	pthread_mutex_destroy(&m_mutex);
	pthread_mutex_destroy(&m_mutex1);
}

void outpool::writecontex(int i)
{
	pthread_mutex_lock(&lockcontex);
	contex = i;
	pthread_mutex_unlock(&lockcontex);
}
bool outpool::readcontx()
{
	pthread_mutex_lock(&lockcontex);
	if (contex == 1)
	{
		pthread_mutex_unlock(&lockcontex);
		return true;
	}
	else {
		pthread_mutex_unlock(&lockcontex);
		return false; }
}
bool outpool::Get(uint8_t *Vsample, mfxU64 *ITimeStamp, mfxU32 *ISampleLength)
{
	pthread_mutex_lock(&m_mutex);
	unsigned long IWear = m_Wead;
	if (m_Read == IWear)
	{
		pthread_mutex_unlock(&m_mutex);
		return false;
	}

	memcpy(ISampleLength, m_pabyBuffer[m_Read], 4);
	if(*ISampleLength>=m_IBufferSize)
	{
		m_Read++;
		if (m_Read == 16)
			m_Read = 0;
		printf("rtmp read size is larger than buf size\n");
		pthread_mutex_unlock(&m_mutex);
		return false;
	}
	memcpy(ITimeStamp, m_pabyBuffer[m_Read] + 4, 8);
	memcpy(Vsample, m_pabyBuffer[m_Read] + 12, *ISampleLength);
	m_Read++;
	if (m_Read == 16)
		m_Read = 0;
	Samplecount--;
	pthread_mutex_unlock(&m_mutex);
	return true;
}
bool outpool::Get(uint8_t *Vsample, mfxU32 *ISampleLength, mfxU64 *ITimeStamp)
{
	pthread_mutex_lock(&m_mutex1);
	unsigned long IWear = m_Wead_A;
	if (m_Read_A == IWear)
	{
		
		pthread_mutex_unlock(&m_mutex1);
		return false;
	}
	
	memcpy(ISampleLength, m_pabyBuffer_A[m_Read_A], 4);
	if (*ISampleLength>=1024)
	{
		m_Read_A++;
		if (m_Read_A == 1000)
			m_Read_A = 0;
		pthread_mutex_unlock(&m_mutex1);
		return false;
	}
	memcpy(ITimeStamp, m_pabyBuffer_A[m_Read_A] + 4, 8);
	memcpy(Vsample, m_pabyBuffer_A[m_Read_A] + 12, *ISampleLength);
	m_Read_A++;
	if (m_Read_A == 1000)
		m_Read_A = 0;
	pthread_mutex_unlock(&m_mutex1);
	return true;
}
void outpool::Write(AVPacket* audio_pkt)
{
	pthread_mutex_lock(&m_mutex1);

	memcpy(m_pabyBuffer_A[m_Wead_A], &(audio_pkt->size), 4);
	if (audio_pkt->size > 1024)
	{	pthread_mutex_unlock(&m_mutex1);
	return ;
}
	memcpy(m_pabyBuffer_A[m_Wead_A] + 4, &(audio_pkt->pts), 8);
	memcpy(m_pabyBuffer_A[m_Wead_A]+12, audio_pkt->data, audio_pkt->size);
	m_Wead_A++;
	if (m_Wead_A == 1000)
		m_Wead_A = 0;
	pthread_mutex_unlock(&m_mutex1);
}
bool outpool::Write(mfxBitstream *pMfxBitstream)
{

	pthread_mutex_lock(&m_mutex);
	unsigned char *a;
	a = (unsigned char*)av_mallocz(sizeof(uint8_t) * 12);
	memcpy(&a[0], &pMfxBitstream->DataLength, 4);
	memcpy(&a[4], &pMfxBitstream->TimeStamp, 12);
	
	if (pMfxBitstream->DataLength >= m_IBufferSize)
	{
		printf("data size more than outpool size\n");
		pthread_mutex_unlock(&m_mutex);
		av_free(a);
		return false;

	}

	memcpy(m_pabyBuffer[m_Wead], a, 12);
	memcpy(m_pabyBuffer[m_Wead] + 12, pMfxBitstream->Data + pMfxBitstream->DataOffset, pMfxBitstream->DataLength);
	m_Wead++;
	if (m_Wead == 16)
		m_Wead = 0;
	Samplecount++;
	pthread_mutex_unlock(&m_mutex);
	av_free(a);
	return true;
}
int outpool::GetSamplecount()
{
	return Samplecount;
}

