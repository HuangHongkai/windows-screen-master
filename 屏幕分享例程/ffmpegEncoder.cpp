#include "ffmpegEncoder.h"

bool IEncoder::encode(AVFrame * frame)
{
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	int ret = avcodec_encode_video2(pCodecCtx, &pkt, frame, &got_output);
	if (ret < 0) {
		printf("Error encoding frame\n");
		return false;
	}
	if (got_output)
	{
		if (fp_out != NULL)
			fwrite(pkt.data, 1, pkt.size, fp_out);

		av_free_packet(&pkt);
		printf("encode a frame................\n");
	}
	else
	{
		printf("failed\n");
		return false;
	}
	return true;
}

bool IEncoder::flush(AVFrame * frame)
{
	//Flush Encoder
	for (got_output = 1; got_output;) {
		fflush(stdout);
		int ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
		if (ret < 0) {
			return false;
		}
		if (got_output) {
			for (int i = 0; i < 3; i++)
				pkt.data[i] = 0;
			pkt.data[3] = 1;
			fwrite(pkt.data, 1, pkt.size, fp_out);

			av_free_packet(&pkt);
		}
	}
	fclose(fp_out);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	return true;
}

void x264Encoder::init()
{
	if (filename != NULL)
		fp_out = fopen(filename, "wb");
	pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	pCodecCtx = avcodec_alloc_context3(pCodec);
	pCodecCtx->bit_rate = 4 * 1024 * 1024;
	pCodecCtx->width = width;
	pCodecCtx->height = height;
	pCodecCtx->frame_number = 1;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 13; // fps
	pCodecCtx->gop_size = 10;
	pCodecCtx->max_b_frames = 0;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->slices = 1;
	AVDictionary *opts = NULL;
	av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
	av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);
	av_dict_set(&opts, "profile", "baseline", 0);
	if (avcodec_open2(pCodecCtx, pCodec, &opts) < 0) {
		printf("Could not open codec\n");
		return;
	}
}

void x264Encoder::init_header()
{
	// sps pps 帧头部

	static unsigned char sps_buffer[24] = {
	0x67,
	0x42,0xC0,0x28,0xDA,0x1,0xE0,0x8,0x9F,0x96,0x10,0x0,0x0,0x3,0x0,0x10,0x0,0x0,0x3,0x3,0x28,
	0xF1,0x83,0x2A, };
	static unsigned char pps_buffer[4] = {
		0x68,
		0xCE,0xF,0xC8, };
	int extradata_len = 8 + sizeof(sps_buffer) + sizeof(pps_buffer) + 2 + 1;
	pCodecCtx->extradata_size = extradata_len;
	pCodecCtx->extradata = (BYTE*)av_mallocz(extradata_len);
	BYTE* body = (BYTE*)pCodecCtx->extradata;

	//H264 AVCC 格式的extradata头，用来存储 SPS，PPS
	int i = 0;
	body[i++] = 0x01;
	body[i++] = sps_buffer[1];
	body[i++] = sps_buffer[2];
	body[i++] = sps_buffer[3];
	body[i++] = 0xff;

	//// SPS 
	body[i++] = 0xe1;
	body[i++] = (sizeof(sps_buffer) >> 8) & 0xff;
	body[i++] = sizeof(sps_buffer) & 0xff;
	memcpy(&body[i], sps_buffer, sizeof(sps_buffer));
	i += sizeof(sps_buffer);

	/// PPS
	body[i++] = 0x01;
	body[i++] = (sizeof(pps_buffer) >> 8) & 0xff;
	body[i++] = (sizeof(pps_buffer)) & 0xff;
	memcpy(&body[i], pps_buffer, sizeof(pps_buffer));

	if (pCodecCtx->flags & AVFMT_GLOBALHEADER)
	{
		pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
		printf("has global flag header\n");
	}
}

x264Encoder::x264Encoder(int width2, int height2, const char * filename2)
{
	if (filename2 != NULL)
		strcpy(filename, filename2);
	width = width2;
	height = height2;
	init();
}

x264Encoder::~x264Encoder()
{
}

void mpeg1Encoder::init()
{
	if (filename != NULL)
		fp_out = fopen(filename, "wb");

	pCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
	if (pCodec == NULL)
	{
		printf("find mpeg1 codec failed.\r\n");
		return;
	}


	pCodecCtx = avcodec_alloc_context3(pCodec);
	pCodecCtx->dct_algo = FF_DCT_FASTINT;
	pCodecCtx->bit_rate = 400000;
	pCodecCtx->width = width;
	pCodecCtx->height = height;
	AVRational avrational = { 1,24 };
	pCodecCtx->time_base = avrational;
	pCodecCtx->gop_size = 10;
	pCodecCtx->max_b_frames = 0;
	pCodecCtx->thread_count = 1;
	pCodecCtx->pix_fmt = PIX_FMT_YUV420P;

	int nret = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (nret < 0)
	{
		printf("open mpeg codec failed.\r\n");
		return;
	}
}

mpeg1Encoder::mpeg1Encoder(int widht2, int height2, const char * filename2)
{
	if (filename2 != NULL)
		strcpy(filename, filename2);
	width = widht2;
	height = height2;
	init();
}

mpeg1Encoder::~mpeg1Encoder()
{
}
