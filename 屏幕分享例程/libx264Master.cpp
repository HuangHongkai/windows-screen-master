#include "libx264Master.h"

void libx264Master::init()
{
	fp_dst = fopen(filename, "wb");

	//Encode 50 frame
	//if set 0, encode all frame
	frame_num = 50;
	csp = X264_CSP_I420;

	iNal = 0;
	pNals = NULL;
	pHandle = NULL;
	pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	pParam = (x264_param_t*)malloc(sizeof(x264_param_t));

	x264_param_default(pParam);
	pParam->i_width = width;
	pParam->i_height = height;
	pParam->i_csp = csp;

	
	//Param
	pParam->i_log_level  = X264_LOG_DEBUG;
	pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
	pParam->i_frame_total = 0;
	pParam->i_keyint_max = 10;
	pParam->i_bframe  = 5;
	pParam->b_open_gop  = 0;
	pParam->i_bframe_pyramid = 0;
	pParam->rc.i_qp_constant=0;
	pParam->rc.i_qp_max=0;
	pParam->rc.i_qp_min=0;
	pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	pParam->i_fps_den  = 1;
	pParam->i_fps_num  = 25;
	pParam->i_timebase_den = pParam->i_fps_num;
	pParam->i_timebase_num = pParam->i_fps_den;
	
	x264_param_apply_profile(pParam, x264_profile_names[5]);

	pHandle = x264_encoder_open(pParam);

	x264_picture_init(pPic_out);
	x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);

	//ret = x264_encoder_headers(pHandle, &pNals, &iNal);

}

bool libx264Master::encode(AVFrame * pFrameYUV)
{
	static int i = 0;
	switch (csp) {
	case X264_CSP_I444: {
		memcpy(pPic_in->img.plane[0], pFrameYUV->data[0], y_size); // Y
		memcpy(pPic_in->img.plane[1], pFrameYUV->data[1], y_size); // U
		memcpy(pPic_in->img.plane[2], pFrameYUV->data[2], y_size); // V
		break; }
	case X264_CSP_I420: {
		memcpy(pPic_in->img.plane[0], pFrameYUV->data[0], y_size); // Y
		memcpy(pPic_in->img.plane[1], pFrameYUV->data[1], y_size / 4); // U
		memcpy(pPic_in->img.plane[2], pFrameYUV->data[2], y_size / 4); // V
		break; }
	default: {
		printf("Colorspace Not Support.\n");
		return false; }
	}
	pPic_in->i_pts = i;
	i++;
	int ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
	if (ret < 0) {
		printf("Error.\n");
		return false;
	}

	printf("Succeed encode frame: %5d\n", i);

	if(fp_dst != NULL)
		for (int j = 0; j < iNal; ++j) {
			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
		}

	return true;
}

bool libx264Master::flush()
{
	//flush encoder
	int i = 0;
	while (1) {
		int ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);
		if (ret == 0) {
			break;
		}
		printf("Flush 1 frame.\n");
		for (int j = 0; j < iNal; ++j) {
			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
		}
		i++;
	}
	x264_picture_clean(pPic_in);
	x264_encoder_close(pHandle);
	pHandle = NULL;

	free(pPic_in);
	free(pPic_out);
	free(pParam);


	fclose(fp_dst);
	return true;
}

libx264Master::libx264Master(int nWidth, int nHeight, const char * filepath)
{
	if (filepath != NULL)
		strcpy(filename, filepath);
	width = nWidth;
	height = nHeight;
	y_size = width * height;
	init();
}

libx264Master::~libx264Master()
{
}

