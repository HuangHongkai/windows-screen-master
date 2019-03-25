#include "ffmpeg.h"

void ffmpeg_init()
{
	av_register_all();
	avformat_network_init();
	avdevice_register_all();
}