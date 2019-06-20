#include "rtmp_client.h"
#include <string.h>
#include <stdio.h>
#define MYSELF_USE_LIBAAC_ENCODER
#define RTMP_PACKET_TYPE_CHUNK_SIZE         0x01
//constexpr auto RTMP_PACKET_TYPE_CHUNK_SIZE = 0x01;
#include"windows.h"
char * put_byte(char *output, uint8_t nVal)
{
	output[0] = nVal;
	return output + 1;
}
char * put_be16(char *output, uint16_t nVal)
{
	output[1] = nVal & 0xff;
	output[0] = nVal >> 8;
	return output + 2;
}
char * put_be24(char *output, uint32_t nVal)
{
	output[2] = nVal & 0xff;
	output[1] = nVal >> 8;
	output[0] = nVal >> 16;
	return output + 3;
}
char * put_be32(char *output, uint32_t nVal)
{
	output[3] = nVal & 0xff;
	output[2] = nVal >> 8;
	output[1] = nVal >> 16;
	output[0] = nVal >> 24;
	return output + 4;
}
char * put_be64(char *output, uint64_t nVal)
{
	output = put_be32(output, nVal >> 32);
	output = put_be32(output, nVal);
	return output;
}
char * put_amf_string(char *c, const char *str)
{
	uint16_t len = strlen(str);
	c = put_be16(c, len);
	memcpy(c, str, len);
	return c + len;
}
char * put_amf_double(char *c, double d)
{
	*c++ = AMF_NUMBER;  /* type: Number */
	{
		unsigned char *ci, *co;
		ci = (PBYTE)&d;
		co = (PBYTE)c;
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];
	}
	return c + 8;
}

CRtmpClient::CRtmpClient()
{
	m_pFileBuf = new unsigned char[FILEBUFSIZE];
	m_nFileBufSize = 0;
	m_nCurPos = 0;
	m_pRtmp = NULL;
	tps = 0;
	m_bHaveSendVideoMetaData = false;
	m_bHaveSendAudioInfo = false;
	memset(&m_metaData, 0, sizeof(m_metaData));
	m_dwAudioDelay = 0;
	m_nCacheAudioSampleCount = 0;
	m_bRtmpEnable1 = true;
	m_bIsEncoderEnable = true;
	Reset();
}

void CRtmpClient::Reset()
{
	m_bsps = false;
	m_bpps = false;
	m_bHaveSendVideoMetaData = false;
	m_bHaveSendAudioInfo = false;
	m_bConnected = false;
	// mwb 2014.12.08 add 支持从播出端控制AudioDelay
	memset(&m_aAudioPacketData[0], 0, sizeof(AUDIO_PACKET_DATA)*AUDIOPACKETARRAYSIZE);
	m_nBufferAudioSampleCount = 0;
	m_bSendAudioPacket = false;
	m_nWirteAudioPacketIndex = 0;
	m_nReadAudioPacketIndex = 0;
}

CRtmpClient::~CRtmpClient()
{
	FreeRtmp();
}

void CRtmpClient::FreeRtmp()
{
	if (m_pRtmp)
	{
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
	}
}

int CRtmpClient::Initialize()
{
	m_pRtmp = RTMP_Alloc();
	if (NULL == m_pRtmp)
		return -1;

	RTMP_Init(m_pRtmp);
	return 0;
}

bool CRtmpClient::Connect()
{
	//RTMP_SetupURL(m_pRtmp, "rtmp://1.8.250.111/live/stream85");
	int err = RTMP_SetupURL(m_pRtmp, m_strURL);
	if (err < 0)
		return false;

	RTMP_EnableWrite(m_pRtmp);

	err = RTMP_Connect(m_pRtmp, NULL);
	if (err <= 0)
		return false;
	else printf("Connect success\n");
	err = RTMP_ConnectStream(m_pRtmp, 0);
	if (err < 0)
		return false;
	else printf("ConnectStream success\n");
	m_bConnected = true;
	SetChunkSize();
	return true;
}


int CRtmpClient::Send(PBYTE pbuf, int nBufLen, int type, unsigned int dwTimeStamp,unsigned int nCTOffset)
{
	int nRet = 1;
	if (type == RTMP_PACKET_TYPE_VIDEO)
	{// Send Video
		
		NaluUnit nalu;
		nRet = ReadOneNaluFromBuf2(nalu, pbuf, nBufLen, dwTimeStamp, nCTOffset);
	}
	else
	{// Send Audio
			nRet = SendAACPacket(pbuf, nBufLen, dwTimeStamp);
		
	}
	return nRet;
}

int CRtmpClient::Close()
{
	if (m_bConnected)
	{
		RTMP_Close(m_pRtmp);
		m_bConnected = false;
	}
	return 0;
}

int CRtmpClient::ReadOneNaluFromBuf2(NaluUnit &nalu, BYTE *pByte, int nBufLen, unsigned int dwVideoTimeStamp, unsigned int dwCTOffset)
{

	int i = 0;
	int nRet = 1;
	while (i < nBufLen - 4)
	{
		if (pByte[i++] == 0x00 &&
			pByte[i++] == 0x00 &&
			pByte[i++] == 0x00 &&
			pByte[i++] == 0x01)
		{
			int pos = i;
			while (pos < nBufLen - 4)
			{
				if (pByte[pos] == 0x00 &&
					pByte[pos+1] == 0x00 &&
					pByte[pos+2] == 0x00 &&
					pByte[pos+3] == 0x01)
				{
					if (!m_bHaveSendVideoMetaData)
					{
						nalu.type = pByte[i] & 0x1f;
						nalu.size = pos - i;
						nalu.data = &pByte[i];
						FillMetaData(nalu);
						/*if (m_bsps && m_bpps)
						{
							nRet= SendMetaData(&m_metaData);
							
							m_bHaveSendVideoMetaData = true;
						}*/
					}
					pos += 4;
					i = pos;
				    pos++;
					continue;
				}
				else
				{
					if (pByte[pos] == 0x00 &&
						pByte[pos + 1] == 0x00 &&
						pByte[pos + 2] == 0x01 )
					{
						if (!m_bHaveSendVideoMetaData)
						{
							nalu.type = pByte[i] & 0x1f;
							nalu.size = pos - i;
							nalu.data = &pByte[i];
							FillMetaData(nalu);
							
								nRet = SendMetaData(&m_metaData);
								
								m_bHaveSendVideoMetaData = true;
						}
					pos += 3;
					nalu.type = pByte[pos] & 0x1f;
					nalu.size = nBufLen - pos;
					nalu.data = &pByte[pos];
					bool bKeyframe = (nalu.type == 0x05) ? TRUE : FALSE;
					// 发送H264数据帧
					SendH264Packet(nalu.data, nalu.size, bKeyframe, dwVideoTimeStamp, dwCTOffset);
					i = nBufLen;
					break;
			     }
			}
			pos++;
		}
	}
}
return nRet;
}





void CRtmpClient::FillMetaData(NaluUnit &nalu)
{
	
	if (nalu.type == 0x07)
	{// sps
		memset(m_metaData.Sps, 0, sizeof(m_metaData.Sps));
		m_metaData.nSpsLen = nalu.size;
		memcpy(m_metaData.Sps, nalu.data, nalu.size);
		m_bsps = true;

	}
	else if (nalu.type == 0x08)
	{// pps
		memset(m_metaData.Pps, 0, sizeof(m_metaData.Pps));
		m_metaData.nPpsLen = nalu.size;
		memcpy(m_metaData.Pps, nalu.data, nalu.size);
		m_bpps = true;
	}
}

int CRtmpClient::SendRTMPPacket(PBYTE pbuf, int nBufLen, int type, unsigned int timestamp)
{
	if (NULL == m_pRtmp)
		return 0;
	RTMPPacket RtmpPacket;
	RTMPPacket_Reset(&RtmpPacket);
	RTMPPacket_Alloc(&RtmpPacket, nBufLen);

	RtmpPacket.m_packetType = type;

	RtmpPacket.m_nChannel = 0x04;
	RtmpPacket.m_headerType = RTMP_PACKET_SIZE_LARGE;
	RtmpPacket.m_nInfoField2 = m_pRtmp->m_stream_id;

	RtmpPacket.m_nTimeStamp = timestamp;
	RtmpPacket.m_nBodySize = nBufLen;
	memcpy(RtmpPacket.m_body, pbuf, nBufLen);
	
	int nRet = RTMP_SendPacket(m_pRtmp, &RtmpPacket, 0);
	RTMPPacket_Free(&RtmpPacket);
	return nRet;
}

// Notify command
int CRtmpClient::SendMetaData(LPRTMPMetadata pMetaData)
{
	printf("send video head\n");


	if (pMetaData == NULL)
	{
		return false;
	}
	char body[2048] = { 0 };

	char * p = (char *)body;
	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "@setDataFrame");

	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "onMetaData");

	p = put_byte(p, AMF_OBJECT);
	p = put_amf_string(p, "copyright");
	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "ts");

	p = put_amf_string(p, "width");
	p = put_amf_double(p, m_nVideoWidth);

	p = put_amf_string(p, "height");
	p = put_amf_double(p, m_nVideoHeight);

	p = put_amf_string(p, "framerate");
	p = put_amf_double(p, m_nVideoFrameRate);

	p = put_amf_string(p, "videodatarate");
	p = put_amf_double(p, 2000);

	p = put_amf_string(p, "audiodatarate");
	p = put_amf_double(p, 0);

	p = put_amf_string(p, "videocodecid");
	p = put_amf_double(p, FLV_CODECID_H264);

	p = put_amf_string(p, "audiosamplerate");
	p = put_amf_double(p, 48000.0);

	p = put_amf_string(p, "audiosamplesize");
	p = put_amf_double(p, 16.0);

	p = put_amf_string(p, "audiocodecid");
	p = put_amf_double(p, 10);

	p = put_amf_string(p, "");
	p = put_byte(p, AMF_OBJECT_END);

	//  int index = p-body;

	SendRTMPPacket((BYTE*)body, p - body, RTMP_PACKET_TYPE_INFO, 0);

	int i = 0;
	body[i++] = 0x17; // 1:keyframe  7:AVC
	body[i++] = 0x00; // AVC sequence header

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00; // fill in 0;

					  // AVCDecoderConfigurationRecord.
	body[i++] = 0x01; // configurationVersion
	body[i++] = pMetaData->Sps[1]; // AVCProfileIndication
	body[i++] = pMetaData->Sps[2]; // profile_compatibility
	body[i++] = pMetaData->Sps[3]; // AVCLevelIndication
	body[i++] = 0xff; // lengthSizeMinusOne

					  // sps nums
	body[i++] = 0xE1; //&0x1f
					  // sps data length
	body[i++] = pMetaData->nSpsLen >> 8;
	body[i++] = pMetaData->nSpsLen & 0xff;
	// sps data
	memcpy(&body[i], pMetaData->Sps, pMetaData->nSpsLen);
	i = i + pMetaData->nSpsLen;

	// pps nums
	body[i++] = 0x01; //&0x1f
					  // pps data length
	body[i++] = pMetaData->nPpsLen >> 8;
	body[i++] = pMetaData->nPpsLen & 0xff;
	// sps data
	memcpy(&body[i], pMetaData->Pps, pMetaData->nPpsLen);
	i = i + pMetaData->nPpsLen;
	return SendRTMPPacket((BYTE*)body, i, RTMP_PACKET_TYPE_VIDEO, 0);
}

int CRtmpClient::SetChunkSize()
{
	RTMPPacket RtmpPacket;
	RTMPPacket_Reset(&RtmpPacket);
	RTMPPacket_Alloc(&RtmpPacket, 4);

	RtmpPacket.m_packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;

	RtmpPacket.m_nChannel = 0x02;
	RtmpPacket.m_headerType = RTMP_PACKET_SIZE_LARGE; //RTMP_PACKET_SIZE_MEDIUM;   //
	RtmpPacket.m_nInfoField2 = 0;

	RtmpPacket.m_nTimeStamp = 0;
	RtmpPacket.m_nBodySize = 4;

	int nVal = 1024;
	RtmpPacket.m_body[3] = nVal & 0xff;
	RtmpPacket.m_body[2] = nVal >> 8;
	RtmpPacket.m_body[1] = nVal >> 16;
	RtmpPacket.m_body[0] = nVal >> 24;
	m_pRtmp->m_outChunkSize = nVal;
	int nRet = RTMP_SendPacket(m_pRtmp, &RtmpPacket, 1);
	RTMPPacket_Free(&RtmpPacket);
	return nRet;
}

bool CRtmpClient::ReadOneNaluFromBuf(NaluUnit &nalu)
{
	int i = m_nCurPos;
	while (i<m_nFileBufSize - 4)
	{
		if (m_pFileBuf[i++] == 0x00 &&
			m_pFileBuf[i++] == 0x00 &&
			m_pFileBuf[i++] == 0x00 &&
			m_pFileBuf[i++] == 0x01
			)
		{
			int pos = i;
			while (pos<m_nFileBufSize - 4)
			{
				if (m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x01
					)
				{
					break;
				}
			}
			nalu.type = m_pFileBuf[i] & 0x1f;
			nalu.size = (pos - 4) - i;
			nalu.data = &m_pFileBuf[i];
			m_nCurPos = pos - 4;
			return TRUE;
		}
	}
	return FALSE;
}
bool CRtmpClient::SendH264Packet(unsigned char *data, unsigned int size, bool bIsKeyFrame, unsigned int nTimeStamp, unsigned int nCTOffset)
{
	if (data == NULL && size < 11)
	{
		return false;
	}

	unsigned char *body = new unsigned char[size + 9];

	int i = 0;
	if (bIsKeyFrame)
	{
		body[i++] = 0x17;// 1:Iframe  7:AVC  
		keyframepts = nTimeStamp;
	}
	else
	{
		body[i++] = 0x27;// 2:Pframe  7:AVC
		keyframepts = keyframepts + 40;
	}
	body[i++] = 0x01;// AVC NALU  
    /*body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;*/
	unsigned int ctt = nTimeStamp + 200 - keyframepts;
  if (bIsKeyFrame)
	{
	body[i++] = m_nVideoFrameDur >> 16;
	body[i++] = m_nVideoFrameDur >> 8;
	body[i++] = m_nVideoFrameDur & 0xff;
}
	else
	{
		body[i++] = ctt >> 16;
		body[i++] = ctt >> 8;
		body[i++] = ctt & 0xff;
	}
	// NALU size 
	body[i++] = size >> 24;
	body[i++] = size >> 16;
	body[i++] = size >> 8;
	body[i++] = size & 0xff;

	// NALU data  
	memcpy(&body[i], data, size);

	bool bRet = SendRTMPPacket( body, i + size, RTMP_PACKET_TYPE_VIDEO, keyframepts);

	delete[] body;

	return bRet;
}


int CRtmpClient::SendAACPacket(PBYTE pBuf, unsigned int size, ULONG dwAudioTimeStamp)
{
	if (!m_bHaveSendAudioInfo)
	{
		unsigned char header[4];
		header[0] = 0xaf;
		header[1] = 0x00;
		header[2] = 0x11;
		header[3] = 0x90;
		SendRTMPPacket(header, 4, RTMP_PACKET_TYPE_AUDIO, 0);
		printf("send audio head\n");
		m_bHaveSendAudioInfo = true;
	}

    unsigned char *body = new unsigned char[size +2]; // 5 = 7-2 :7 AAC Header 2: rtmp audio tag
	body[0] = 0xaf;
	body[1] = 0x01;
	memcpy(&body[2], &pBuf[0], size);  // 拷贝AAC Raw Data
	int nRet = SendRTMPPacket(body, size +2, RTMP_PACKET_TYPE_AUDIO, dwAudioTimeStamp);
	
	/*unsigned char *body = new unsigned char[size - 7]; // 7 = 9-2 :9 AAC Header + 2 byte CRC 2: rtmp audio tag
	body[0] = 0xaf;
	body[1] = 0x01;
	memcpy(&body[2], &pBuf[9], size - 9);
	int nRet = SendRTMPPacket(body, size - 7, RTMP_PACKET_TYPE_AUDIO, dwAudioTimeStamp/90);*/


	delete[] body;
	return nRet;
}

void CRtmpClient::SetVideoParam(int nVideoWidth, int nVideoHeight, int nVideoRate)
{
	if (nVideoWidth == 1920)
		nVideoHeight = 1080;
	else if (nVideoWidth == 720)
		nVideoHeight = 576;
	else if (nVideoWidth == 704)
		nVideoHeight = 576;
	m_nVideoWidth = nVideoWidth;
	m_nVideoHeight = nVideoHeight;
	m_nVideoFrameRate = 1000 / nVideoRate;
	if (m_nVideoFrameRate >= 50)
		m_nVideoFrameRate /= 2;
	// 1000： 1000毫秒
	m_nVideoFrameDur = 1000 / m_nVideoFrameRate;
	m_metaData.nHeigth = nVideoHeight;
	m_metaData.nWidth = nVideoWidth;
	m_metaData.nFrameRate = 25;
}

void CRtmpClient::SetAudioParam(int nFreq)
{
	// 1000:将秒转化为毫秒
	m_nAudioFrameDur = (1024 * 1000) / nFreq;
}

void CRtmpClient::VideoRateChanged(int nVideoRate)
{
	m_nVideoFrameRate = 1000 / nVideoRate;
	if (m_nVideoFrameRate >= 50)
		m_nVideoFrameRate /= 2;
	m_nVideoFrameDur = 1000 / m_nVideoFrameRate;
	//	SendMetaData(&m_metaData);
}

void CRtmpClient::SetAudioDelay(unsigned int dwAudioDelay)
{
	m_dwAudioDelay = dwAudioDelay;

	// mwb 2014.12.08 add
	// 21:代表AAC 每帧持续时间为21.33ms
	m_nCacheAudioSampleCount = m_dwAudioDelay / 21;
	// 减去2，主要是在测试时发现若按照m_nCacheAudioSampleCount去延迟，延迟有些多。
	m_nCacheAudioSampleCount -= 2;
	if (m_nCacheAudioSampleCount < 0)
		m_nCacheAudioSampleCount = 0;
	if (m_nCacheAudioSampleCount > AUDIOPACKETARRAYSIZE)
		m_nCacheAudioSampleCount = AUDIOPACKETARRAYSIZE;
}

void CRtmpClient::SetURL(const char *strURL)
{
	
	sprintf(m_strURL, "rtmp://%s", strURL);
}
