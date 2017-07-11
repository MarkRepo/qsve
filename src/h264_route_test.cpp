#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <cstddef>

#include "qsvenc.h"

#define SEC_TO_NANO_ULL(sec)    ((unsigned long long)sec * 1000000000)
#define MICRO_TO_NANO_ULL(sec)  ((unsigned long long)sec * 1000)

inline bool NvQueryPerformanceFrequency(unsigned long long *freq)
{
    *freq = 0;
    *freq = 1000000000;
    return true;
}

inline bool NvQueryPerformanceCounter(unsigned long long *counter)
{
    *counter = 0;
    struct timeval tv;
    int ret;
    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        return false;
    }

    *counter = SEC_TO_NANO_ULL(tv.tv_sec) + MICRO_TO_NANO_ULL(tv.tv_usec);
    return true;
}


int main(int argc, char* argv[])
{
//generate r g b file of one frame
#if 0
	FILE* r_file = fopen("r.rgb32", "wb+");
	FILE* g_file = fopen("g.rgb32", "wb+");
	FILE* b_file = fopen("b.rgb32", "wb+");
	FILE* bg_file = fopen("bg.rgb32", "wb+");
	FILE* br_file = fopen("br.rgb32", "wb+");
	FILE* gr_file = fopen("gr.rgb32", "wb+");
	FILE* bgr_file = fopen("bgr.rgb32", "wb+");

	unsigned int b = 0xff, g = 0xff00, r = 0xff0000, bg = 0xffff, br = 0xff00ff, gr = 0xffff00, bgr=0xffffff;
	for(int i = 0; i< 1920*1080; i++){
		fwrite(&r, 1, sizeof(r), r_file);
		fwrite(&g, 1, sizeof(g), g_file);
		fwrite(&b, 1, sizeof(b), b_file);
		fwrite(&bg, 1, sizeof(bg), bg_file);
		fwrite(&br, 1, sizeof(br), br_file);
		fwrite(&gr, 1, sizeof(gr), gr_file);
		fwrite(&bgr, 1, sizeof(bgr), bgr_file);
	}
	return 0;
#endif
	int result = 0;
	ENCODE_TYPE encode_type = ENCODE_YV12;
	if(strncmp(argv[1], "rgb", 3) == 0)
		encode_type = ENCODE_RGB4;
	else if(strncmp(argv[1], "yuv", 3) == 0)
		encode_type = ENCODE_YV12;
	
	//printf("qsve_init start\n");
	int width  = atoi(argv[4]);
	int height = atoi(argv[5]);
	int bitRate = atoi(argv[7]);
	int targetUsage = atoi(argv[8]);
	int frameSize = 0;
	if(encode_type == ENCODE_RGB4)
		frameSize = width*height*4;
	else if(encode_type == ENCODE_YV12)
		frameSize = width*height*3/2;
	
	int pNum   = atoi(argv[6]);
	pid_t pid = 0;
	while(pNum > 0){
		pid = fork();
		if(pid<0){
			printf("fork error.\n");
			break;
		}
		else if(pid == 0)
			break;
		else{
			pNum--;
		}
	}
	if(pid < 0)
		exit(1);

	unsigned long long eStart, eEnd, Freq;
	NvQueryPerformanceFrequency(&Freq);
	NvQueryPerformanceCounter(&eStart);
	result  = qsve_init(0, encode_type);
	if(result != 0){
		printf("qsve_init failed. result: %d\n", result);
		return -1;
	}
	TestParams params;
	params.bitRate = (unsigned short)bitRate;
	params.targetUsage = (unsigned short)targetUsage;
	qsve_set_params(&params);
	//printf("qsve_init success!\n");
	//printf("qsve_create_mem start\n");
	result = qsve_create_mem(width*4,height,0);
	if(result != 0){
		printf("qsve_create_mem failed, result: %d\n", result);
		return result;
	}
	//printf("qsve_create_mem success \n");

	int nBytesRead = 0;
	char dst_file_name[128]={0};
	snprintf(dst_file_name, 128, "%s_%d.h264", argv[3], getpid());
	FILE* srcFile = fopen(argv[2], "rb");
	FILE* dstFile = fopen(dst_file_name, "wb+");
	//void* pSurf = NULL;
	void* pBitStreamBuf = NULL;
	size_t   pBitStreamLen = 0;
	char* pBuffer = (char*)malloc(frameSize);
	memset(pBuffer, 0, frameSize);
	int frames = 0;
	//int times = 0;
	while(1)
	{	
		nBytesRead = fread(pBuffer, 1, frameSize, srcFile);
		if(nBytesRead != frameSize)
			break;
	#if 0
		if(nBytesRead != frameSize && times< 20)
		{
			fseek(srcFile, 0, SEEK_SET);
			times++;
			nBytesRead = fread(pBuffer, 1, frameSize, srcFile);
		}
		else if(nBytesRead != frameSize)
			break;
	#endif
		result = qsve_encode_frame(0, pBuffer, &pBitStreamBuf,&pBitStreamLen, 0);
		if(result != 0){
			printf("qsve_encode_frame failed. result : %d\n", result);
			return result;
		}
		//if(frames >= 5)
		fwrite(pBitStreamBuf, 1, pBitStreamLen, dstFile);
		frames++;
	}
	qsve_release();
	NvQueryPerformanceCounter(&eEnd);
	double elapsTime = (double)(eEnd-eStart);
	printf("elapsTime: %6.2f ms\n", elapsTime*1000.0/Freq);
	return 0;	
}
