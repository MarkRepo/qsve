#include "qsvenc.h"
#include "qsv_h264.h"
#include "qsv_log.h"
#include <unistd.h>
#include <stdlib.h>
#include <cstddef>

CEncodingVmemVpp* gVppEncoder = NULL; // 如果是全局变量，析构的时候allocator会出崩溃

int    qsve_init(int max_number_surface, ENCODE_TYPE encode_type)
{
	gVppEncoder = new CEncodingVmemVpp;
	if(!gVppEncoder)
		return -1;
	char log_file_name[128] = {0};
	snprintf(log_file_name, 127, "%s_%d.log", gLogFileName, getpid());
	H264_LogInitFile(log_file_name);
	mfxStatus sts = MFX_ERR_NONE;
    CmdOptions options;
	initCmdOptions(&options);
    gVppEncoder->Init(&options);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	return 0;
}

int qsve_set_params(TestParams * params)
{
	gVppEncoder->SetTestParams(params);
	return 0;
}

int    qsve_create_mem(int stride, int height, int id)
{
	int wstride = stride;
	if(stride < 0)
		wstride  *= -1;
	return gVppEncoder->ResetMFXResolution(wstride/4,height);
}

int    qsve_destroy_mem(int id)
{
	return 0;
}

int	  qsve_encode_frame(int id, const void* pInputBuffer, void** pBitStreamBuffer , size_t* pBitStreamLen, int flags)
{
	mfxStatus sts = MFX_ERR_NONE;
	sts = gVppEncoder->EncodeFrame(pInputBuffer, pBitStreamBuffer, pBitStreamLen, flags);
	if(sts != MFX_ERR_NONE)
		return -1;
	return 0;
}

void   qsve_release()
{
	if(gVppEncoder)
		delete gVppEncoder;
	gVppEncoder = NULL;
}

