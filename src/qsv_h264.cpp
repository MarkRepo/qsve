#include "common_utils.h"
#include "qsv_h264.h"
#include "qsv_log.h"
#include <unistd.h>
#include <stdlib.h>

#define _USE_IPP_
#ifdef _USE_IPP_
#include "ippcore.h"
#include "ippcc.h"
#endif
FILE* gNV12File = NULL;
CEncodingVmemVpp::CEncodingVmemVpp()
{
	m_fourcc        		= MFX_FOURCC_RGB4;   //default rgb4 , can set to mfx_fourcc_yv12
	m_nSurfNumVPPIn 		= 0;
	m_nSurfNumVPPOutEnc 	= 0;
	m_pMfxAllocator 		= NULL;
	m_pMfxEnc       		= NULL;
	m_pMfxVpp       		= NULL;
	m_pMfxSurfacesVPPIn     = NULL;
	m_pVPPSurfacesVPPOutEnc = NULL;
}

CEncodingVmemVpp::~CEncodingVmemVpp()
{
    MSDK_SAFE_DELETE_ARRAY(m_pMfxSurfacesVPPIn);
    MSDK_SAFE_DELETE_ARRAY(m_pVPPSurfacesVPPOutEnc);
	
    MSDK_SAFE_DELETE_ARRAY(m_mfxBS.Data);
    MSDK_SAFE_DELETE_ARRAY(m_extDoNotUse.AlgList);

    m_pMfxAllocator->Free(m_pMfxAllocator->pthis, &m_mfxResponseVPPIn);
    m_pMfxAllocator->Free(m_pMfxAllocator->pthis, &m_mfxResponseVPPOutEnc);
	Release();
	
	MSDK_SAFE_DELETE(m_pMfxEnc);
	MSDK_SAFE_DELETE(m_pMfxVpp);
	MSDK_SAFE_FREE(m_pMfxAllocator);
}

mfxStatus CEncodingVmemVpp::Init(CmdOptions* options)
{
//	gNV12File = fopen("/home/thinputer/Downloads/libQsve/surface_nv12.yuv", "wb+");
	mfxStatus sts = MFX_ERR_NONE;
	sts = InitSessionAndSetAllocator(options);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	
	InitEncParams(options);
	m_pMfxEnc = new MFXVideoENCODE(m_session);
	MSDK_CHECK_POINTER(m_pMfxEnc, MFX_ERR_MEMORY_ALLOC);
	
#ifdef _USE_IPP_
	ippInit();
#else
	InitVppParams(options);
	m_pMfxVpp = new MFXVideoVPP(m_session);
	MSDK_CHECK_POINTER(m_pMfxVpp, MFX_ERR_MEMORY_ALLOC);
#endif	

	return MFX_ERR_NONE;
}

mfxStatus CEncodingVmemVpp::InitSessionAndSetAllocator(CmdOptions * options)
{
	mfxStatus sts = MFX_ERR_NONE;
	mfxVersion ver = { {0, 1} };
	mfxIMPL impl = options->impl;
	m_pMfxAllocator = (mfxFrameAllocator*)malloc(sizeof(mfxFrameAllocator));
	MSDK_CHECK_POINTER(m_pMfxAllocator, MFX_ERR_MEMORY_ALLOC);
	sts = Initialize(impl, ver, &m_session, m_pMfxAllocator);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	return MFX_ERR_NONE;
}

void CEncodingVmemVpp::SetTestParams(TestParams * params)
{
	m_mfxEncParams.mfx.TargetKbps  = params->bitRate;
	m_mfxEncParams.mfx.TargetUsage = params->targetUsage;
}

void CEncodingVmemVpp::InitEncParams(CmdOptions * options)
{
	memset(&m_mfxEncParams, 0, sizeof(m_mfxEncParams));
    m_mfxEncParams.mfx.CodecId = MFX_CODEC_AVC;
    m_mfxEncParams.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
    m_mfxEncParams.mfx.TargetKbps = options->Bitrate;
    m_mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
    m_mfxEncParams.mfx.FrameInfo.FrameRateExtN = options->FrameRateN;
    m_mfxEncParams.mfx.FrameInfo.FrameRateExtD = options->FrameRateD;
    m_mfxEncParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
    m_mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    m_mfxEncParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    m_mfxEncParams.mfx.FrameInfo.CropX = 0;
    m_mfxEncParams.mfx.FrameInfo.CropY = 0;
    m_mfxEncParams.mfx.FrameInfo.CropW = options->Width;
    m_mfxEncParams.mfx.FrameInfo.CropH = options->Height;
    m_mfxEncParams.mfx.FrameInfo.Width = MSDK_ALIGN16(options->Width);
    m_mfxEncParams.mfx.FrameInfo.Height =
        (MFX_PICSTRUCT_PROGRESSIVE == m_mfxEncParams.mfx.FrameInfo.PicStruct) ?
        MSDK_ALIGN16(options->Height) :
        MSDK_ALIGN32(options->Height);
	m_mfxEncParams.mfx.GopRefDist = 1;
	m_mfxEncParams.AsyncDepth = 1;
	m_mfxEncParams.mfx.NumRefFrame = 1;
    m_mfxEncParams.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY;
}

void CEncodingVmemVpp::InitVppParams(CmdOptions * options)
{
	memset(&m_mfxVppParams, 0, sizeof(m_mfxVppParams));
    // Input params
    m_mfxVppParams.vpp.In.FourCC = MFX_FOURCC_RGB4;
    m_mfxVppParams.vpp.In.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    m_mfxVppParams.vpp.In.CropX = 0;
    m_mfxVppParams.vpp.In.CropY = 0;
    m_mfxVppParams.vpp.In.CropW = options->Width;
    m_mfxVppParams.vpp.In.CropH = options->Height;
    m_mfxVppParams.vpp.In.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    m_mfxVppParams.vpp.In.FrameRateExtN = options->FrameRateN;
    m_mfxVppParams.vpp.In.FrameRateExtD = options->FrameRateD;
    m_mfxVppParams.vpp.In.Width = MSDK_ALIGN16(options->Width);
    m_mfxVppParams.vpp.In.Height =
        (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVppParams.vpp.In.PicStruct) ?
        MSDK_ALIGN16(options->Height) :
        MSDK_ALIGN32(options->Height);

	// Output params
    m_mfxVppParams.vpp.Out.FourCC = MFX_FOURCC_NV12;
    m_mfxVppParams.vpp.Out.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    m_mfxVppParams.vpp.Out.CropX = 0;
    m_mfxVppParams.vpp.Out.CropY = 0;
    m_mfxVppParams.vpp.Out.CropW = options->Width;
    m_mfxVppParams.vpp.Out.CropH = options->Height;
    m_mfxVppParams.vpp.Out.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    m_mfxVppParams.vpp.Out.FrameRateExtN = options->FrameRateN;
    m_mfxVppParams.vpp.Out.FrameRateExtD = options->FrameRateD;
    m_mfxVppParams.vpp.Out.Width = MSDK_ALIGN16(m_mfxVppParams.vpp.Out.CropW);
    m_mfxVppParams.vpp.Out.Height =
        (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVppParams.vpp.Out.PicStruct) ?
        MSDK_ALIGN16(m_mfxVppParams.vpp.Out.CropH) :
        MSDK_ALIGN32(m_mfxVppParams.vpp.Out.CropH);

	m_mfxVppParams.AsyncDepth = 1;

    m_mfxVppParams.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
}

mfxStatus  CEncodingVmemVpp::AllocFrameBuffers()
{
	mfxStatus sts = MFX_ERR_NONE;
	
	mfxFrameAllocRequest EncRequest;
    memset(&EncRequest, 0, sizeof(EncRequest));
    sts = m_pMfxEnc->QueryIOSurf(&m_mfxEncParams, &EncRequest);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	m_nSurfNumVPPOutEnc = EncRequest.NumFrameSuggested;

#ifndef _USE_IPP_
    // Query number of required surfaces for VPP
    mfxFrameAllocRequest VPPRequest[2];     // [0] - in, [1] - out
    memset(&VPPRequest, 0, sizeof(mfxFrameAllocRequest) * 2);
    sts = m_pMfxVpp->QueryIOSurf(&m_mfxVppParams, VPPRequest);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    EncRequest.Type |= MFX_MEMTYPE_FROM_VPPOUT;     // surfaces are shared between VPP output and encode input

    // Determine the required number of surfaces for VPP input and for VPP output (encoder input)
    m_nSurfNumVPPIn = VPPRequest[0].NumFrameSuggested;
    m_nSurfNumVPPOutEnc = EncRequest.NumFrameSuggested + VPPRequest[1].NumFrameSuggested;

    sts = m_pMfxAllocator->Alloc(m_pMfxAllocator->pthis, &VPPRequest[0], &m_mfxResponseVPPIn);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    m_pMfxSurfacesVPPIn = new mfxFrameSurface1[m_nSurfNumVPPIn];
    MSDK_CHECK_POINTER(m_pMfxSurfacesVPPIn, MFX_ERR_MEMORY_ALLOC);
    for (int i = 0; i < m_nSurfNumVPPIn; i++) {
        memset(&m_pMfxSurfacesVPPIn[i], 0, sizeof(mfxFrameSurface1));
        memcpy(&(m_pMfxSurfacesVPPIn[i].Info), &(m_mfxVppParams.vpp.In), sizeof(mfxFrameInfo));
        m_pMfxSurfacesVPPIn[i].Data.MemId = m_mfxResponseVPPIn.mids[i];
    }

	// Disable default VPP operations
    memset(&m_extDoNotUse, 0, sizeof(mfxExtVPPDoNotUse));
    m_extDoNotUse.Header.BufferId = MFX_EXTBUFF_VPP_DONOTUSE;
    m_extDoNotUse.Header.BufferSz = sizeof(mfxExtVPPDoNotUse);
    m_extDoNotUse.NumAlg = 4;
    m_extDoNotUse.AlgList = new mfxU32[m_extDoNotUse.NumAlg];
    MSDK_CHECK_POINTER(m_extDoNotUse.AlgList, MFX_ERR_MEMORY_ALLOC);
    m_extDoNotUse.AlgList[0] = MFX_EXTBUFF_VPP_DENOISE;       // turn off denoising (on by default)
    m_extDoNotUse.AlgList[1] = MFX_EXTBUFF_VPP_SCENE_ANALYSIS;        // turn off scene analysis (on by default)
    m_extDoNotUse.AlgList[2] = MFX_EXTBUFF_VPP_DETAIL;        // turn off detail enhancement (on by default)
    m_extDoNotUse.AlgList[3] = MFX_EXTBUFF_VPP_PROCAMP;       // turn off processing amplified (on by default)

    // Add extended VPP buffers
    
	mfxExtVPPVideoSignalInfo video_signal_info;
	memset(&video_signal_info, 0, sizeof(video_signal_info));
    video_signal_info.Header.BufferId = MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO;
    video_signal_info.Header.BufferSz = sizeof(mfxExtVPPVideoSignalInfo);
    video_signal_info.In.TransferMatrix = MFX_TRANSFERMATRIX_BT709;
    video_signal_info.In.NominalRange = MFX_NOMINALRANGE_0_255;
	video_signal_info.Out.TransferMatrix = MFX_TRANSFERMATRIX_BT709;
    video_signal_info.Out.NominalRange = MFX_NOMINALRANGE_0_255;
	
    mfxExtBuffer* extBuffers[2];
    extBuffers[0] = (mfxExtBuffer*) & m_extDoNotUse;
	extBuffers[1] = (mfxExtBuffer*) & video_signal_info;
    m_mfxVppParams.ExtParam = extBuffers;
    m_mfxVppParams.NumExtParam = 1;
	
#endif
	EncRequest.NumFrameSuggested = m_nSurfNumVPPOutEnc;
	sts = m_pMfxAllocator->Alloc(m_pMfxAllocator->pthis, &EncRequest, &m_mfxResponseVPPOutEnc);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    m_pVPPSurfacesVPPOutEnc = new mfxFrameSurface1[m_nSurfNumVPPOutEnc];
    MSDK_CHECK_POINTER(m_pVPPSurfacesVPPOutEnc, MFX_ERR_MEMORY_ALLOC);
    for (int i = 0; i < m_nSurfNumVPPOutEnc; i++) {
        memset(&m_pVPPSurfacesVPPOutEnc[i], 0, sizeof(mfxFrameSurface1));
        memcpy(&(m_pVPPSurfacesVPPOutEnc[i].Info), &(m_mfxVppParams.vpp.Out), sizeof(mfxFrameInfo));
        m_pVPPSurfacesVPPOutEnc[i].Data.MemId = m_mfxResponseVPPOutEnc.mids[i];
    }

	sts = m_pMfxEnc->Init(&m_mfxEncParams);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

#ifndef _USE_IPP_
    // Initialize Media SDK VPP
    sts = m_pMfxVpp->Init(&m_mfxVppParams);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#endif

    // Retrieve video parameters selected by encoder.
    // - BufferSizeInKB parameter is required to set bit stream buffer size
    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = m_pMfxEnc->GetVideoParam(&par);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Prepare Media SDK bit stream buffer
    memset(&m_mfxBS, 0, sizeof(m_mfxBS));
    m_mfxBS.MaxLength = par.mfx.BufferSizeInKB * 1000;
    m_mfxBS.Data = new mfxU8[m_mfxBS.MaxLength];
    MSDK_CHECK_POINTER(m_mfxBS.Data, MFX_ERR_MEMORY_ALLOC);
	return MFX_ERR_NONE;
}

mfxStatus CEncodingVmemVpp::EncodeFrame(const void* pInputBuffer, void** pBitStreamBuf, size_t* pBitStremBufLen, int flags)
{
	mfxStatus sts = MFX_ERR_NONE;
	int nEncSurfIdx = 0;
    mfxSyncPoint  syncpEnc;
	static int frames= 0;
	
#ifndef _USE_IPP_
	int nVPPSurfIdx = 0;
	mfxSyncPoint syncpVPP;

	nVPPSurfIdx = GetFreeSurfaceIndex(m_pMfxSurfacesVPPIn, m_nSurfNumVPPIn);    // Find free input frame surface
    MSDK_CHECK_ERROR(MFX_ERR_NOT_FOUND, nVPPSurfIdx, MFX_ERR_MEMORY_ALLOC);

    // Surface locking required when read/write video surfaces
    sts = m_pMfxAllocator->Lock(m_pMfxAllocator->pthis, m_pMfxSurfacesVPPIn[nVPPSurfIdx].Data.MemId, &(m_pMfxSurfacesVPPIn[nVPPSurfIdx].Data));
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = LoadRawRGBFrame(&m_pMfxSurfacesVPPIn[nVPPSurfIdx], (char*)pInputBuffer);  // Load frame from file into surface
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = m_pMfxAllocator->Unlock(m_pMfxAllocator->pthis, m_pMfxSurfacesVPPIn[nVPPSurfIdx].Data.MemId, &(m_pMfxSurfacesVPPIn[nVPPSurfIdx].Data));
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    nEncSurfIdx = GetFreeSurfaceIndex(m_pVPPSurfacesVPPOutEnc, m_nSurfNumVPPOutEnc);    // Find free output frame surface
    MSDK_CHECK_ERROR(MFX_ERR_NOT_FOUND, nEncSurfIdx, MFX_ERR_MEMORY_ALLOC);

    for (;;) {
        // Process a frame asychronously (returns immediately)
        sts = m_pMfxVpp->RunFrameVPPAsync(&m_pMfxSurfacesVPPIn[nVPPSurfIdx], &m_pVPPSurfacesVPPOutEnc[nEncSurfIdx], NULL, &syncpVPP);
        if (MFX_WRN_DEVICE_BUSY == sts) {
            MSDK_SLEEP(1);  // Wait if device is busy, then repeat the same call
        } else
            break;
    }
	
    if (MFX_ERR_MORE_DATA == sts)
        NvLog("VPPAsync MFX_ERR_MORE_DATA");

    // MFX_ERR_MORE_SURFACE means output is ready but need more surface (example: Frame Rate Conversion 30->60)
    // * Not handled in this example!

    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#else
	
	nEncSurfIdx = GetFreeSurfaceIndex(m_pVPPSurfacesVPPOutEnc, m_nSurfNumVPPOutEnc);	// Find free output frame surface
	MSDK_CHECK_ERROR(MFX_ERR_NOT_FOUND, nEncSurfIdx, MFX_ERR_MEMORY_ALLOC);

	sts = m_pMfxAllocator->Lock(m_pMfxAllocator->pthis, m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data.MemId, 
														&(m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data));
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	IppiSize roiSize;
	roiSize.width = m_mfxEncParams.mfx.FrameInfo.CropW;
	roiSize.height = m_mfxEncParams.mfx.FrameInfo.CropH;
	int pitch = m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data.Pitch;
	ippiBGRToYCbCr420_8u_AC4P2R( (Ipp8u*)pInputBuffer, roiSize.width*4,  (Ipp8u*)m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data.Y, pitch,
									(Ipp8u*)m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data.UV, pitch, roiSize);	
#if 0
	fwrite(m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data.Y,  roiSize.width*roiSize.height,   1, gNV12File);
	for(int i = 0; i<roiSize.height/2; i++)
		fwrite(m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data.UV + i*pitch, roiSize.width, 1, gNV12File);
#endif
	sts = m_pMfxAllocator->Unlock(m_pMfxAllocator->pthis, m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data.MemId, 
															&(m_pVPPSurfacesVPPOutEnc[nEncSurfIdx].Data));
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#endif

    for (;;) {
        // Encode a frame asychronously (returns immediately)
        
        if(flags == 1){
			mfxVideoParam par;
    		memset(&par, 0, sizeof(par));
    		sts = m_pMfxEnc->GetVideoParam(&par);
    		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			
			mfxExtEncoderResetOption resetOption;
			memset(&resetOption, 0, sizeof(resetOption));
			resetOption.Header.BufferId = MFX_EXTBUFF_ENCODER_RESET_OPTION;
			resetOption.Header.BufferSz = sizeof(resetOption);
			resetOption.StartNewSequence = MFX_CODINGOPTION_ON;
			
			mfxExtBuffer* extendedBuffers[1];
    		extendedBuffers[0] = (mfxExtBuffer*) & resetOption;
			par.NumExtParam = 1;
			par.ExtParam = extendedBuffers;

			sts = m_pMfxEnc->Reset(&par);
			MSDK_CHECK_RESULT(sts,MFX_ERR_NONE,sts);
			mfxEncodeCtrl curEncCtrl;	
			memset(&curEncCtrl, 0, sizeof(curEncCtrl));
			curEncCtrl.FrameType = MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF | MFX_FRAMETYPE_IDR;
			sts = m_pMfxEnc->EncodeFrameAsync(&curEncCtrl, &m_pVPPSurfacesVPPOutEnc[nEncSurfIdx], &m_mfxBS, &syncpEnc);
        }
		else{
		#if 0
			mfxVideoParam par;
			memset(&par, 0, sizeof(par));
			sts = m_pMfxEnc->GetVideoParam(&par);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

			par.mfx.TargetUsage = frames;
			sts = m_pMfxEnc->Reset(&par);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			
			memset(&par, 0, sizeof(par));
			sts = m_pMfxEnc->GetVideoParam(&par);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		#endif
			sts = m_pMfxEnc->EncodeFrameAsync(NULL, &m_pVPPSurfacesVPPOutEnc[nEncSurfIdx], &m_mfxBS, &syncpEnc);
		}
        if (MFX_ERR_NONE < sts && !syncpEnc) {  // Repeat the call if warning and no output
            if (MFX_WRN_DEVICE_BUSY == sts)
                MSDK_SLEEP(1);  // Wait if device is busy, then repeat the same call
        } else if (MFX_ERR_NONE < sts && syncpEnc) {
            sts = MFX_ERR_NONE;     // Ignore warnings if output is available
            break;
        } else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts) {
            // Allocate more bitstream buffer memory here if needed...
            break;
        } else
            break;
    }

    if (MFX_ERR_NONE == sts) {
        sts = m_session.SyncOperation(syncpEnc, 60000);   // Synchronize. Wait until encoded frame is ready
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

//        sts = WriteBitStreamFrame(&mfxBS, fSink);
		*pBitStreamBuf = m_mfxBS.Data + m_mfxBS.DataOffset;
		*pBitStremBufLen = m_mfxBS.DataLength;
		//NvLog("bitstreamlen: %d", *pBitStremBufLen);
		m_mfxBS.DataLength = 0;
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }

	frames = (frames+1) % 7;
	return MFX_ERR_NONE;
}

void CEncodingVmemVpp::Close()
{
	if(m_pMfxEnc)
		m_pMfxEnc->Close();
	if(m_pMfxVpp)
    	m_pMfxVpp->Close();

    MSDK_SAFE_DELETE_ARRAY(m_pMfxSurfacesVPPIn);
    MSDK_SAFE_DELETE_ARRAY(m_pVPPSurfacesVPPOutEnc);
	
    MSDK_SAFE_DELETE_ARRAY(m_mfxBS.Data);
    MSDK_SAFE_DELETE_ARRAY(m_extDoNotUse.AlgList);

    m_pMfxAllocator->Free(m_pMfxAllocator->pthis, &m_mfxResponseVPPIn);
    m_pMfxAllocator->Free(m_pMfxAllocator->pthis, &m_mfxResponseVPPOutEnc);
}

mfxStatus CEncodingVmemVpp::ResetMFXResolution(mfxU32 width, mfxU32 height)
{
	m_mfxEncParams.mfx.FrameInfo.Width  = MSDK_ALIGN16(width);
    m_mfxEncParams.mfx.FrameInfo.Height = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxEncParams.mfx.FrameInfo.PicStruct)?
    MSDK_ALIGN16(height) : MSDK_ALIGN32(height);

	m_mfxEncParams.mfx.FrameInfo.CropW = width;
	m_mfxEncParams.mfx.FrameInfo.CropH = height;
	
	m_mfxVppParams.vpp.In.Width     = MSDK_ALIGN16(width);
    m_mfxVppParams.vpp.In.Height    = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVppParams.vpp.In.PicStruct)?
        MSDK_ALIGN16(height) : MSDK_ALIGN32(height);
	
	m_mfxVppParams.vpp.Out.Width = m_mfxVppParams.vpp.In.Width;
	m_mfxVppParams.vpp.Out.Height = m_mfxVppParams.vpp.In.Height;

	m_mfxVppParams.vpp.In.CropW = m_mfxVppParams.vpp.Out.CropW = width;
	m_mfxVppParams.vpp.In.CropH = m_mfxVppParams.vpp.Out.CropH = height;

	Close();
	AllocFrameBuffers();
	return MFX_ERR_NONE;
}

void initCmdOptions(CmdOptions* cmd_options)
{
	memset(cmd_options, 0, sizeof(CmdOptions));
	cmd_options->impl 			= MFX_IMPL_HARDWARE;
	cmd_options->Width 			= 1920;
    cmd_options->Height 		= 1080;
	cmd_options->Bitrate 		= 5000;
	cmd_options->FrameRateN  	= 30;
    cmd_options->FrameRateD  	= 1;
}

