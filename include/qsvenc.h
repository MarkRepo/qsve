#ifndef _H264_ROUTE_H_
#define _H264_ROUTE_H_

#include <stdint.h> 
#include <cstddef>

typedef enum 
{
	ENCODE_RGB4 = 0,
	ENCODE_YV12 = 1,
}ENCODE_TYPE;

typedef struct {
	unsigned short bitRate;
	unsigned short targetUsage;
}TestParams;


#ifdef __cplusplus
extern "C" 
{
#endif

extern int    qsve_init(int max_number_surface, ENCODE_TYPE encode_type);
extern int 	  qsve_set_params(TestParams* params);
extern int    qsve_create_mem(int stride, int height, int id);
extern int    qsve_destroy_mem(int id);
extern int	  qsve_encode_frame(int id, const void* pInputBuffer, void** pBitStreamBuffer , size_t* pBitStreamLen, int flags);
extern void   qsve_release();

#ifdef __cplusplus
}
#endif

#endif

