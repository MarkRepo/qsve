#ifndef _SIMPLE_ENCODE_VMEM_PREPROC_H_
#define _SIMPLE_ENCODE_VMEM_PREPROC_H_

#include "common_utils.h"
#include "qsvenc.h"
#include <cstddef>

struct CmdOptions {
    mfxIMPL impl; // OPTION_IMPL
    mfxU16 Width; // OPTION_GEOMETRY
    mfxU16 Height;
    mfxU16 Bitrate; // OPTION_BITRATE
    mfxU16 FrameRateN; // OPTION_FRAMERATE
    mfxU16 FrameRateD;
};


void initCmdOptions(CmdOptions * cmd_options);

class CEncodingVmemVpp
{
	public:
		CEncodingVmemVpp();
		~CEncodingVmemVpp();
		mfxStatus Init(CmdOptions* options);
		mfxStatus InitSessionAndSetAllocator(CmdOptions* options);
		void 	  InitEncParams(CmdOptions* option);
		void      InitVppParams(CmdOptions* option);
		void      SetTestParams(TestParams* params);
		mfxStatus AllocFrameBuffers();
		mfxStatus EncodeFrame(const void* pInputBuffer, void** pBitStreamBuf, size_t* pBitStremBufLen, int flags);
		mfxStatus ResetMFXResolution(mfxU32 width, mfxU32 height);
		void 	  Close();

	public:
		mfxU32					m_fourcc;
		MFXVideoSession 		m_session;
		mfxFrameAllocator* 		m_pMfxAllocator;
		mfxVideoParam  			m_mfxEncParams;
		mfxVideoParam  			m_mfxVppParams;
		MFXVideoENCODE*			m_pMfxEnc;
		MFXVideoVPP*			m_pMfxVpp;
		mfxBitstream 			m_mfxBS;
		mfxFrameSurface1*  		m_pMfxSurfacesVPPIn;
		mfxFrameSurface1*  		m_pVPPSurfacesVPPOutEnc;
		mfxU16              	m_nSurfNumVPPIn;
		mfxU16 					m_nSurfNumVPPOutEnc;
		mfxFrameAllocResponse 	m_mfxResponseVPPIn;
    	mfxFrameAllocResponse 	m_mfxResponseVPPOutEnc;
		mfxExtVPPDoNotUse 		m_extDoNotUse;
};

#endif

