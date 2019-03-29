#include <WinSock.h>
#include <vector>
#include <string>
#include <setjmp.h>
#include <jpeglib.h>
#pragma comment(lib,"turbojpeg.lib")
using namespace std;

#include "web_stream.h"


#pragma comment(lib,"ws2_32.lib")

const char* boundary = "++&&**boundary--stream-jpeg++";

web_stream::web_stream()
{
	::InitializeCriticalSection(&cs);
	listenfd = -1;
	quit = false;
	jpeg_quality = 80;
}

web_stream::~web_stream()
{
	if (listenfd) {
		closesocket(listenfd); 
	}
	::DeleteCriticalSection(&cs);
}

int web_stream::start(const char* strip, int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		return -1;
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (strip) {
		addr.sin_addr.S_un.S_addr = inet_addr(strip);
	}
	addr.sin_port = htons(port);
	if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
		closesocket(fd);
		return -1;
	}
	this->listenfd = fd;
	listen(fd, 5);
	DWORD tid;
	HANDLE h = CreateThread(0, 0, accept_thread, this, 0, &tid);
	CloseHandle(h);
	return 0;
}

struct web_client_t
{
	int s;
	web_stream* ws;
};
DWORD CALLBACK web_stream::accept_thread(void* _p)
{
	web_stream* ws = (web_stream*)_p;
	while (!ws->quit) {
		fd_set rdst; FD_ZERO(&rdst);  FD_SET(ws->listenfd, &rdst);
		timeval tmo; tmo.tv_sec = 1; tmo.tv_usec = 0;
		int r = select(0, &rdst, NULL, NULL, &tmo);
		if (r <= 0)continue;
		if (ws->quit)break;
		int s = accept(ws->listenfd, NULL, NULL); if (s < 0)continue;
	
		int tcp_nodelay = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&tcp_nodelay, sizeof(int));
	
		DWORD tid;
		web_client_t* p = new web_client_t;
		p->s = s; p->ws = ws;
		CloseHandle(CreateThread(0, 0, client_thread, (void*)p, 0, &tid));

	}

	return 0;
}
DWORD CALLBACK web_stream::client_thread(void* _p)
{
	web_client_t* p = (web_client_t*)_p;
	web_stream* ws = p->ws;
	int s = p->s;

	char buffer[8192];
	bool quit = false;

	int pos = 0; int LEN = sizeof(buffer);
	while (pos < LEN - 1) { //只能处理 GET 命令
		int r = recv(s, buffer + pos, LEN - pos, 0);
		if (r <= 0) { quit = true; break; }
		pos += r;
		buffer[pos] = 0;
		if (strstr(buffer, "\r\n\r\n"))break;
	}
	if (quit) {
		closesocket(s); delete p;
		return 0;
	}

	buffer[pos] = 0; printf("GET Info [%s]\n", buffer);

	const char* response = "HTTP/1.0 200 OK\r\n"
		"Connection: close\r\n"
		"Server: Web-Stream\r\n"
		"Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n"
		"Pragma: no-cache\r\n"
		"Content-Type: multipart/x-mixed-replace;boundary=%s\r\n\r\n";
	sprintf(buffer, response, boundary);
	send(s, buffer, strlen(buffer), 0);

	::EnterCriticalSection(&ws->cs);
	ws->socks.push_back(s);
	::LeaveCriticalSection(&ws->cs);
	
	while (!ws->quit) {
		int r = recv(s, buffer, sizeof(buffer), 0);
		if (r <= 0)break;///
	}
	
	/////
	::EnterCriticalSection(&ws->cs);
	for (vector<int>::iterator it = ws->socks.begin(); it != ws->socks.end(); it != ws->socks.end()) {
		if (*it == s) {
			ws->socks.erase(it); break;
		}
	}
	::LeaveCriticalSection(&ws->cs);
	closesocket(s);

	delete p;
	return 0;
}

struct jpeg_error_t {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};
static void jpeg_error_exit(j_common_ptr cinfo)
{
	jpeg_error_t* myerr = (jpeg_error_t*)cinfo->err;
	////
	(*cinfo->err->output_message) (cinfo);

	longjmp(myerr->setjmp_buffer, 1);
}

void web_stream::frame(dp_frame_t* frame)
{
	vector<int> socks;
	::EnterCriticalSection(&cs);
	socks = this->socks;
	::LeaveCriticalSection(&cs);

	if (socks.size() == 0)return;

	if (frame->bitcount == 8)return; // 8位色不处理

	unsigned char* line_data = NULL;
	struct jpeg_compress_struct cinfo; //
	memset(&cinfo, 0, sizeof(cinfo)); //全部初始化为0， 否则要出问题

	struct jpeg_error_t jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpeg_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		////
		jpeg_destroy_compress(&cinfo);
		if (line_data)free(line_data); 
		return ;
	}
	jpeg_create_compress(&cinfo);

	byte* out_ptr = NULL;
	unsigned long out_size = 0;

	jpeg_mem_dest(&cinfo, &out_ptr, (unsigned long*)&out_size );
	cinfo.image_width = frame->cx;
	cinfo.image_height = frame->cy;

	int bit = frame->bitcount;
	if (bit == 16) {
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_EXT_BGR; 
		line_data = (byte*)malloc(cinfo.image_width * 3); /// BGR data
	}
	else {
		cinfo.input_components = bit / 8; 
		if (bit == 32) cinfo.in_color_space = JCS_EXT_BGRA;     
		else if (bit == 24) cinfo.in_color_space = JCS_EXT_BGR;
	}
	jpeg_set_defaults(&cinfo); 
	
	int quality = jpeg_quality; ////压缩质量

	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	while (cinfo.next_scanline < cinfo.image_height) {
		if (bit == 16) {
			WORD* dat = (WORD*)(frame->buffer + cinfo.next_scanline * frame->line_stride );
			int k = 0;
			for (int i = 0; i < cinfo.image_width; ++i) {
				WORD bb = dat[i];
				line_data[k] = ((bb & 0x1F) << 3);          // b
				line_data[k + 1] = ((bb >> 5) & 0x1F) << 3; // g
				line_data[k + 2] = ((bb >> 10) & 0x1F) << 3; // r
															 ///
				k += 3; ///
			}
			jpeg_write_scanlines(&cinfo, &line_data, 1);
			////
		}
		else {
			byte* line = (byte*)frame->buffer + cinfo.next_scanline * frame->line_stride; ///
			jpeg_write_scanlines(&cinfo, &line, 1);
		}
	}

	jpeg_finish_compress(&cinfo);

	jpeg_destroy_compress(&cinfo);
	if (line_data)free(line_data);
	
	char hdr[4096];
	sprintf(hdr, "--%s\r\n"
		"Content-Type: image/jpeg\r\n"
		"Content-Length: %d\r\n\r\n",
		boundary, out_size);

	for (int i = 0; i < socks.size(); ++i) {
		int s = socks[i];
		int r = send(s, hdr, strlen(hdr), 0);

		r = send(s, (char*)out_ptr, out_size, 0); 
		if (r <= 0) {
			::EnterCriticalSection(&this->cs);
			for (vector<int>::iterator it = this->socks.begin(); it != this->socks.end(); it != this->socks.end()) {
				if (*it == s) {
					this->socks.erase(it); break;
				}
			}
			::LeaveCriticalSection(&this->cs);
			closesocket(s);
		}
	
	}
	free(out_ptr); 
}

