#include "rgb2yuv.h"

YUVencoder::~YUVencoder()
{
	if (avctx)
	{
		avcodec_close(avctx);
	}
	if (avframe)
	{
		//av_free(avframe->data[0]);
		av_frame_free(&avframe);
	}
}

YUVencoder::YUVencoder(int w, int h)
{
	width = w;
	height = h;

	AVFrame* frame = av_frame_alloc();
	if (!frame) 
		return;
	
	frame->width = w;
	frame->height = h;
	frame->format = AV_PIX_FMT_YUV420P;
	frame->pts = 0;

	unsigned char *out_buffer = (unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, width, height));
	avpicture_fill((AVPicture *)frame, out_buffer, AV_PIX_FMT_YUV420P, width, height);

	this->avframe = frame;
}

AVFrame * YUVencoder::encode(unsigned char * rgb32, int rgb32_len, int width, int height, unsigned char * out, int out_len)
{
	SwsContext* sws_ctx = NULL;
	AVPixelFormat fmt = AV_PIX_FMT_RGB32, fmt2 = AV_PIX_FMT_YUV420P;
	sws_ctx = sws_getContext(width, height, fmt, width, height, fmt2, SWS_BILINEAR, NULL, NULL, NULL);
	if (!sws_ctx)return NULL;
	AVPicture spic;
	int r1 = avpicture_fill(&spic, (uint8_t*)rgb32, fmt, width, height);
	int ret = sws_scale(sws_ctx, spic.data, spic.linesize, 0, height, this->avframe->data, this->avframe->linesize);
	sws_freeContext(sws_ctx);
	static int npts = 0;
	avframe->pts = npts ++ ;
	return avframe;
}
