#include "ipp.h"
#include "ippcc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <cstddef>

int main(int argc, char* argv[])
{

	FILE* srcRGB = fopen(argv[1], "rb");
	FILE* destY  = fopen(argv[2], "wb+");
	//FILE* destUV = fopen(argv[3], "wb+");
	//FILE* destU  = fopen(argv[3], "wb+");
	//FILE* destV  = fopen(argv[4], "wb+");
   const IppLibraryVersion *lib;
   IppStatus status;
   Ipp64u mask, emask;
   /* Init IPP library */
   ippInit();
   /* Get IPP library version info */
   lib = ippGetLibVersion();
   printf("%s %s\n", lib->Name, lib->Version);

   /* Get CPU features and features enabled with selected library level */

   status = ippGetCpuFeatures( &mask, 0 );

   if( ippStsNoErr == status ) {

      emask = ippGetEnabledCpuFeatures();

      printf("Features supported by CPU\tby IPP\n");

      printf("-----------------------------------------\n");
      printf("  ippCPUID_MMX        = ");
      printf("%c\t%c\t",( mask & ippCPUID_MMX ) ? 'Y':'N',( emask & ippCPUID_MMX ) ? 'Y':'N');
      printf("Intel(R) Architecture MMX technology supported\n");
      printf("  ippCPUID_SSE        = ");
      printf("%c\t%c\t",( mask & ippCPUID_SSE ) ? 'Y':'N',( emask & ippCPUID_SSE ) ? 'Y':'N');
      printf("Intel(R) Streaming SIMD Extensions\n");
      printf("  ippCPUID_SSE2       = ");
      printf("%c\t%c\t",( mask & ippCPUID_SSE2 ) ? 'Y':'N',( emask & ippCPUID_SSE2 ) ? 'Y':'N');
      printf("Intel(R) Streaming SIMD Extensions 2\n");
      printf("  ippCPUID_SSE3       = ");
      printf("%c\t%c\t",( mask & ippCPUID_SSE3 ) ? 'Y':'N',( emask & ippCPUID_SSE3 ) ? 'Y':'N');
      printf("Intel(R) Streaming SIMD Extensions 3\n");
      printf("  ippCPUID_SSSE3      = ");
      printf("%c\t%c\t",( mask & ippCPUID_SSSE3 ) ? 'Y':'N',( emask & ippCPUID_SSSE3 ) ? 'Y':'N');
      printf("Intel(R) Supplemental Streaming SIMD Extensions 3\n");
      printf("  ippCPUID_MOVBE      = ");
      printf("%c\t%c\t",( mask & ippCPUID_MOVBE ) ? 'Y':'N',( emask & ippCPUID_MOVBE ) ? 'Y':'N');
      printf("The processor supports MOVBE instruction\n");
      printf("  ippCPUID_SSE41      = ");
      printf("%c\t%c\t",( mask & ippCPUID_SSE41 ) ? 'Y':'N',( emask & ippCPUID_SSE41 ) ? 'Y':'N');
      printf("Intel(R) Streaming SIMD Extensions 4.1\n");
      printf("  ippCPUID_SSE42      = ");
      printf("%c\t%c\t",( mask & ippCPUID_SSE42 ) ? 'Y':'N',( emask & ippCPUID_SSE42 ) ? 'Y':'N');
      printf("Intel(R) Streaming SIMD Extensions 4.2\n");
      printf("  ippCPUID_AVX        = ");
      printf("%c\t%c\t",( mask & ippCPUID_AVX ) ? 'Y':'N',( emask & ippCPUID_AVX ) ? 'Y':'N');
      printf("Intel(R) Advanced Vector Extensions instruction set\n");
      printf("  ippAVX_ENABLEDBYOS  = ");
      printf("%c\t%c\t",( mask & ippAVX_ENABLEDBYOS ) ? 'Y':'N',( emask & ippAVX_ENABLEDBYOS ) ? 'Y':'N');
      printf("The operating system supports Intel(R) AVX\n");
      printf("  ippCPUID_AES        = ");
      printf("%c\t%c\t",( mask & ippCPUID_AES ) ? 'Y':'N',( emask & ippCPUID_AES ) ? 'Y':'N');
      printf("Intel(R) AES instruction\n");
      printf("  ippCPUID_SHA        = ");
      printf("%c\t%c\t",( mask & ippCPUID_SHA ) ? 'Y':'N',( emask & ippCPUID_SHA ) ? 'Y':'N');
      printf("Intel(R) SHA new instructions\n");
      printf("  ippCPUID_CLMUL      = ");
      printf("%c\t%c\t",( mask & ippCPUID_CLMUL ) ? 'Y':'N',( emask & ippCPUID_CLMUL ) ? 'Y':'N');
      printf("PCLMULQDQ instruction\n");
      printf("  ippCPUID_RDRAND     = ");
      printf("%c\t%c\t",( mask & ippCPUID_RDRAND ) ? 'Y':'N',( emask & ippCPUID_RDRAND ) ? 'Y':'N');
      printf("Read Random Number instructions\n");
      printf("  ippCPUID_F16C       = ");
      printf("%c\t%c\t",( mask & ippCPUID_F16C ) ? 'Y':'N',( emask & ippCPUID_F16C ) ? 'Y':'N');
      printf("Float16 instructions\n");
      printf("  ippCPUID_AVX2       = ");
      printf("%c\t%c\t",( mask & ippCPUID_AVX2 ) ? 'Y':'N',( emask & ippCPUID_AVX2 ) ? 'Y':'N');
      printf("Intel(R) Advanced Vector Extensions 2 instruction set\n");
      printf("  ippCPUID_AVX512F    = ");
      printf("%c\t%c\t",( mask & ippCPUID_AVX512F ) ? 'Y':'N',( emask & ippCPUID_AVX512F ) ? 'Y':'N');
      printf("Intel(R) Advanced Vector Extensions 3.1 instruction set\n");
      printf("  ippCPUID_AVX512CD   = ");
      printf("%c\t%c\t",( mask & ippCPUID_AVX512CD ) ? 'Y':'N',( emask & ippCPUID_AVX512CD ) ? 'Y':'N');
      printf("Intel(R) Advanced Vector Extensions CD (Conflict Detection) instruction set\n");
      printf("  ippCPUID_AVX512ER   = ");
      printf("%c\t%c\t",( mask & ippCPUID_AVX512ER ) ? 'Y':'N',( emask & ippCPUID_AVX512ER ) ? 'Y':'N');
      printf("Intel(R) Advanced Vector Extensions ER instruction set\n");
      printf("  ippCPUID_ADCOX      = ");
      printf("%c\t%c\t",( mask & ippCPUID_ADCOX ) ? 'Y':'N',( emask & ippCPUID_ADCOX ) ? 'Y':'N');
      printf("ADCX and ADOX instructions\n");
      printf("  ippCPUID_RDSEED     = ");
      printf("%c\t%c\t",( mask & ippCPUID_RDSEED ) ? 'Y':'N',( emask & ippCPUID_RDSEED ) ? 'Y':'N');
      printf("The RDSEED instruction\n");
      printf("  ippCPUID_PREFETCHW  = ");
      printf("%c\t%c\t",( mask & ippCPUID_PREFETCHW ) ? 'Y':'N',( emask & ippCPUID_PREFETCHW ) ? 'Y':'N');
      printf("The PREFETCHW instruction\n");
      printf("  ippCPUID_KNC        = ");
      printf("%c\t%c\t",( mask & ippCPUID_KNC ) ? 'Y':'N',( emask & ippCPUID_KNC ) ? 'Y':'N');
      printf("Intel? Xeon Phi? Coprocessor instruction set\n");
   }

	IppiSize roiSize;
	roiSize.width = 1920;
	roiSize.height = 1080;
	int frameSize = 1920*1080*4;
	unsigned char* pBuffer = (unsigned char*)malloc(frameSize);
	unsigned char* pYBuffer = (unsigned char*)malloc(frameSize/4);
	unsigned char* pUBuffer = (unsigned char*)malloc(1920*1080/4);
	unsigned char* pVBuffer = (unsigned char*)malloc(1920*1080/4);
	unsigned char* pDst[3] = {pYBuffer, pUBuffer, pVBuffer};
	int dstStep[3] = {1920, 1920/2, 1920/2};
	unsigned char* pUVBuffer = (unsigned char*)malloc(1920*1080/2);
	int nBytesRead = 0;
	while(1)
	{	
		nBytesRead = fread(pBuffer, 1, frameSize, srcRGB);
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
	//status = ippiRGBToYUV420_8u_C3P3(pBuffer, pDst, roiSize);
	//status = ippiRGBToYCrCb420_8u_AC4P3R( (Ipp8u*)pBuffer, 1920*4, (Ipp8u**)pDst, dstStep, roiSize );
	memset(pYBuffer, 0, 1920*1080);
	memset(pUVBuffer, 0, 1920*1080/2);
	status = ippiBGRToYCbCr420_8u_AC4P2R( (Ipp8u*)pBuffer, 1920*4,  (Ipp8u*)pYBuffer, 1920, (Ipp8u*)pUVBuffer, 1920,  roiSize);
	if(status == ippStsNoErr)
		fwrite(pYBuffer, 1, 1920*1080, destY);
		fwrite(pUVBuffer, 1, 1920*1080/2, destY);
	}
	
   
   return 0;
}
