#pragma once
#include"pthread.h"
extern "C"
{
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libavfilter/avfiltergraph.h"  
#include "libavfilter/buffersink.h"  
#include "libavfilter/buffersrc.h"  
#include "libavutil/avutil.h"  
#include "libavutil/opt.h"  
#include "libavutil/pixdesc.h"
#include "libavutil/avstring.h"
#include"libswresample/swresample.h"
#include"libavutil/audio_fifo.h"
#include"libswresample/swresample_internal.h"
}
typedef struct sendmes
{
	int weith;
	int heght;
	int docedetype;
	bool initbool;
}sensymbol;
class transcode
{
public:
	transcode();
	~transcode();

	int init( int j);
	void trans(int i);
private:
	int open_output(int j);
	int open_input(int i);
	//int read_data(void *opaque, uint8_t *buf, int buf_size);
	int videoindex;
	int audioindex;
	int index11;
	int dum;
	
	AVFormatContext *ifmt_ctx;
	AVFormatContext *ofmt_ctx;
	AVCodecContext *input_codec_context;
	AVCodecContext *onput_codec_context;
	SwrContext *resample_context;
	AVAudioFifo *fifo;
	AVFrame *input_frame, *output_frame;
	uint8_t **converted_input_samples;
	AVPacket input_packet,output_packet;
};
