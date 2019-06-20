/******************************************************************************\
Copyright (c) 2005-2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
\**********************************************************************************/
#pragma comment(lib,"ws2_32.lib")
#define BUF_SIZE 1024
#include "mfx_samples_config.h"
#include"stdio.h"
#include"stdlib.h"
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "sample_multi_transcode.h"
int sendnum =40;//路数
#if defined(LIBVA_WAYLAND_SUPPORT)
#include "class_wayland.h"
#endif
pthread_mutex_t lockepg = PTHREAD_MUTEX_INITIALIZER;
//FILE *fp1 = fopen("test.ts", "wb");
struct portinfo
{ 
	int port_id;
	unsigned char num_id;
}*port;

char *program_id[] = { "CCTV1","CCTV2","CCTV5","CCTV6","CCTV10","CCTV8","CCTV7","CCTV3","CCTV4",
"CCTV9","CCTV11","CCTV12","CCTV13","CCTV15","CCTV16","CCTV5-PLUS","AHTV1","BTV1","CCQTV1","FJTV2",
"GDTV1","SZTV1","NANFANG2","GUANXI1","GUIZOUTV1","TCTC1","HEBEI1","HLJTV1","HNTV1","HUBEI1",
"HUNANTV1","JSTV1","JXTV1","JILIN1","LNTV1","NMGTV1","NXTV2","SXTV1","SDTV1","DONGFANG1","SHXITV1",
"SCTV1","TJTV1","XJTV1","YNTV1","ZJTV1","QHTV1","SANSHATV","PHOENIX2","PHOENIXHK","PHOENIX1"};
char *program_name[100];
//FILE *fp1 = fopen("ts1.ts", "wb");
int InitSockets()
{

	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(2, 2);
	return (WSAStartup(version, &wsaData) == 0);

}


//void *RunSendRtmp(void* param)
//{
//	int i = *(int*)param;
//	mfxU64 *ITimeStamp;
//	ITimeStamp = (mfxU64*)av_mallocz(sizeof(mfxU64));
//	mfxU32 *ISampleLength;
//	ISampleLength = (mfxU32*)av_mallocz(sizeof(mfxU32));
//	uint8_t *pAudioSample;
//	pAudioSample = (uint8_t*)av_mallocz(sizeof(uint8_t) * 1024 * 1024 + 10);
//	ULONG lVideoMaxTime = 0;
//	ULONG lVideoMinTime = -1;
//	bool bFirstSendRtmp = true;
//	int indexnum = 0;
//
//	while (1)
//	{
//		if (sendbuf[i]->Get(pAudioSample, ISampleLength, ITimeStamp))
//		{
//
//			if (m_pRtmpClient1[i]->m_bRtmpEnable1 && m_pRtmpClient1[i])
//			{
//				*ITimeStamp = *ITimeStamp / 90;
//
//				if (1 != m_pRtmpClient1[i]->Send(pAudioSample, (int)(*ISampleLength), RTMP_PACKET_TYPE_AUDIO, (DWORD)(*ITimeStamp + 200)))
//				{
//
//					printf("send frame error\n");
//					m_pRtmpClient1[i]->Close();
//					m_pRtmpClient1[i]->Close();
//					m_pRtmpClient1[i]->FreeRtmp();
//					InitRtmpClient1(i);
//				}
//			}
//
//			if (sendbuf[i]->Get(pAudioSample, ITimeStamp, ISampleLength))
//			{
//				indexnum = 0;
//				ULONG lCurTimeStamp = *ITimeStamp / 90;
//				//fwrite(pAudioSample, *ISampleLength, 1, fp);
//				if (m_pRtmpClient1[i]->m_bRtmpEnable1 && m_pRtmpClient1[i])
//				{
//					//*ITimeStamp = *ITimeStamp / 90;
//					if (bFirstSendRtmp)
//					{
//						bFirstSendRtmp = false;
//
//						m_pRtmpClient1[i]->SetVideoParam(decodebuf[i]->weith, 0, 25);
//					}
//					//printf("lVideoMaxTime=%d\n", (DWORD)lVideoMaxTime);
//					if (1 != m_pRtmpClient1[i]->Send(pAudioSample, (int)(*ISampleLength), RTMP_PACKET_TYPE_VIDEO, (DWORD)lCurTimeStamp, 0))
//					{
//						printf("send frame error\n");
//						m_pRtmpClient1[i]->Close();
//						m_pRtmpClient1[i]->FreeRtmp();
//						InitRtmpClient1(i);
//					}
//				}
//			}
//			else {
//					indexnum++;
//					if (indexnum >= 3000)
//					{
//						printf("number %d road cannot read data from encode\n", i);
//						indexnum = 0;
//						sendbuf[i]->writecontex (1);
//					}
//				}
//		
//
//		}
//		else
//		{
//			Sleep(5);
//		}
//
//	}
//	av_free(ISampleLength);
//	av_free(ITimeStamp);
//	av_free(pAudioSample);
//	return NULL;
//}

void *RunSendRtmp(void* param)
{
	int i = *(int*)param;
	mfxU64 *ITimeStamp;
	ITimeStamp = (mfxU64*)av_mallocz(sizeof(mfxU64));
	mfxU32 *ISampleLength;
	ISampleLength = (mfxU32*)av_mallocz(sizeof(mfxU32));
	uint8_t *pAudioSample;
	pAudioSample = (uint8_t*)av_mallocz(sizeof(uint8_t) * 1024 * 1024 + 10);
	ULONG lVideoMaxTime = 0;
	ULONG lVideoMinTime = -1;
	bool bFirstSendRtmp = true;
	int indexnum = 0;
	mfxU32 lenth = 0;
	while (1)
	{
		
		if (sendbuf[i]->Get(pAudioSample, ISampleLength, ITimeStamp))
		{
			lenth = *ISampleLength + 7;
			unsigned char *p = new unsigned char[lenth];
			p[0] = 0xFF;
			p[1] = 0xF1;
			p[2] = 0x4C;
			p[3] = (lenth >> 11) | 0x80;
			p[4] = (lenth >> 3) & 0xFF;
			p[5] = (lenth << 5) & 0xFF;
			p[6] = 0xFC;
			//*ITimeStamp = *ITimeStamp + 7200;
			memcpy(&p[7], pAudioSample, *ISampleLength);
			tsencode[i]->writeaudio(p, *ITimeStamp, lenth);
			//tsencode[i]->ts_write(p, lenth, 0, *ITimeStamp, 1);
			delete[] p;
			if (sendbuf[i]->Get(pAudioSample, ITimeStamp, ISampleLength))
			{
				indexnum = 0;
				tsencode[i]->readaudio(*ITimeStamp,i);
				tsencode[i]->ts_write(pAudioSample, *ISampleLength, 0, *ITimeStamp, 0,i);
			}
			else {
				Sleep(1);
				indexnum++;
				if (indexnum >= 3000)
				{
					printf("number %d road cannot read data from encode\n", i);
					indexnum = 0;
					sendbuf[i]->writecontex(1);
				}
			}
		}
		else
			Sleep(1);
		

	}
	av_free(ISampleLength);
	av_free(ITimeStamp);
	av_free(pAudioSample);
	return NULL;
}
size_t write_data(void * ptr, size_t size, size_t nmemb, void * stream)
{
	int a = 0;
	memcpy(&a, stream, 4);
	memcpy((unsigned char*)stream + a+4, ptr, size * nmemb);
	a += size * nmemb;
	memcpy(stream, &a, 4);

//	fwrite(ptr, size, nmemb, (FILE*)stream);
	return size * nmemb;
}

void dateadd_one(unsigned char *date)
{
	if (!date)
		return;
	date[1]++;
	if (date[1] <= 30)
		return;
	else if (date[1] >= 32)
	{
		date[0] = (date[0] + 1) % 12;
		date[1] = 1;
	}
	else if (date[0] == 2 || date[0] == 4 || date[0] == 6 || date[0] == 9 || date[0] == 11)
	{
		date[0] = (date[0] + 1) % 12;
		date[1] = 1;
	}
}
void *inforecv(void* Param)
{
	int number = *(int*)Param;
	
	char idtxtname[255];
	sprintf(idtxtname, "inumber%d.txt", number);

	FILE *fnumber =fopen(idtxtname, "rb");
	char idnumber[4];

	if (fnumber)
	{
		fread(idnumber, 1, 4, fnumber);
		fclose(fnumber);
	}
	/*sprintf(idtxtname, "del inumber%d.txt", number);
	system(idtxtname);*/
	int firstid = 0;
	memcpy(&firstid, idnumber, 4);
	int lastid = 0;
	bool firstread = true;
	epginfo fuinfo[14];
	//for (int i = 0; i < 14; i++)
	//	fuinfo[i].index = 0;
	while (1)
	{
		if (number == 0)
			number = 0;
		else if (number == 1)
			number = 1;
		else if (number == 2)
			number = 2;
		else if (number == 3)
			number = 3;
		else if (number == 4)
			number = 4;
		else if (number == 5)
			number = 5;
		else if (number == 6)
			number = 6;
		else if (number == 7)
			number = 7;
		else if (number == 8)
			number = 8;
		else if (number == 9)
			number = 9;
		else if (number == 10)
			number = 10;
		else if (number == 11)
			number = 11;
		else if (number == 12)
			number = 12;
		else if (number == 13)
			number = 13;
		else if (number == 14)
			number = 14;
		else if (number == 15)
			number = 15;
		//CURL *curl;             //定义CURL类型的指针
		//CURLcode res;           //定义CURLcode类型的变量，保存返回状态码

		

		int lengthg = 0;
		char s[255];
		char c1[400];
		pthread_mutex_lock(&lockepg);
		sprintf(c1, "curl -o text%d.txt", number);
		c1[strlen(c1) + 1] = '\0';
		c1[strlen(c1)] = ' ';
		sprintf(s+1, "http://1.8.203.196:8080/EPG/schedule?secret=VYDcCe1s&id=%s", program_id[number]);
		//EPG信息网址
		//sprintf(s + 1, "http://api.deepepg.com/api/schedule?secret=VYDcCe1s&id=%s", program_id[number]);	//	固定为CCTV2
		s[0] = '""';
		s[strlen(s) + 1] = '\0';
		s[strlen(s)] = '""';
		
		strcat( c1,s);
		
		system(c1);
		memset(s, 0, 255);
		sprintf(s, "text%d.txt", number);
		FILE *fp123 = fopen(s, "rb");
		fseek(fp123, 0, 2);
		lengthg = ftell(fp123);
		fseek(fp123, 0, 0);
		unsigned char *p = new unsigned char[lengthg];
		fread(p, 1, lengthg, fp123);
		fclose(fp123);
		pthread_mutex_unlock(&lockepg);
		/*curl = curl_easy_init();        //初始化一个CURL类型的指针
		if (curl != NULL)
		{
			char s[255];
			if (number< 50)
			{sprintf(s, "http://1.8.203.198:8080/EPG/schedule?secret=VYDcCe1s&id=%s", program_id[number]);
			}
				//sprintf(s, "http://111.20.115.242:8080/EPG/schedule?secret=VYDcCe1s&id=%s", program_id[port[number].num_id - 1]);

			
			else
			{
				port[number].num_id -= 50;
				sprintf(s, "http://1.8.203.198:8080/EPG/schedule?secret=VYDcCe1s&id=%s", program_id[number]);
			}
			curl_easy_setopt(curl, CURLOPT_URL, s);

			
			if (number == 0)
				number = 0;
			else if (number == 1)
				number = 1;
			else if (number == 2)
				number = 2;
			else if (number == 3)
				number = 3;
			else if (number == 4)
				number = 4;
			else if (number == 5)
				number = 5;
			else if (number == 6)
				number = 6;
			else if (number == 7)
				number = 7;
			else if (number == 8)
				number = 8;
			else if (number == 9)
				number = 9;
			else if (number == 10)
				number = 10;
			else if (number == 11)
				number = 11;
			else if (number == 12)
				number = 12;
			else if (number == 13)
				number = 13;
			else if (number == 14)
				number = 14;
			else if (number == 15)
				number = 15; 
			
			memset(p, 0, sizeof(p));
			res=curl_easy_setopt(curl, CURLOPT_WRITEDATA, p);
			res=curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			
			pthread_mutex_lock(&lockepg);
			res = curl_easy_perform(curl);
			pthread_mutex_unlock(&lockepg);
			memcpy(&lengthg, p, 4);
			if (lengthg >= 1024 * 1024)
				lengthg = 1024 * 1024;
			curl_easy_cleanup(curl);
		}*/

		time_t t = time(0);
		char tmp[64];
		struct tm *cc = localtime(&t);
		cc->tm_year = cc->tm_year + 1900;

		cc->tm_mon += 1;

		int i = 4;

		
		uint8_t a = 0;
		uint8_t b = 0;
		uint8_t c = 0;
		uint8_t d = 0;
		uint8_t e = 0;
		uint8_t f = 0;
		while (i < lengthg)
		{
			if (number == 0)
				number = 0;
			else if (number == 1)
				number = 1;
			else if (number == 2)
				number = 2;
			else if (number == 3)
				number = 3;
			else if (number == 4)
				number = 4;
			else if (number == 5)
				number = 5;
			else if (number == 6)
				number = 6;
			else if (number == 7)
				number = 7;
			else if (number == 8)
				number = 8;
			else if (number == 9)
				number = 9;
			else if (number == 10)
				number = 10;
			else if (number == 11)
				number = 11;
			else if (number == 12)
				number = 12;
			else if (number == 13)
				number = 13;
			else if (number == 14)
				number = 14;
			else if (number == 15)
				number = 15;
			if (p[i++] == 'd' && p[i++] == 'a' && p[i++] == 't' && p[i++] == 'e' && p[i++] == '=')
			{
				i++;
				int year = 0;
				for (int numbe = 0; numbe < 4; numbe++)
				{
					year = year * 10 + (p[i++] - '0');
				}
				i++;
				int mon_ = 0;
				mon_ = p[i++] - '0';
				mon_ = mon_ * 10 + (p[i++] - '0');
				i++;
				int day_ = 0;
				day_ = p[i++] - '0';
				day_ = day_ * 10 + (p[i++] - '0');
				if (year == cc->tm_year && mon_ == cc->tm_mon && day_ == cc->tm_mday)
				{
					while (i < lengthg)
					{
						if (number == 0)
							number = 0;
						else if (number == 1)
							number = 1;
						else if (number == 2)
							number = 2;
						else if (number == 3)
							number = 3;
						else if (number == 4)
							number = 4;
						else if (number == 5)
							number = 5;
						else if (number == 6)
							number = 6;
						else if (number == 7)
							number = 7;
						else if (number == 8)
							number = 8;
						else if (number == 9)
							number = 9;
						else if (number == 10)
							number = 10;
						else if (number == 11)
							number = 11;
						else if (number == 12)
							number = 12;
						else if (number == 13)
							number = 13;
						else if (number == 14)
							number = 14;
						else if (number == 15)
							number = 15;
						if (p[i++] == 's' && p[i++] == 't' && p[i++] == 'a' && p[i++] == 'r' && p[i++] == 't' && p[i++] == '_' && p[i++] == 't' && p[i++] == 'i' && p[i++] == 'm' && p[i++] == 'e')
						{

							i++;
							a = p[i++] - '0';
							a = p[i++] -'0'+a*10;
							
							i++;
							b = p[i++] - '0';
							b = p[i++] - '0'+b*10;
							
							e = cc->tm_hour;
							f = cc->tm_min;
							i++;
							if (e * 60 + f < a * 60 + b)
							{
								i = lengthg;
								break;
							}
							else
							{
								while (i < lengthg)
								{
									if (p[i++] == 'e' && p[i++] == 'n' && p[i++] == 'd' && p[i++] == '_' && p[i++] == 't' && p[i++] == 'i' && p[i++] == 'm' && p[i++] == 'e')
									{
										i++;
										c = p[i++] - '0';
										c = p[i++] -'0'+c*10;
										
										i++;
										d = p[i++] - '0';
										d = p[i++] -'0'+d*10;
										
										i++;
										if (c * 60 + d < a * 60 + b || (c * 60 + d > a * 60 + b && e * 60 + f < c * 60 + d))
										{
											int j = 0;
											while (i < lengthg)
											{
												if (p[i++] == 'C' && p[i++] == 'D' && p[i++] == 'A' && p[i++] == 'T' && p[i++] == 'A')
												{
													i++;
													while (p[i] != ']')
													{
														i++;
														j++;
													}
													break;
												}
											}
											if (firstread)
											{
												fuinfo[0].idnum = firstid;
												firstread = false;
											}
											else
											{
												
												if (abs(fuinfo[1].idnum - fuinfo[0].idnum) > 100)
													fuinfo[0].idnum = fuinfo[0].idnum;
												int insteadi = i;
												while (insteadi < lengthg)
												{

													if (p[insteadi++] == 's' && p[insteadi++] == 't' && p[insteadi++] == 'a' && p[insteadi++] == 'r' && p[insteadi++] == 't' && p[insteadi++] == '_' && p[insteadi++] == 't' && p[insteadi++] == 'i' && p[insteadi++] == 'm' && p[insteadi++] == 'e')
													{

														BYTE insteadtime[2];
														insteadi++;
														insteadtime[0] = (p[insteadi++] - '0');
														insteadtime[0] = insteadtime[0] * 10 + (p[insteadi++] - '0');
														insteadi++;
														insteadtime[1] = (p[insteadi++] - '0');
														insteadtime[1] = insteadtime[1] * 10 + (p[insteadi++] - '0');
														if (insteadtime[0] * 60 + insteadtime[1] > fuinfo[1].start_time[0] * 60 + fuinfo[1].start_time[1])
														{
															fuinfo[1].start_time[0] = insteadtime[0];
															fuinfo[1].start_time[1] = insteadtime[1];
														}
														break;
													}
												}
												if (fuinfo[0].start_time[0] * 60 + fuinfo[0].start_time[1] >= a * 60 + b && fuinfo[0].data[1] == day_)
												{
													i = lengthg;
													break;
												}
												else
												{
													if (number == 15)
														number = 15;
													if (abs(fuinfo[0].start_time[0] * 60 + fuinfo[0].start_time[1] - a * 60 - b) < 10)
													{
														unsigned char nowname[256];
														if (j < 256)
															memcpy(nowname, &p[i - j], j);
														int ss = j > fuinfo[0].size ? fuinfo[0].size : j;
														int nn = 0;
														for (; nn < ss; nn++)
														{
															if (nowname[nn] != fuinfo[0].name[nn])
																break;
														}
														if (nn == ss)
														{
															fuinfo[0].start_time[0] = a;
															fuinfo[0].start_time[1] = b;
															i = lengthg;
															break;
														}
													}
												}
												fuinfo[0].idnum++;
												printf("第%d路更新為%d\n", number, fuinfo[0].idnum);
												if (number == 15)
													number = 15;
												sprintf(idtxtname, "inumber%d.txt", number);
												//printf("%s\n", fuinfo[0].name);
												FILE *fidnumber = fopen(idtxtname, "wb");
												fwrite(&fuinfo[0].idnum, 4, 1, fidnumber);
												fclose(fidnumber);
											}
											if (number == 15)
												number = 15;
											
											//printf("%s\n", fuinfo[0].name);
											fuinfo[0].data[0] = cc->tm_mon;
											fuinfo[0].data[1] = cc->tm_mday;
											fuinfo[0].start_time[0] = a;
											fuinfo[0].start_time[1] = b;
											fuinfo[0].pdname_size = strlen(program_name[number]);
											memcpy(fuinfo[0].pdname, program_name[number], fuinfo[0].pdname_size);
											fuinfo[0].size = j;
											memcpy(fuinfo[0].name, &p[i - j], j);
											unsigned char Pframedata[2];
											Pframedata[0] = cc->tm_mon;
											Pframedata[1] = cc->tm_mday;
											for (int inti = 1; inti < 14; inti++)
											{

												while (i < lengthg)
												{
													
													if (p[i++] == 's' && p[i++] == 't' && p[i++] == 'a' && p[i++] == 'r' && p[i++] == 't' && p[i++] == '_' && p[i++] == 't' && p[i++] == 'i' && p[i++] == 'm' && p[i++] == 'e')
													{
			
														
														i++;
														fuinfo[inti].start_time[0] = (p[i++] - '0');
														fuinfo[inti].start_time[0] = fuinfo[inti].start_time[0] * 10 + (p[i++] - '0');
														i++;
														fuinfo[inti].start_time[1] = (p[i++] - '0');
														fuinfo[inti].start_time[1] = fuinfo[inti].start_time[1] * 10 + (p[i++] - '0');
														/*if (inti != 14)
														{
															if (fuinfo[inti].start_time[0] == fuinfo[inti + 1].start_time[0] && fuinfo[inti].start_time[1] == fuinfo[inti + 1].start_time[1])
															{
																fuinfo[inti].index = fuinfo[inti+1].index;
															}
															else 
																fuinfo[inti].index= fuinfo[inti + 1].index+1;
														}
														else
															fuinfo[inti].index=0;*/
														if (fuinfo[inti].start_time[0] * 60 + fuinfo[inti].start_time[1] < fuinfo[inti - 1].start_time[0] * 60 + fuinfo[inti - 1].start_time[1])
														{
															dateadd_one(Pframedata);
															/*int j = i - 46;
															Pframedata[0] = p[j++] - '0';
															Pframedata[0] = Pframedata[0] * 10 + p[j++] - '0';
															j++;
															Pframedata[1] = p[j++] - '0';
															Pframedata[1] = Pframedata[1] * 10 + p[j++] - '0';*/
														}
														while (i < lengthg)
														{
															if (p[i++] == 'C' && p[i++] == 'D' && p[i++] == 'A' && p[i++] == 'T' && p[i++] == 'A')
															{
																i++;
																int intj = 0;
																while (p[i] != ']')
																{
																	i++;
																	intj++;
																}
																if (number == 15)
																	number = 15;
																fuinfo[inti].data[0] = Pframedata[0];
																fuinfo[inti].data[1] = Pframedata[1];
																fuinfo[inti].idnum = fuinfo[inti - 1].idnum + 1;
																fuinfo[inti].size = intj;
																memcpy(fuinfo[inti].name, &p[i - intj], intj);

																break;
															}
														}
														break;
													}
													
												}
											}


											
											tsencode[number]->writeinfo(fuinfo);
											i = lengthg;
											break;

										}
										else
											break;
									}
								}
							}
						}
					}
				}
			}
		}
		Sleep(300000);
		delete[]p;
		p = NULL;
	}
		
	while (1)
	{
		printf("%d info out\n", number);
		Sleep(10);
	}
	return NULL;
}
int  numrecv()
{
	unsigned int server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(6666);
	memset(server_addr.sin_zero, 0, 8);
	char re_flag = 1;
	int re_len = sizeof(int);
	setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &re_flag, re_len);

	bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));


	struct sockaddr_in client_addr;
	int client_addr_length = sizeof(client_addr);
	uint8_t buffer[BUF_SIZE];
	while (1)
	{

		memset(buffer, 0, BUF_SIZE);
		struct timeval tv;
		fd_set readfds;
		tv.tv_sec = 3;
		tv.tv_usec = 10;
		FD_ZERO(&readfds);
		FD_SET(server_socket_fd, &readfds);
		select(server_socket_fd + 1, &readfds, NULL, NULL, &tv);
		int num = 0;
		if (FD_ISSET(server_socket_fd, &readfds))
		{
			int len = recvfrom(server_socket_fd, (char*)buffer, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_length);
			//	printf("recv length = %d\nport=%d\n", len, port);
			if (len <= 0)
				printf("%d received data error\n");

			else if (len > 0)
			{
				int j = 0;
				while (j < len)
				{
					if (buffer[j++] == 0x50)
					{
						if (buffer[j++] == 2)
						{
							num = buffer[j++];
							port = (struct portinfo*)malloc(sizeof(struct portinfo)*num);
							for (int cc = 0; cc < num; cc++)
							{
								port[cc].num_id = buffer[j++];
								
								j += 12;
								port[cc].port_id = buffer[j++]*256;
								port[cc].port_id = port[cc].port_id + buffer[j++];
								
							}
							
							
							return num;
						}
					}
					j++;
				}
			}

		}

	}

	//WSACleanup();
}
void *udp_tsrecv(void* Param)
{
	int portin = *(int*)Param;
	/*WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) //调用Windows Sockets DLL
	{
	printf("Winsock无法初始化!\n");
	WSACleanup();
	}*/
	

	unsigned int server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//接收端口
	server_addr.sin_port = htons(10000+ portin);
	//server_addr.sin_port = htons(port[portin].port_id);

	memset(server_addr.sin_zero, 0, 8);
	
	/*int nRecvBuf = 128*1024;
	setsockopt(server_socket_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));*/

	bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));


	struct sockaddr_in client_addr;
	int client_addr_length = sizeof(client_addr);
	uint8_t buffer[BUF_SIZE];
	while (1)
	{

		memset(buffer, 0, BUF_SIZE);
		struct timeval tv;
		fd_set readfds;
		tv.tv_sec = 3;
		tv.tv_usec = 10;
		FD_ZERO(&readfds);
		FD_SET(server_socket_fd, &readfds);
		select(server_socket_fd + 1, &readfds, NULL, NULL, &tv);
		if (FD_ISSET(server_socket_fd, &readfds))
		{
			int len = recvfrom(server_socket_fd, (char*)buffer, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_length);
			//	printf("recv length = %d\nport=%d\n", len, port);
			if (len <= 0)
				printf("%d received data error\n", portin);

			else if (len > 0)
			{
				/*if(portin==0)
				fwrite(buffer, len, 1, fp);*/
				
				recvpool[portin]->put_queue(buffer, len);
			}

		}

	}

	WSACleanup();

}
using namespace std;
using namespace TranscodingSample;

void *transcodess_start(void* param)
{
	int i=*(int *)param;
	while (1)
	{
		mfxStatus sts;
		Launcher transcode;
		sts = transcode.Init(i);
		fflush(stdout);
		fflush(stderr);


		transcode.Run(i);
		sendbuf[i]->writecontex(0);
		sts = transcode.ProcessResult();
		fflush(stdout);
		fflush(stderr);

	}
	return NULL;
}

void *udp_send(void* param)
{
	int i = *(int*)param;
	unsigned char p[752];
	while (1)
	{
		
		memset(p, 0, 752);
		
		if (tsencode[i]->r_addr == tsencode[i]->w_addr)
		{
			Sleep(1);
			continue;
		}
		else if (tsencode[i]->r_addr < tsencode[i]->w_addr)
		{

			if (tsencode[i]->r_addr + 4 < tsencode[i]->w_addr)
			{
				memcpy(p, tsencode[i]->ts_buf[tsencode[i]->r_addr], TS_PACKET_LEN);
				memcpy(&p[188], tsencode[i]->ts_buf[tsencode[i]->r_addr + 1], TS_PACKET_LEN);
				memcpy(&p[376], tsencode[i]->ts_buf[tsencode[i]->r_addr + 2], TS_PACKET_LEN);
				memcpy(&p[564], tsencode[i]->ts_buf[tsencode[i]->r_addr + 3], TS_PACKET_LEN);
				
				sendto(udpsend[i]->send_socket_fd, (const char*)p, TS_PACKET_LEN * 4, 0, (LPSOCKADDR)&udpsend[i]->des_addr, sizeof(struct sockaddr_in));
				//if(i==0)
				//sendto(udpsend[i]->server_socket_fd, (const char*)p, TS_PACKET_LEN * 4, 0, (LPSOCKADDR)&udpsend[i]->src_addr, sizeof(struct sockaddr_in));
				tsencode[i]->r_addr = (tsencode[i]->r_addr + 4) % 4096;
			/*	if(i==0)
				fwrite(p, 1, 752, fp1);
			*/
			}
			else
			{
				Sleep(1);
				continue;
			}
		}
		else if (tsencode[i]->r_addr > tsencode[i]->w_addr)
		{
			if ((4096 - tsencode[i]->r_addr + tsencode[i]->w_addr) > 4)
			{

				memcpy(p, tsencode[i]->ts_buf[tsencode[i]->r_addr], 188);
				memcpy(&p[188], tsencode[i]->ts_buf[(tsencode[i]->r_addr + 1) % 4096], 188);
				memcpy(&p[376], tsencode[i]->ts_buf[(tsencode[i]->r_addr + 2) % 4096], 188);
				memcpy(&p[564], tsencode[i]->ts_buf[(tsencode[i]->r_addr + 3) % 4096], 188);
				sendto(udpsend[i]->send_socket_fd, (const char*)p, TS_PACKET_LEN * 4, 0, (LPSOCKADDR)&udpsend[i]->des_addr, sizeof(struct sockaddr_in));
				//if(i==0)
				//sendto(udpsend[i]->server_socket_fd, (const char*)p, TS_PACKET_LEN * 4, 0, (LPSOCKADDR)&udpsend[i]->src_addr, sizeof(struct sockaddr_in));
				tsencode[i]->r_addr = (tsencode[i]->r_addr + 4) % 4096;
				/*if (i == 0)
					fwrite(p, 1, 752, fp1);
				*/
		
			}
			else {
				Sleep(1);
				continue;
			}
		}
	}
	return NULL;
}
void *ts_demuxer(void* param)
{
	int i = *(int *)param;
	decodebuf[i] = new decodepool();

	tscode[i]->init(i);
	decodebuf[i]->init();
	
	
	pthread_t transcodess;
	
	
	memset(&transcodess, 0, sizeof(transcodess));
	
	pthread_create(&transcodess, NULL, transcodess_start, &i);
   
	pthread_detach(transcodess);
	int a[40];
	for(int j=0;j<sendnum;j++)
		sendbuf[j] = new outpool(decodebuf[i]->weith);
	for (int j = 0; j < sendnum; j++)
	{
		a[j] = j;
		
		pthread_t send_rtmp;
		pthread_t send_udp;
		memset(&send_udp, 0, sizeof(send_udp));
		memset(&send_rtmp, 0, sizeof(send_rtmp));
		pthread_create(&send_rtmp, NULL, RunSendRtmp, &a[j]);
		pthread_create(&send_udp, NULL, udp_send, &a[j]);
		pthread_detach(send_rtmp);
		pthread_detach(send_udp);
	}
	tscode[i]->trans(i);
	return NULL;
}


Launcher::Launcher():
    m_StartTime(0),
    m_eDevType(static_cast<mfxHandleType>(0))
{
} // Launcher::Launcher()

Launcher::~Launcher()
{
    Close();
} // Launcher::~Launcher()

CTranscodingPipeline* CreatePipeline()
{
    MOD_SMT_CREATE_PIPELINE;

    return new CTranscodingPipeline;
}

mfxStatus Launcher::Init(int n)
{
    mfxStatus sts;
    int i = 0;
    SafetySurfaceBuffer* pBuffer = NULL;
    mfxU32 BufCounter = 0;
    mfxHDL hdl = NULL;
    sInputParams    InputParams;

    //parent transcode pipeline
    CTranscodingPipeline *pParentPipeline = NULL;
    // source transcode pipeline use instead parent in heterogeneous pipeline
    CTranscodingPipeline *pSinkPipeline = NULL;

    // parse input par file
    sts = m_parser.ParseCmdLine(n);
    MSDK_CHECK_PARSE_RESULT(sts, MFX_ERR_NONE, sts);

    // get parameters for each session from parser
    while(m_parser.GetNextSessionParams(InputParams))
    {
        m_InputParamsArray.push_back(InputParams);
        InputParams.Reset();
    }

    // check correctness of input parameters
    sts = VerifyCrossSessionsOptions();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

#if defined(_WIN32) || defined(_WIN64)
    if (m_eDevType == MFX_HANDLE_D3D9_DEVICE_MANAGER)
    {
        m_pAllocParam.reset(new D3DAllocatorParams);
        m_hwdev.reset(new CD3D9Device());
        /* The last param set in vector always describe VPP+ENCODE or Only VPP
         * So, if we want to do rendering we need to do pass HWDev to CTranscodingPipeline */
        if (m_InputParamsArray[m_InputParamsArray.size() -1].eModeExt == VppCompOnly)
        {
            /* Rendering case */
            sts = m_hwdev->Init(NULL, 1, MSDKAdapter::GetNumber() );
            m_InputParamsArray[m_InputParamsArray.size() -1].m_hwdev = m_hwdev.get();
        }
        else /* NO RENDERING*/
        {
            sts = m_hwdev->Init(NULL, 0, MSDKAdapter::GetNumber() );
        }
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        sts = m_hwdev->GetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, (mfxHDL*)&hdl);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        // set Device Manager to external dx9 allocator
        D3DAllocatorParams *pD3DParams = dynamic_cast<D3DAllocatorParams*>(m_pAllocParam.get());
        pD3DParams->pManager =(IDirect3DDeviceManager9*)hdl;
    }
#if MFX_D3D11_SUPPORT
    else if (m_eDevType == MFX_HANDLE_D3D11_DEVICE)
    {

        m_pAllocParam.reset(new D3D11AllocatorParams);
        m_hwdev.reset(new CD3D11Device());
        /* The last param set in vector always describe VPP+ENCODE or Only VPP
         * So, if we want to do rendering we need to do pass HWDev to CTranscodingPipeline */
        if (m_InputParamsArray[m_InputParamsArray.size() -1].eModeExt == VppCompOnly)
        {
            /* Rendering case */
            sts = m_hwdev->Init(NULL, 1, MSDKAdapter::GetNumber() );
            m_InputParamsArray[m_InputParamsArray.size() -1].m_hwdev = m_hwdev.get();
        }
        else /* NO RENDERING*/
        {
            sts = m_hwdev->Init(NULL, 0, MSDKAdapter::GetNumber() );
        }
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        sts = m_hwdev->GetHandle(MFX_HANDLE_D3D11_DEVICE, (mfxHDL*)&hdl);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        // set Device to external dx11 allocator
        D3D11AllocatorParams *pD3D11Params = dynamic_cast<D3D11AllocatorParams*>(m_pAllocParam.get());
        pD3D11Params->pDevice =(ID3D11Device*)hdl;

    }
#endif
#elif defined(LIBVA_X11_SUPPORT) || defined(LIBVA_DRM_SUPPORT)
    if (m_eDevType == MFX_HANDLE_VA_DISPLAY)
    {
        mfxI32  libvaBackend = 0;

        m_pAllocParam.reset(new vaapiAllocatorParams);
        vaapiAllocatorParams *pVAAPIParams = dynamic_cast<vaapiAllocatorParams*>(m_pAllocParam.get());
        /* The last param set in vector always describe VPP+ENCODE or Only VPP
         * So, if we want to do rendering we need to do pass HWDev to CTranscodingPipeline */
        if (m_InputParamsArray[m_InputParamsArray.size() -1].eModeExt == VppCompOnly)
        {
            sInputParams& params = m_InputParamsArray[m_InputParamsArray.size() -1];
            libvaBackend = params.libvaBackend;

            /* Rendering case */
            m_hwdev.reset(CreateVAAPIDevice(params.libvaBackend));
            if(!m_hwdev.get()) {
                msdk_printf(MSDK_STRING("error: failed to initialize VAAPI device\n"));
                return MFX_ERR_DEVICE_FAILED;
            }
            sts = m_hwdev->Init(&params.monitorType, 1, MSDKAdapter::GetNumber() );
            if (params.libvaBackend == MFX_LIBVA_DRM_MODESET) {
                CVAAPIDeviceDRM* drmdev = dynamic_cast<CVAAPIDeviceDRM*>(m_hwdev.get());
                pVAAPIParams->m_export_mode = vaapiAllocatorParams::CUSTOM_FLINK;
                pVAAPIParams->m_exporter = dynamic_cast<vaapiAllocatorParams::Exporter*>(drmdev->getRenderer());

            }
#if defined(LIBVA_WAYLAND_SUPPORT)
            else if (params.libvaBackend == MFX_LIBVA_WAYLAND) {
                VADisplay va_dpy = NULL;
                sts = m_hwdev->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL *)&va_dpy);
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
                hdl = pVAAPIParams->m_dpy =(VADisplay)va_dpy;

                mfxHDL whdl = NULL;
                mfxHandleType hdlw_t = (mfxHandleType)HANDLE_WAYLAND_DRIVER;
                Wayland *wld;
                sts = m_hwdev->GetHandle(hdlw_t, &whdl);
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
                wld = (Wayland*)whdl;
                wld->SetRenderWinPos(params.nRenderWinX, params.nRenderWinY);
                wld->SetPerfMode(params.bPerfMode);

                pVAAPIParams->m_export_mode = vaapiAllocatorParams::PRIME;
            }
#endif // LIBVA_WAYLAND_SUPPORT
            params.m_hwdev = m_hwdev.get();
        }
        else /* NO RENDERING*/
        {
            m_hwdev.reset(CreateVAAPIDevice());
            if(!m_hwdev.get()) {
                msdk_printf(MSDK_STRING("error: failed to initialize VAAPI device\n"));
                return MFX_ERR_DEVICE_FAILED;
            }
            sts = m_hwdev->Init(NULL, 0, MSDKAdapter::GetNumber());
        }
        if (libvaBackend != MFX_LIBVA_WAYLAND) {
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        sts = m_hwdev->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL*)&hdl);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        // set Device to external vaapi allocator
        pVAAPIParams->m_dpy =(VADisplay)hdl;
    }
    }
#endif
    if (!m_pAllocParam.get())
    {
        m_pAllocParam.reset(new SysMemAllocatorParams);
    }

    // each pair of source and sink has own safety buffer
    sts = CreateSafetyBuffers();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    /* One more hint. Example you have 3 dec + 1 enc sessions
    * (enc means vpp_comp call invoked. m_InputParamsArray.size() is 4.
    * You don't need take vpp comp params from last one session as it is enc session.
    * But you need process {0, 1, 2} sessions - totally 3.
    * So, you need start from 0 and end at 2.
    * */
    for(mfxI32 jj = 0; jj<(mfxI32)m_InputParamsArray.size() - 1; jj++)
    {
        /* Save params for VPP composition */
        sVppCompDstRect tempDstRect;
        tempDstRect.DstX = m_InputParamsArray[jj].nVppCompDstX;
        tempDstRect.DstY = m_InputParamsArray[jj].nVppCompDstY;
        tempDstRect.DstW = m_InputParamsArray[jj].nVppCompDstW;
        tempDstRect.DstH = m_InputParamsArray[jj].nVppCompDstH;
        m_VppDstRects.push_back(tempDstRect);
    }

    // create sessions, allocators
    for (i = 0; i < m_InputParamsArray.size(); i++)
    {
        GeneralAllocator* pAllocator = new GeneralAllocator;
        sts = pAllocator->Init(m_pAllocParam.get());
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        m_pAllocArray.push_back(pAllocator);

        std::auto_ptr<ThreadTranscodeContext> pThreadPipeline(new ThreadTranscodeContext);
        // extend BS processing init
        m_InputParamsArray[i].nTimeout == 0 ? m_pExtBSProcArray.push_back(new FileBitstreamProcessor) :
                                        m_pExtBSProcArray.push_back(new FileBitstreamProcessor_WithReset);
        pThreadPipeline->pPipeline.reset(CreatePipeline());

        pThreadPipeline->pBSProcessor = m_pExtBSProcArray.back();
        if (Sink == m_InputParamsArray[i].eMode)
        {
            /* N_to_1 mode */
            if ((VppComp == m_InputParamsArray[i].eModeExt) ||
                (VppCompOnly == m_InputParamsArray[i].eModeExt))
            {
                // Taking buffers from tail because they are stored in m_pBufferArray in reverse order
                // So, by doing this we'll fill buffers properly according to order from par file
                pBuffer = m_pBufferArray[m_pBufferArray.size()-1-BufCounter];
                BufCounter++;
            }
            else /* 1_to_N mode*/
            {
                pBuffer = m_pBufferArray[m_pBufferArray.size() - 1];
            }
            pSinkPipeline = pThreadPipeline->pPipeline.get();
            sts = m_pExtBSProcArray.back()->Init(m_InputParamsArray[i].strSrcFile, NULL,i);
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        }
        else if (Source == m_InputParamsArray[i].eMode)
        {
            /* N_to_1 mode */
            if ((VppComp == m_InputParamsArray[i].eModeExt) ||
                (VppCompOnly == m_InputParamsArray[i].eModeExt))
            {
                pBuffer = m_pBufferArray[m_pBufferArray.size() - 1];
            }
            else /* 1_to_N mode*/
            {
                pBuffer = m_pBufferArray[BufCounter];
                BufCounter++;
            }
            sts = m_pExtBSProcArray.back()->Init(NULL, m_InputParamsArray[i].strDstFile,i, m_InputParamsArray[i].usebigf);
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        }
        else
        {
            sts = m_pExtBSProcArray.back()->Init(m_InputParamsArray[i].strSrcFile, m_InputParamsArray[i].strDstFile,0);
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
            pBuffer = NULL;
        }

        /**/
        /* Vector stored linearly in the memory !*/
        m_InputParamsArray[i].pVppCompDstRects = m_VppDstRects.empty() ? NULL : &m_VppDstRects[0];

        // if session has VPP plus ENCODE only (-i::source option)
        // use decode source session as input
        sts = MFX_ERR_MORE_DATA;
        if (Source == m_InputParamsArray[i].eMode)
        {
            sts = pThreadPipeline->pPipeline->Init(&m_InputParamsArray[i],
                                                   m_pAllocArray[i],
                                                   hdl,
                                                   pSinkPipeline,
                                                   pBuffer,
                                                   m_pExtBSProcArray.back(),
				                                   i,n);
        }
        else
        {
            sts =  pThreadPipeline->pPipeline->Init(&m_InputParamsArray[i],
                                                    m_pAllocArray[i],
                                                    hdl,
                                                    pParentPipeline,
                                                    pBuffer,
                                                    m_pExtBSProcArray.back(),
				                                    i,n);
        }

        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        if (!pParentPipeline && m_InputParamsArray[i].bIsJoin)
            pParentPipeline = pThreadPipeline->pPipeline.get();

        // set the session's start status (like it is waiting)
        pThreadPipeline->startStatus = MFX_WRN_DEVICE_BUSY;
        // set other session's parameters
        pThreadPipeline->implType = m_InputParamsArray[i].libType;
        m_pSessionArray.push_back(pThreadPipeline.release());

        mfxVersion ver = {{0, 0}};
        sts = m_pSessionArray[i]->pPipeline->QueryMFXVersion(&ver);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        PrintInfo(i, &m_InputParamsArray[i], &ver);
    }

    for (i = 0; i < m_InputParamsArray.size(); i++)
    {
        sts = m_pSessionArray[i]->pPipeline->CompleteInit();
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        if (m_pSessionArray[i]->pPipeline->GetJoiningFlag())
            msdk_printf(MSDK_STRING("Session %d was joined with other sessions\n"), i);
        else
            msdk_printf(MSDK_STRING("Session %d was NOT joined with other sessions\n"), i);

        m_pSessionArray[i]->pPipeline->SetPipelineID(i);
    }

    msdk_printf(MSDK_STRING("\n"));

    return sts;

} // mfxStatus Launcher::Init()

void Launcher::Run(int n)
{
    mfxU32 totalSessions;
	int index = 0;
    msdk_printf(MSDK_STRING("Transcoding started\n"));
    // mark start time
    m_StartTime = GetTick();

    // get parallel sessions parameters
    totalSessions = (mfxU32) m_pSessionArray.size();

    mfxU32 i;
    mfxStatus sts;

    MSDKThread * pthread = NULL;
	
	
	
    for (i = 0; i < totalSessions; i++)
    {
		m_pSessionArray[i]->index = i;
		m_pSessionArray[i]->num = n;
        pthread = new MSDKThread(sts, ThranscodeRoutine, (void *)m_pSessionArray[i]);

        m_HDLArray.push_back(pthread);
    } 
	/*nnn++;
	n--;
	if (n > 0)
	{
		mfxStatus sts;
		Launcher transcode;
		sts = transcode.Init(nnn);
		fflush(stdout);
		fflush(stderr);


		transcode.Run(n);
	}*/
    for (i = 0; i < m_pSessionArray.size(); i++)
    {
        m_HDLArray[i]->Wait();
    }
	
   // msdk_printf(MSDK_STRING("\nTranscoding finished\n"));

} // mfxStatus Launcher::Init()

mfxStatus Launcher::ProcessResult()
{
    FILE* pPerfFile = m_parser.GetPerformanceFile();
    msdk_printf(MSDK_STRING("\nCommon transcoding time is  %.2f sec \n"), GetTime(m_StartTime));

    m_parser.PrintParFileName();

    if (pPerfFile)
    {
        msdk_fprintf(pPerfFile, MSDK_STRING("Common transcoding time is  %.2f sec \n"), GetTime(m_StartTime));
    }

    // get result
    bool SuccessTranscode = true;
    mfxU32 i;
    for (i = 0; i < m_pSessionArray.size(); i++)
    {
        mfxStatus sts = m_pSessionArray[i]->transcodingSts;
        if (MFX_ERR_NONE != sts)
        {
            SuccessTranscode = false;
            msdk_printf(MSDK_STRING("MFX session %d transcoding FAILED:\nProcessing time: %.2f sec \nNumber of processed frames: %d\n"),
                i,
                m_pSessionArray[i]->working_time,
                m_pSessionArray[i]->numTransFrames);
            if (pPerfFile)
            {
                msdk_fprintf(pPerfFile, MSDK_STRING("MFX session %d transcoding FAILED:\nProcessing time: %.2f sec \nNumber of processed frames: %d\n"),
                    i,
                    m_pSessionArray[i]->working_time,
                    m_pSessionArray[i]->numTransFrames);
            }

        }
        else
        {
            msdk_printf(MSDK_STRING("MFX session %d transcoding PASSED:\nProcessing time: %.2f sec \nNumber of processed frames: %d\n"),
                i,
                m_pSessionArray[i]->working_time,
                m_pSessionArray[i]->numTransFrames);
            if (pPerfFile)
            {
                msdk_fprintf(pPerfFile, MSDK_STRING("MFX session %d transcoding PASSED:\nProcessing time: %.2f sec \nNumber of processed frames: %d\n"),
                    i,
                    m_pSessionArray[i]->working_time,
                    m_pSessionArray[i]->numTransFrames);
            }
        }

        if (pPerfFile)
        {
            if (Native == m_InputParamsArray[i].eMode || Sink == m_InputParamsArray[i].eMode)
            {
                msdk_fprintf(pPerfFile, MSDK_STRING("Input stream: %s\n"), m_InputParamsArray[i].strSrcFile);
            }
            else
                msdk_fprintf(pPerfFile, MSDK_STRING("Input stream: from parent session\n"));
            msdk_fprintf(pPerfFile, MSDK_STRING("\n"));
        }


    }

    if (SuccessTranscode)
    {
        msdk_printf(MSDK_STRING("\nThe test PASSED\n"));
        if (pPerfFile)
        {
            msdk_fprintf(pPerfFile, MSDK_STRING("\nThe test PASSED\n"));
        }
        return MFX_ERR_NONE;
    }
    else
    {
        msdk_printf(MSDK_STRING("\nThe test FAILED\n"));
        if (pPerfFile)
        {
            msdk_fprintf(pPerfFile, MSDK_STRING("\nThe test FAILED\n"));
        }
        return MFX_ERR_UNKNOWN;
    }
} // mfxStatus Launcher::ProcessResult()

mfxStatus Launcher::VerifyCrossSessionsOptions()
{
    bool IsSinkPresence = false;
    bool IsSourcePresence = false;
    bool IsHeterSessionJoin = false;
    bool IsFirstInTopology = true;
    bool areAllInterSessionsOpaque = true;

    mfxU16 minAsyncDepth = 0;
    bool bUseExternalAllocator = false;
    for (mfxU32 i = 0; i < m_InputParamsArray.size(); i++)
    {
        if (!m_InputParamsArray[i].bUseOpaqueMemory &&
            ((m_InputParamsArray[i].eMode == Source) || (m_InputParamsArray[i].eMode == Sink)))
        {
            areAllInterSessionsOpaque = false;
        }

        if (m_InputParamsArray[i].bOpenCL ||
            m_InputParamsArray[i].EncoderFourCC ||
            m_InputParamsArray[i].DecoderFourCC)
        {
            bUseExternalAllocator = true;
        }

        // All sessions have to know about timeout
        if (m_InputParamsArray[i].nTimeout && (m_InputParamsArray[i].eMode == Sink))
        {
            for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++)
            {
                if (m_InputParamsArray[j].MaxFrameNumber != MFX_INFINITE)
                {
                    msdk_printf(MSDK_STRING("\"-timeout\" option isn't compatible with \"-n\". \"-n\" will be ignored.\n"));
                    for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++)
                        m_InputParamsArray[j].MaxFrameNumber = MFX_INFINITE;
                }
            }
            msdk_printf(MSDK_STRING("Timeout %d seconds has been set to all sessions\n"), m_InputParamsArray[i].nTimeout);
            for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++)
                m_InputParamsArray[j].nTimeout = m_InputParamsArray[i].nTimeout;
        }

        if (Source == m_InputParamsArray[i].eMode)
        {
            if (m_InputParamsArray[i].nAsyncDepth < minAsyncDepth)
            {
                minAsyncDepth = m_InputParamsArray[i].nAsyncDepth;
            }
            // topology definition
            if (!IsSinkPresence)
            {
                PrintError(MSDK_STRING("Error in par file. Decode source session must be declared BEFORE encode sinks \n"));
                return MFX_ERR_UNSUPPORTED;
            }
            IsSourcePresence = true;

            if (IsFirstInTopology)
            {
                if (m_InputParamsArray[i].bIsJoin)
                    IsHeterSessionJoin = true;
                else
                    IsHeterSessionJoin = false;
            }
            else
            {
                if (m_InputParamsArray[i].bIsJoin && !IsHeterSessionJoin)
                {
                    PrintError(MSDK_STRING("Error in par file. All heterogeneous sessions must be joined \n"));
                    return MFX_ERR_UNSUPPORTED;
                }
                if (!m_InputParamsArray[i].bIsJoin && IsHeterSessionJoin)
                {
                    PrintError(MSDK_STRING("Error in par file. All heterogeneous sessions must be NOT joined \n"));
                    return MFX_ERR_UNSUPPORTED;
                }
            }

            if (IsFirstInTopology)
                IsFirstInTopology = false;

        }
        else if (Sink == m_InputParamsArray[i].eMode)
        {
            minAsyncDepth = m_InputParamsArray[i].nAsyncDepth;
            IsSinkPresence = true;

            if (IsFirstInTopology)
            {
                if (m_InputParamsArray[i].bIsJoin)
                    IsHeterSessionJoin = true;
                else
                    IsHeterSessionJoin = false;
            }
            else
            {
                if (m_InputParamsArray[i].bIsJoin && !IsHeterSessionJoin)
                {
                    PrintError(MSDK_STRING("Error in par file. All heterogeneous sessions must be joined \n"));
                    return MFX_ERR_UNSUPPORTED;
                }
                if (!m_InputParamsArray[i].bIsJoin && IsHeterSessionJoin)
                {
                    PrintError(MSDK_STRING("Error in par file. All heterogeneous sessions must be NOT joined \n"));
                    return MFX_ERR_UNSUPPORTED;
                }
            }

            if (IsFirstInTopology)
                IsFirstInTopology = false;
        }
        if (MFX_IMPL_SOFTWARE != m_InputParamsArray[i].libType)
        {
            // TODO: can we avoid ifdef and use MFX_IMPL_VIA_VAAPI?
#if defined(_WIN32) || defined(_WIN64)
            m_eDevType = (MFX_IMPL_VIA_D3D11 == MFX_IMPL_VIA_MASK(m_InputParamsArray[i].libType))?
                MFX_HANDLE_D3D11_DEVICE :
                MFX_HANDLE_D3D9_DEVICE_MANAGER;
#elif defined(LIBVA_SUPPORT)
            m_eDevType = MFX_HANDLE_VA_DISPLAY;
#endif
        }
    }

    if (bUseExternalAllocator)
    {
        for(mfxU32 i = 0; i < m_InputParamsArray.size(); i++)
        {
            m_InputParamsArray[i].bUseOpaqueMemory = false;
        }
        msdk_printf(MSDK_STRING("OpenCL or chroma conversion is present at least in one session. External memory allocator will be used for all sessions .\n"));
    }

    // Async depth between inter-sessions should be equal to the minimum async depth of all these sessions.
    for (mfxU32 i = 0; i < m_InputParamsArray.size(); i++)
    {
        if ((m_InputParamsArray[i].eMode == Source) || (m_InputParamsArray[i].eMode == Sink))
        {
            m_InputParamsArray[i].nAsyncDepth = minAsyncDepth;

            //--- If at least one of inter-session is not using opaque memory, all of them should not use it
            if(!areAllInterSessionsOpaque)
            {
                m_InputParamsArray[i].bUseOpaqueMemory=false;
            }
        }
    }

    if(!areAllInterSessionsOpaque)
    {
        msdk_printf(MSDK_STRING("Some inter-sessions do not use opaque memory (possibly because of -o::raw).\nOpaque memory in all inter-sessions is disabled.\n"));
    }

    if (IsSinkPresence && !IsSourcePresence)
    {
        PrintError(MSDK_STRING("Error: Sink must be defined"));
        return MFX_ERR_UNSUPPORTED;
    }
    return MFX_ERR_NONE;

} // mfxStatus Launcher::VerifyCrossSessionsOptions()

mfxStatus Launcher::CreateSafetyBuffers()
{
    SafetySurfaceBuffer* pBuffer     = NULL;
    SafetySurfaceBuffer* pPrevBuffer = NULL;

    for (mfxU32 i = 0; i < m_InputParamsArray.size(); i++)
    {
        /* this is for 1 to N case*/
        if ((Source == m_InputParamsArray[i].eMode) &&
            (Native == m_InputParamsArray[0].eModeExt))
        {
            pBuffer = new SafetySurfaceBuffer(pPrevBuffer);
            pPrevBuffer = pBuffer;
            m_pBufferArray.push_back(pBuffer);
        }

        /* And N_to_1 case: composition should be enabled!
         * else it is logic error */
        if ( (Source != m_InputParamsArray[i].eMode) &&
             ( (VppComp     == m_InputParamsArray[0].eModeExt) ||
               (VppCompOnly == m_InputParamsArray[0].eModeExt) ) )
        {
            pBuffer = new SafetySurfaceBuffer(pPrevBuffer);
            pPrevBuffer = pBuffer;
            m_pBufferArray.push_back(pBuffer);
        }
    }
    return MFX_ERR_NONE;

} // mfxStatus Launcher::CreateSafetyBuffers

void Launcher::Close()
{
    while(m_pSessionArray.size())
    {
        delete m_pSessionArray[m_pSessionArray.size()-1];
        m_pSessionArray[m_pSessionArray.size() - 1] = NULL;
        delete m_pAllocArray[m_pSessionArray.size()-1];
        m_pAllocArray[m_pSessionArray.size() - 1] = NULL;
        m_pAllocArray.pop_back();
        m_pSessionArray.pop_back();
    }
    while(m_pBufferArray.size())
    {
        delete m_pBufferArray[m_pBufferArray.size()-1];
        m_pBufferArray[m_pBufferArray.size() - 1] = NULL;
        m_pBufferArray.pop_back();
    }

    while(m_pExtBSProcArray.size())
    {
        delete m_pExtBSProcArray[m_pExtBSProcArray.size() - 1];
        m_pExtBSProcArray[m_pExtBSProcArray.size() - 1] = NULL;
        m_pExtBSProcArray.pop_back();
    }

    while (m_HDLArray.size())
    {
        delete m_HDLArray[m_HDLArray.size()-1];
        m_HDLArray.pop_back();
    }
} // void Launcher::Close()

#if defined(_WIN32) || defined(_WIN64)
int _tmain(int argc, TCHAR *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	
	FILE *fp = fopen("program_name.txt", "rb");
	char *s=new char[1024*5];
	int size = fread(s, 1, 1024 * 5, fp);
	fclose(fp);
	
	char c[255];
	int n = 0;
	for (int i = 0; i < size; i++)
	{
		int j = 0;
		if (s[i] == '"')
		{
			i++;
			while (i < size)
			{
				c[j++] = s[i++];
				if (s[i] == '"')
					break;
			}
			program_name[n] = new char[j+1];
			memcpy(program_name[n], c, j);
			program_name[n][j] = '\0';
			n++;
		}
	}
	
	InitSockets();
	 //n=numrecv();

	
	 n = 1;
	int *a = (int *)malloc(sizeof(int)*40);
	
	
	
	
	for (int i = 0; i < n; i++)
	{
		a[i] = i;
		

		recvpool[i] = new udprecv();
		recvpool[i]->init();
		tscode[i] = new transcode();
		

		pthread_t udp;
		pthread_t ts_demux;
		
		//pthread_t send_udp;
		memset(&udp, 0, sizeof(udp));
		memset(&ts_demux, 0, sizeof(ts_demux));
		
		//memset(&send_udp, 0, sizeof(recvinfo));
		pthread_create(&udp, NULL, udp_tsrecv, &a[i]);
		pthread_create(&ts_demux, NULL, ts_demuxer, &a[i]);
		
		
		pthread_detach(udp);
		
		pthread_detach(ts_demux);
		
		Sleep(10);
	}
	
	for (int i = 0; i <sendnum; i++)
	{
		a[i] = i;
		char *p = "192.168.99.5";//发送IP
		udpsend[i] = new udp();
		
		udpsend[i]->sendinit(p, i);
		tsencode[i] = new tsenc();
		tsencode[i]->init();
		pthread_t recvinfo;
		memset(&recvinfo, 0, sizeof(recvinfo));
		pthread_create(&recvinfo, NULL, inforecv, &a[i]);
		pthread_detach(recvinfo);
	}
	//udpsend[0]->init(0);
	getchar();
    return 0;
}

