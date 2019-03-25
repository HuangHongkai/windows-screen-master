#pragma once
/*
实现一个播放器
*/
#include "ffmpeg.h"
extern "C"
{
	#include "SDL/SDL.h"
}

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

namespace SDLMaster
{
	static int thread_exit;
	static SDL_Rect rect;
	static SDL_Overlay *bmp;
	const int SCREEN_W = 1000, SCREEN_H = 800;

	int static sfp_refresh_thread(LPVOID)
	{

		thread_exit = 0;
		while (!thread_exit) 
		{
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
			SDL_Delay(40);
		}
		return 0;
	}

	void static init(int width, int height)
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
		{
			printf("Could not initialize SDL - %s\n", SDL_GetError());
		}
		
		int screen_w = width, screen_h = height;
		SDL_Surface *screen;
		screen = SDL_SetVideoMode(screen_w, screen_h, 0, 0);
		if (!screen) 
		{
			printf("SDL: could not set video mode - exiting:%s\n", SDL_GetError());
			return ;
		}
		bmp = SDL_CreateYUVOverlay(width, height, SDL_YV12_OVERLAY, screen);
		rect.x = 0;
		rect.y = 0;
		rect.w = screen_w;
		rect.h = screen_h;

		SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread, NULL);
		SDL_WM_SetCaption("Desktop Player", NULL);
	}

	void static updateScreen(AVFrame *pFrameYUV)
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT)
		{
			SDL_LockYUVOverlay(bmp);
			// y u v
			bmp->pixels[0] = pFrameYUV->data[0];
			bmp->pixels[2] = pFrameYUV->data[1];
			bmp->pixels[1] = pFrameYUV->data[2];
			bmp->pitches[0] = pFrameYUV->linesize[0];
			bmp->pitches[2] = pFrameYUV->linesize[1];
			bmp->pitches[1] = pFrameYUV->linesize[2];
			SDL_UnlockYUVOverlay(bmp);
			SDL_DisplayYUVOverlay(bmp, &rect);
		}
		else if (event.type == SDL_QUIT) 
		{
			printf("quit\n");
			thread_exit = 1;
		}
	}
};

