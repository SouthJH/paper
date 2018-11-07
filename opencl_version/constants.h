#pragma once
#include <CL/cl.h>
#include <vector>

//#define		DEBUG_MODE

#ifdef	DEBUG_MODE
	#include <stdio.h>
#endif

#define		SUCCESS   0

// api.c ERRORS
#define		M2S_INVALID_NUM_ENTRIES		0x010
#define		M2S_TRANSFER_DATA_ERROR		0x011
#define		M2S_INVALID_DEVICE_HINT		0x012
#define		M2S_MEMORY_ALLOC_FAILED		0x013
#define		M2S_CREATE_QUEUE_FAILED		0x014
#define		M2S_CREATE_BUFFR_FAILED		0x015
#define		M2S_SET_ARGUMENT_FAILED		0x016


// hints
#define		M2S_HINT_HEAD			0x100
#define		M2S_HINT_1D_ALL			0x101
#define		M2S_HINT_1D_X			0x102
#define		M2S_HINT_1D_SORT		0x103
#define		M2S_HINT_2D_ALL			0x104
#define		M2S_HINT_2D_X			0x105
#define		M2S_HINT_2D_Y			0x106
#define		M2S_HINT_2D_BLK			0x107
#define		M2S_HINT_3D_ALL			0x108
#define		M2S_HINT_3D_X			0x109
#define		M2S_HINT_3D_Y			0x110
#define		M2S_HINT_3D_Z			0x111
#define		M2S_HINT_3D_BLK			0x112
#define		M2S_HINT_TAIL			0x113


#define		M2S_DATA_TYPE_HEAD			0x200
#define		M2S_DATA_TYPE_CHAR			0x201
#define		M2S_DATA_TYPE_INT			0x202
#define		M2S_DATA_TYPE_UINT			0x203
#define		M2S_DATA_TYPE_FLOAT			0x204
#define		M2S_DATA_TYPE_DOUBLE		0x205
#define		M2S_DATA_TYPE_TAIL			0x206


// parameters
#define		M2S_MEM_SIZE					sizeof(cl_mem)

#define		M2S_TRUE						CL_TRUE
#define		M2S_FALSE						CL_FALSE
#define		M2S_BLOCKING					CL_BLOCKING
#define		M2S_NON_BLOCKING				CL_NON_BLOCKING

#define		M2S_MEM_READ_WRITE				CL_MEM_READ_WRITE
#define		M2S_MEM_READ_ONLY				CL_MEM_READ_ONLY
#define		M2S_MEM_WRITE_ONLY				CL_MEM_WRITE_ONLY
#define		M2S_MEM_COPY_HOST_PTR			CL_MEM_COPY_HOST_PTR

#define		M2S_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE		CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE

#define		M2S_PLATFORM_NAME				CL_PLATFORM_NAME
#define		M2S_PLATFORM_VENDOR				CL_PLATFORM_VENDOR
#define		M2S_PLATFORM_VERSION			CL_PLATFORM_VERSION
#define		M2S_PLATFORM_PROFILE			CL_PLATFORM_PROFILE
#define		M2S_PLATFORM_EXTENSIONS			CL_PLATFORM_EXTENSIONS
#define		M2S_DEVICE_TYPE_ALL				CL_DEVICE_TYPE_ALL
#define		M2S_DEVICE_TYPE_GPU				CL_DEVICE_TYPE_GPU
#define		M2S_DEVICE_TYPE_CPU				CL_DEVICE_TYPE_CPU
#define		M2S_DEVICE_TYPE_ACCELERATOR		CL_DEVICE_TYPE_ACCELERATOR
#define		M2S_DEVICE_TYPE_CUSTOM			CL_DEVICE_TYPE_CUSTOM
#define		M2S_DEVICE_TYPE_DEFAULT			CL_DEVICE_TYPE_DEFAULT
