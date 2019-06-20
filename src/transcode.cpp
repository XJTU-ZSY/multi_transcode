#include"transcode.h"
#include"udprecv.h"
#include"decodepool.h"
#include"outpool.h"
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
//template<typename T1, typename T2>
//constexpr auto FFMIN(T1 a, T2 b) { return ((a) > (b) ? (b) : (a)); }
extern udprecv* recvpool[40];
extern decodepool *decodebuf[40];
extern outpool* sendbuf[40];
pthread_mutex_t lockover = PTHREAD_MUTEX_INITIALIZER;
extern int sendnum;
transcode::transcode()
{
	converted_input_samples = NULL;
	ifmt_ctx = NULL;
	ofmt_ctx = NULL;
	resample_context = NULL;
	input_codec_context = NULL;
	onput_codec_context = NULL;
	fifo = NULL;
	videoindex = 0;
	audioindex = 0;
	input_frame = NULL;
	output_frame = NULL;
	index11 = 0;
	dum = 0;
}
transcode::~transcode()
{

}
int read_data(void *opaque, uint8_t *buf, int buf_size)
{
	int i = *(int*)opaque;
	int size = buf_size;
	int ret;
	do {
		ret = recvpool[i]->get_queue(buf, size);
		if (recvpool[i]->indexnumber >= 3000)
		{
			printf("%d road udp not recv data\n", i);
			recvpool[i]->indexnumber = 0;
		}
		if (ret == 1)
		{
			Sleep(1);
		}
	} while (ret);
	return size;
}
int transcode::init(int j)
{

	int error = 0;
	error = open_input(j);
	if (error != 1)
		printf("open input error/n");
	error = open_output(j);
	if (error != 1)
		printf("open output error/n");

	if (!(fifo = av_audio_fifo_alloc(onput_codec_context->sample_fmt,
		onput_codec_context->channels, 1))) {
		fprintf(stderr, "Could not allocate FIFO\n");
		return AVERROR(ENOMEM);
	}
	converted_input_samples = (uint8_t **)calloc(onput_codec_context->channels, sizeof(*converted_input_samples));
}

int transcode::open_input(int i)
{
	index11 = i;
	av_register_all();
	AVCodecContext *avctx;
	AVCodec *input_codec;
	AVInputFormat *piFmt = NULL;
	int error;
	AVIOContext *pb = NULL;
	//step1:申请一个AVIOContext
	uint8_t *buffer = (uint8_t*)av_mallocz(sizeof(uint8_t) * 1024);
	pb = avio_alloc_context(buffer, 1024, 0, &index11, read_data, NULL, NULL);
	if (!pb) {
		printf("avio alloc failed!\n");
		return 0;
	}
	//step2:探测流格式
	if (av_probe_input_buffer(pb, &piFmt, "", NULL, 0, 0) < 0) {
		printf("probe failed!\n");
		return 0;
	}
	else {
		printf("probe success!\n");
		printf("format: %s[%s]\n", piFmt->name, piFmt->long_name);
	}
	ifmt_ctx = avformat_alloc_context();
	ifmt_ctx->pb = pb;

	if (avformat_open_input(&ifmt_ctx, "", piFmt, NULL) < 0) {
		printf("avformat open failed.\n");
		return 0;
	}
	else {
		printf("open stream success!\n");
	}

	if ((error = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
		printf("find stream info erro/n");
		return error;
	}

	for (int n = 0; n < 2; n++)
	{
		if (ifmt_ctx->streams[n]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			videoindex = n;
		if (ifmt_ctx->streams[n]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			audioindex = n;
	}
	input_codec_context = ifmt_ctx->streams[audioindex]->codec;

	avctx = ifmt_ctx->streams[videoindex]->codec;
	decodebuf[i]->weith = avctx->width;
	if (avctx->width == 1920)
		dum = 1800;
	else dum = 3600;
	/*sensymbo1[i].weith = avctx->width;
	sensymbo1[i].heght = avctx->height;
	sensymbo1[i].docedetype = avctx->codec_id;
	sensymbo1[i].initbool = true;*/
	if (input_codec_context->codec_type == AVMEDIA_TYPE_AUDIO)
	{
		int ret = 0;
		ret = avcodec_open2(input_codec_context,
			avcodec_find_decoder(input_codec_context->codec_id), NULL);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Failed toopen decoder for stream #%u\n", i);
			return ret;

		}
	}
	return 1;
}

int transcode::open_output(int j)
{
	AVCodecContext *avctx = NULL;
	AVIOContext *output_io_context = NULL;
	AVStream *stream = NULL;
	AVCodec *output_codec = NULL;
	int error;
	const char* instead = NULL;
	if (j == 0)
		instead = "transcode.aac";
	else if (j == 1)
		instead = "transcode1.aac";
	else if (j == 2)
		instead = "transcode2.aac";
	else if (j == 3)
		instead = "transcode3.aac";
	else if (j == 4)
		instead = "transcode4.aac";
	else if (j == 5)
		instead = "transcode5.aac";
	else if (j == 6)
		instead = "transcode6.aac";
	else if (j == 7)
		instead = "transcode7.aac";
	else if (j == 8)
		instead = "transcode8.aac";
	else if (j == 9)
		instead = "transcode9.aac";
	else if (j == 10)
		instead = "transcode10.aac";
	else if (j == 11)
		instead = "transcode11.aac";
	else if (j == 12)
		instead = "transcode12.aac";
	else if (j == 13)
		instead = "transcode13.aac";
	else if (j == 14)
		instead = "transcode14.aac";
	else if (j == 15)
		instead = "transcode15.aac";
	else if (j == 16)
		instead = "transcode16.aac";
	else if (j == 17)
		instead = "transcode17.aac";
	else if (j == 18)
		instead = "transcode18.aac";
	else if (j == 19)
		instead = "transcode19.aac";
	else if (j == 20)
		instead = "transcode20.aac";
	else if (j == 21)
		instead = "transcode21.aac";
	else if (j == 22)
		instead = "transcode22.aac";
	else if (j == 23)
		instead = "transcode23.aac";

	if ((error = avio_open(&output_io_context, instead,
		AVIO_FLAG_WRITE)) < 0) {
		printf("cannot open output file/n");
		return error;
	}
	if (!(ofmt_ctx = avformat_alloc_context())) {
		fprintf(stderr, "Could not allocate output format context\n");
		return AVERROR(ENOMEM);
	}
	ofmt_ctx->pb = output_io_context;
	if (!(ofmt_ctx->oformat = av_guess_format(NULL, instead,
		NULL))) {
		fprintf(stderr, "Could not find output file format\n");
	}
	av_strlcpy(ofmt_ctx->filename, instead,
		sizeof(ofmt_ctx->filename));
	if (!(output_codec = avcodec_find_encoder(AV_CODEC_ID_AAC))) {
		fprintf(stderr, "Could not find an AAC encoder.\n");
	}
	if (!(stream = avformat_new_stream(ofmt_ctx, NULL))) {
		fprintf(stderr, "Could not create new stream\n");
		error = AVERROR(ENOMEM);
	}
	avctx = avcodec_alloc_context3(output_codec);
	if (!avctx) {
		fprintf(stderr, "Could not allocate an encoding context\n");
		error = AVERROR(ENOMEM);
	}
	avctx->channels = 2;
	avctx->channel_layout = av_get_default_channel_layout(2);
	avctx->sample_rate = input_codec_context->sample_rate;
	avctx->sample_fmt = output_codec->sample_fmts[0];
	avctx->bit_rate = 64000;
	avctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	stream->time_base.den = input_codec_context->sample_rate;
	stream->time_base.num = 1;
	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	/** Open the encoder for the audio stream to use it later. */
	if ((error = avcodec_open2(avctx, output_codec, NULL)) < 0) {
		printf("cannot open audio decode/n");
	}

	error = avcodec_parameters_from_context(stream->codecpar, avctx);
	if (error < 0) {
		fprintf(stderr, "Could not initialize stream parameters\n");
	}

	/** Save the encoder context for easier access later. */
	onput_codec_context = avctx;

	return 1;
}
void transcode::trans(int i)
{
	int error = 0;
	int got_frame;
	const int output_frame_size = onput_codec_context->frame_size;
	//avformat_write_header(ofmt_ctx, NULL);
	int64_t lastpts = 0;
	int64_t lastdts = 0;
	while (1)
	{
		av_init_packet(&input_packet);
		input_packet.data = NULL;
		input_packet.size = 0;

		if (error = av_read_frame(ifmt_ctx, &input_packet) == 0)
		{

			if (input_packet.stream_index == videoindex)
			{
				if (index11 == 0)
					index11 = 0;
				else if (index11 == 1)
					index11 = 1;
				else if (index11 == 2)
					index11 = 2;
				else if (index11 == 3)
					index11 = 3;
				else if (index11 == 4)
					index11 = 4;
				else if (index11 == 5)
					index11 = 5;
				else if (index11 == 6)
					index11 = 6;
				else if (index11 == 7)
					index11 = 7;
				else if (index11 == 8)
					index11 = 8;
				else if (index11 == 9)
					index11 = 9;
				else if (index11 == 10)
					index11 = 10;
				else if (index11 == 11)
					index11 = 11;
				else if (index11 == 12)
					index11 = 12;
				else if (index11 == 13)
					index11 = 13;
				else if (index11 == 14)
					index11 = 14;
				else if (index11 == 15)
					index11 = 15;
				if (input_packet.pts < 0)
				{
					input_packet.pts = lastpts + 14400;
				}
				lastpts = input_packet.pts;

				if (abs(input_packet.dts - lastdts - dum)>3000)
				{
					lastdts = input_packet.dts;
					printf(" %d video dts error\n", index11);
					av_free_packet(&input_packet);
					continue;
				}
				lastdts = input_packet.dts;
				decodebuf[index11]->putts(input_packet.data, input_packet.size, input_packet.pts);
			}
			else if (input_packet.stream_index == audioindex)
			{
				/*if (input_packet.pts < 0)
				{
				av_free_packet(&input_packet);
				continue;
				}
				if (input_packet.dts < lastadts || input_packet.dts- lastadts>5000)
				{
				lastadts = input_packet.dts;
				printf("audio dts error\n");
				av_free_packet(&input_packet);
				continue;
				}
				lastadts = input_packet.dts;*/
				input_frame = av_frame_alloc();
				if (avcodec_decode_audio4(input_codec_context, input_frame, &got_frame, &input_packet) >= 0)
				{
					if (got_frame == 1)
					{
						if (av_samples_alloc(converted_input_samples, NULL, onput_codec_context->channels, input_frame->nb_samples, onput_codec_context->sample_fmt, 0) < 0)
							printf("alloc audio Sample failed\n");
						resample_context = swr_alloc_set_opts(NULL,
							av_get_default_channel_layout(onput_codec_context->channels),
							onput_codec_context->sample_fmt,
							onput_codec_context->sample_rate,
							av_get_default_channel_layout(input_codec_context->channels),
							input_codec_context->sample_fmt,
							input_codec_context->sample_rate,
							0, NULL);
						if (!resample_context) {
							fprintf(stderr, "Could not allocate resample context\n");
						}
						if ((error = swr_init(resample_context)) < 0) {
							fprintf(stderr, "Could not open resample context\n");
							swr_free(&resample_context);
						}
						if (swr_convert(resample_context, converted_input_samples, input_frame->nb_samples, (const uint8_t**)input_frame->extended_data, input_frame->nb_samples) < 0)
							printf("swr convert error %d\n");
						av_audio_fifo_write(fifo, (void **)converted_input_samples, input_frame->nb_samples);

						while (av_audio_fifo_size(fifo) >= onput_codec_context->frame_size)
						{
							int frame_size = 0;
							frame_size = FFMIN(av_audio_fifo_size(fifo), onput_codec_context->frame_size);

							output_frame = av_frame_alloc();
							output_frame->nb_samples = frame_size;
							output_frame->channel_layout = onput_codec_context->channel_layout;
							output_frame->sample_rate = onput_codec_context->sample_rate;
							output_frame->format = onput_codec_context->sample_fmt;
							av_frame_get_buffer(output_frame, 0);
							av_audio_fifo_read(fifo, (void **)&output_frame->data, frame_size);
							output_frame->pts = input_packet.pts - (av_audio_fifo_size(fifo) - onput_codec_context->frame_size) / 128 * 24 * 10;
							av_init_packet(&output_packet);
							output_packet.data = NULL;
							output_packet.size = 0;
							if (avcodec_encode_audio2(onput_codec_context, &output_packet, output_frame, &got_frame) == 0)
							{
								if (got_frame == 1)
								{
									for (int i = 0; i<sendnum; i++)
										sendbuf[i]->Write(&output_packet);

									//av_write_frame(ofmt_ctx, &output_packet);

								}
								av_free_packet(&output_packet);
							}
							else printf("encode audio frame error\n");

							av_frame_free(&output_frame);
						}
						swr_free(&resample_context);
						if (converted_input_samples)
							av_freep(&converted_input_samples[0]);
					}
				}
				av_frame_free(&input_frame);
			}
		}
		av_free_packet(&input_packet);
	}
}