#define _CRT_SECURE_NO_WARNINGS
#include "m2s_api.h"
#include <chrono>
#include <iostream>


#define   CHECK_ERROR(err) \
  if (err != CL_SUCCESS) { \
     printf("[%s:%d] ERROR %d\n", __FILE__, __LINE__, err); \
     exit(EXIT_FAILURE); \
  }
void printType(cl_device_type type) {
	printf("\t  - Type \t\t");

	if (type & CL_DEVICE_TYPE_CPU)
		printf("CPU");

	if (type & CL_DEVICE_TYPE_GPU)
		printf("GPU");

	if (type & CL_DEVICE_TYPE_ACCELERATOR)
		printf("ACCELERATOR");

	if (type & CL_DEVICE_TYPE_DEFAULT)
		printf("DEFAULT");

	if (type & CL_DEVICE_TYPE_CUSTOM)
		printf("CUSTOM");

	printf("\n");
}
char *get_source_code(const char *file_name, size_t *len) {
	char *source_code;
	size_t length;
	FILE *file = fopen(file_name, "rb");
	if (file == NULL) {
		printf("[%s:%d] Failed to open %s\n", __FILE__, __LINE__, file_name);
		exit(EXIT_FAILURE);
	}

	fseek(file, 0, SEEK_END);
	length = (size_t)ftell(file);
	rewind(file);

	source_code = (char *)malloc(length + 1);
	fread(source_code, length, 1, file);
	source_code[length] = '\0';

	fclose(file);

	*len = length;
	return source_code;
}

int device_info()
{
	cl_platform_id		*platforms;
	cl_device_id		**devices;

	cl_int err;

	cl_uint num_platforms;

	err = clGetPlatformIDs(0, NULL, &num_platforms);			CHECK_ERROR(err);	printf("Number of platforms: %u\n", num_platforms);

	platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * num_platforms);
	devices = (cl_device_id **)malloc(sizeof(cl_device_id *) * num_platforms);

	err = clGetPlatformIDs(num_platforms, platforms, NULL);		CHECK_ERROR(err);

	for (cl_uint idx = 0; idx < num_platforms; ++idx)
	{
		printf("\nplatform #%u\n", idx);

		//size_t name_size;
		//char *name;
		char name[1024];

		//err = clGetPlatformInfo(platforms[idx], CL_PLATFORM_NAME, 0, NULL, &name_size);		CHECK_ERROR(err);
		//name = (char *)malloc(sizeof(char) * name_size);
		err = clGetPlatformInfo(platforms[idx], CL_PLATFORM_NAME, 1024, name, NULL);	CHECK_ERROR(err);	printf(" - Name \t\t%s\n", name);
		err = clGetPlatformInfo(platforms[idx], CL_PLATFORM_VENDOR, 1024, name, NULL);	CHECK_ERROR(err);	printf(" - Vendor \t\t%s\n", name);
		err = clGetPlatformInfo(platforms[idx], CL_PLATFORM_VERSION, 1024, name, NULL); CHECK_ERROR(err);	printf(" - Version \t\t%s\n", name);

		cl_uint num_devices;
		err = clGetDeviceIDs(platforms[idx], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);			CHECK_ERROR(err);	printf(" - Number of devices \t%u\n", num_devices);
		devices[idx] = (cl_device_id *)malloc(sizeof(cl_device_id) * num_devices);
		err = clGetDeviceIDs(platforms[idx], CL_DEVICE_TYPE_ALL, num_devices, devices[idx], NULL);	CHECK_ERROR(err);
		for (cl_uint jdx = 0; jdx < num_devices; ++jdx)
		{
			printf("\t device #%u\n", jdx);

			cl_device_type type;
			cl_uint cu;
			cl_ulong mem;
			size_t wg;

			err = clGetDeviceInfo(devices[idx][jdx], CL_DEVICE_NAME, 1024, name, NULL);								CHECK_ERROR(err);	printf("\t  - Name \t\t%s\n", name);
			err = clGetDeviceInfo(devices[idx][jdx], CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);			CHECK_ERROR(err);	printType(type);
			err = clGetDeviceInfo(devices[idx][jdx], CL_DEVICE_VENDOR, 1024, name, NULL);							CHECK_ERROR(err);	printf("\t  - Vendor \t\t%s\n", name);
			err = clGetDeviceInfo(devices[idx][jdx], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &cu, NULL);		CHECK_ERROR(err);	printf("\t  - Max CU's # \t\t%u\n", cu);
			err = clGetDeviceInfo(devices[idx][jdx], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &wg, NULL);		CHECK_ERROR(err);	printf("\t  - Max Workgroup Size \t%zu\n", wg);
			err = clGetDeviceInfo(devices[idx][jdx], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &mem, NULL);		CHECK_ERROR(err);	printf("\t  - Global Mem Size \t%lu\n", mem);
			err = clGetDeviceInfo(devices[idx][jdx], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &mem, NULL);		CHECK_ERROR(err);	printf("\t  - local Mem Size \t%lu\n", mem);
			err = clGetDeviceInfo(devices[idx][jdx], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &mem, NULL); 	CHECK_ERROR(err);	printf("\t  - Max Mem Alloc Size \t%lu\n", mem);
		}
	}

	return 0;
}

void test_1d(int n) {
	m2s_platform_id platform;
	m2s_device_id device;
	m2s_context context;
	m2s_command_queue queue;
	m2s_program program;
	char *kernel_source;
	size_t kernel_source_size;
	m2s_kernel kernel;
	m2s_int err;

	err = m2sGetPlatformIDs(1, &platform, NULL);								CHECK_ERROR(err);
	err = m2sGetDeviceIDs(platform, M2S_DEVICE_TYPE_GPU, n, &device, NULL);		CHECK_ERROR(err);
	context = m2sCreateContext(NULL, device, NULL, NULL, &err);					CHECK_ERROR(err);
	queue = m2sCreateCommandQueue(context, &device, 0, &err);					CHECK_ERROR(err);

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = m2sCreateProgramWithSource(context, 1, (const char **)&kernel_source, &kernel_source_size, &err);		CHECK_ERROR(err);
	err = m2sBuildProgram(program, &device, "", NULL, NULL);
	if (err == CL_BUILD_PROGRAM_FAILURE) {
		printf("failed to build program\n");
		exit(0);
	} CHECK_ERROR(err);

	kernel = m2sCreateKernel(program, "vec_add", &err);		CHECK_ERROR(err);

	int A[16384], B[16384], C[16384];
	for (int idx = 0; idx < 16384; ++idx) {
		A[idx] = rand() % 100;
		B[idx] = rand() % 100;
	}

	m2s_mem buffer_A, buffer_B, buffer_C;

	buffer_A = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, M2S_MEM_READ_ONLY, sizeof(int), 16384, &err);		CHECK_ERROR(err);
	buffer_B = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, M2S_MEM_READ_ONLY, sizeof(int), 16384, &err);		CHECK_ERROR(err);
	buffer_C = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, M2S_MEM_WRITE_ONLY, sizeof(int), 16384, &err);	CHECK_ERROR(err);

	m2sCreateDeviceHint(kernel, &device, sizeof(int) * 16384 * 3);

	err = m2sEnqueueWriteBuffer(&queue, &buffer_A, M2S_FALSE, 0, sizeof(int) * 16384, A, 0, NULL, NULL);		CHECK_ERROR(err);
	err = m2sEnqueueWriteBuffer(&queue, &buffer_B, M2S_FALSE, 0, sizeof(int) * 16384, B, 0, NULL, NULL);		CHECK_ERROR(err);

	err = m2sSetKernelArg(kernel, &device, 0, M2S_MEM_SIZE, &buffer_A);		CHECK_ERROR(err);
	err = m2sSetKernelArg(kernel, &device, 1, M2S_MEM_SIZE, &buffer_B);		CHECK_ERROR(err);
	err = m2sSetKernelArg(kernel, &device, 2, M2S_MEM_SIZE, &buffer_C);		CHECK_ERROR(err);

	size_t global_size = 16384;
	size_t local_size = 256;
	err = m2sEnqueueNDRangeKernel(&queue, kernel, M2S_HINT_1D_ALL, NULL, &global_size, &local_size, 0, NULL, NULL);	CHECK_ERROR(err);

	err = m2sEnqueueReadBuffer(&queue, &buffer_C, M2S_TRUE, 0, sizeof(int) * 16384, C, 0, NULL, NULL);	CHECK_ERROR(err);

	int idx;

	for (idx = 0; idx < 16384; ++idx) {
		if (A[idx] + B[idx] != C[idx]) {
			printf("\nverification is failed\n");
			break;
		}
	}

	if (idx == 16384)
		printf("\nverification success\n");
}

void test_2d(int n) {
	m2s_platform_id platform;
	m2s_device_id device;
	m2s_context context;
	m2s_command_queue queue;
	m2s_program program;
	char *kernel_source;
	size_t kernel_source_size;
	m2s_kernel kernel;
	m2s_int err;

	err = m2sGetPlatformIDs(1, &platform, NULL);								CHECK_ERROR(err);
	err = m2sGetDeviceIDs(platform, M2S_DEVICE_TYPE_GPU, n, &device, NULL);		CHECK_ERROR(err);
	context = m2sCreateContext(NULL, device, NULL, NULL, &err);					CHECK_ERROR(err);
	queue = m2sCreateCommandQueue(context, &device, 0, &err);					CHECK_ERROR(err);

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = m2sCreateProgramWithSource(context, 1, (const char **)&kernel_source, &kernel_source_size, &err);		CHECK_ERROR(err);
	err = m2sBuildProgram(program, &device, "", NULL, NULL);
	if (err == CL_BUILD_PROGRAM_FAILURE) {
		printf("failed to build program\n");
		exit(0);
	} CHECK_ERROR(err);

	kernel = m2sCreateKernel(program, "vec_add_2d", &err);		CHECK_ERROR(err);

	int A[128][128], B[128][128], C[128][128];
	for (int idx = 0; idx < 128; ++idx) {
		for (int jdx = 0; jdx < 128; ++jdx) {
			A[idx][jdx] = rand() % 100;
			B[idx][jdx] = rand() % 100;
		}
	}

	m2s_mem buffer_A, buffer_B, buffer_C;

	buffer_A = m2sCreateBuffer(context, &device, M2S_HINT_2D_ALL, M2S_MEM_READ_ONLY, sizeof(int), 16384, &err);		CHECK_ERROR(err);
	buffer_B = m2sCreateBuffer(context, &device, M2S_HINT_2D_ALL, M2S_MEM_READ_ONLY, sizeof(int), 16384, &err);		CHECK_ERROR(err);
	buffer_C = m2sCreateBuffer(context, &device, M2S_HINT_2D_ALL, M2S_MEM_WRITE_ONLY, sizeof(int), 16384, &err);	CHECK_ERROR(err);

	m2sCreateDeviceHint(kernel, &device, sizeof(int) * 16384 * 3);

	err = m2sEnqueueWriteBuffer(&queue, &buffer_A, M2S_FALSE, 0, sizeof(int) * 16384, A, 0, NULL, NULL);		CHECK_ERROR(err);
	err = m2sEnqueueWriteBuffer(&queue, &buffer_B, M2S_FALSE, 0, sizeof(int) * 16384, B, 0, NULL, NULL);		CHECK_ERROR(err);

	err = m2sSetKernelArg(kernel, &device, 0, M2S_MEM_SIZE, &buffer_A);		CHECK_ERROR(err);
	err = m2sSetKernelArg(kernel, &device, 1, M2S_MEM_SIZE, &buffer_B);		CHECK_ERROR(err);
	err = m2sSetKernelArg(kernel, &device, 2, M2S_MEM_SIZE, &buffer_C);		CHECK_ERROR(err);

	//size_t global_size = 16384;
	//size_t local_size = 256;
	size_t global_size[2] = { 128, 128 };
	size_t local_size[2] = { 16, 16 };
	err = m2sEnqueueNDRangeKernel(&queue, kernel, M2S_HINT_2D_ALL, NULL, global_size, local_size, 0, NULL, NULL);		CHECK_ERROR(err);

	err = m2sEnqueueReadBuffer(&queue, &buffer_C, M2S_TRUE, 0, sizeof(int) * 16384, C, 0, NULL, NULL);	CHECK_ERROR(err);

	int idx, jdx;

	for (idx = 0; idx < 128; ++idx) {
		for (jdx = 0; jdx < 128; ++jdx) {
			if (A[idx][jdx] + B[idx][jdx] != C[idx][jdx]) {
				printf("\nverification is failed\n");
				break;
			}
		}
	}

	if (idx == 128 && jdx == 128)
		printf("\nverification success\n");
}

void cl_test_single(size_t size) {
	cl_platform_id		platform[2];
	cl_device_id		device;
	cl_context			context;
	cl_program			program;
	cl_kernel			kernel;
	cl_command_queue	queue;

	char *kernel_source;
	size_t kernel_source_size;

	cl_ulong mem, local_size, total_size, global_work_size;
	cl_int err;

	clGetPlatformIDs(2, platform, NULL);

	clGetDeviceIDs(platform[0], CL_DEVICE_TYPE_ALL, 1, &device, NULL);
	
	err = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &mem, NULL);
	printf("- Global Mem Size \t%lu\n", mem);
	err = clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &mem, NULL);
	printf("- Max Mem Alloc Size \t%lu\n\n", mem);

	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);

	queue = clCreateCommandQueue(context, device, NULL, &err);

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, &kernel_source_size, &err);

	clBuildProgram(program, 1, &device, "", NULL, NULL);

	kernel = clCreateKernel(program, "vec_add", &err);

	cl_mem buffer_A, buffer_B, buffer_C;

	int *A = (int *)malloc(sizeof(int) * size);
	int *B = (int *)malloc(sizeof(int) * size);
	int *C = (int *)malloc(sizeof(int) * size);

	printf("total size: %lld bytes\n", size * sizeof(int));
	printf("total size: %.2f Kbytes\n", (float)size * sizeof(int) / 1024);
	printf("total size: %.2f Mbytes\n", (float)size * sizeof(int) / 1024 / 1024);
	printf("total size: %.2f Gbytes\n", (float)size * sizeof(int) / 1024 / 1024 / 1024);

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	/*
	buffer_A = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(int), NULL, &err);
	buffer_B = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(int), NULL, &err);
	buffer_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(int), NULL, &err);

	clEnqueueWriteBuffer(queue, buffer_A, CL_FALSE, 0, size * sizeof(int), A, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, buffer_B, CL_FALSE, 0, size * sizeof(int), B, 0, NULL, NULL);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_A);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_B);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_C);

	clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &size, NULL, 0, NULL, NULL);

	clEnqueueReadBuffer(queue, buffer_C, CL_FALSE, 0, size * sizeof(int), C, 0, NULL, NULL);
	//*/

	//*
		
	buffer_A = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(int), NULL, &err);
	buffer_B = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(int), NULL, &err);
	buffer_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(int), NULL, &err);

	clEnqueueWriteBuffer(queue, buffer_A, CL_TRUE, 0, size * sizeof(int), A, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, buffer_B, CL_TRUE, 0, size * sizeof(int), B, 0, NULL, NULL);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_A);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_B);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_C);
		
	clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &size, NULL, 0, NULL, NULL);

	clEnqueueReadBuffer(queue, buffer_C, CL_TRUE, 0, size * sizeof(int), C, 0, NULL, NULL);

	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
	std::cout << "elapsed time : " << sec.count() << " seconds" << std::endl;


	int idx;
	for (idx = 0; idx < size; ++idx) {
		if (A[idx] + B[idx] != C[idx]) {
			printf("\nverification is failed\n");
			break;
		}
	}
	if (idx == size)
		printf("\nverification success\n");
}

void m2s_test_single(size_t size) {
	m2s_platform_id		platform[2];
	m2s_device_id		device;
	m2s_context			context;
	m2s_program			program;
	m2s_kernel			kernel;
	m2s_command_queue	queue;

	char *kernel_source;
	size_t kernel_source_size;

	cl_int err;

	m2sGetPlatformIDs(2, platform, NULL);

	m2sGetDeviceIDs(platform[0], CL_DEVICE_TYPE_GPU, 1, &device, NULL);

	context = m2sCreateContext(NULL, device, NULL, NULL, &err);

	queue = m2sCreateCommandQueue(context, &device, NULL, &err);

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = m2sCreateProgramWithSource(context, 1, (const char **)&kernel_source, &kernel_source_size, &err);

	m2sBuildProgram(program, &device, "", NULL, NULL);

	kernel = m2sCreateKernel(program, "vec_add", &err);

	m2s_mem buffer_A, buffer_B, buffer_C;

	int *A = (int *)malloc(sizeof(int) * size);
	int *B = (int *)malloc(sizeof(int) * size);
	int *C = (int *)malloc(sizeof(int) * size);

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	//--------------------------------------------------------------------------------------------------------------

	buffer_A = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, CL_MEM_READ_ONLY, sizeof(int), size, &err);
	buffer_B = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, CL_MEM_READ_ONLY, sizeof(int), size, &err);
	buffer_C = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, CL_MEM_WRITE_ONLY, sizeof(int), size, &err);

	m2sCreateDeviceHint(kernel, &device, sizeof(int) * size * 3);

	m2sEnqueueWriteBuffer(&queue, &buffer_A, CL_TRUE, 0, size * sizeof(int), A, 0, NULL, NULL);
	m2sEnqueueWriteBuffer(&queue, &buffer_B, CL_TRUE, 0, size * sizeof(int), B, 0, NULL, NULL);

	m2sSetKernelArg(kernel, &device, 0, sizeof(m2s_mem), &buffer_A);
	m2sSetKernelArg(kernel, &device, 1, sizeof(m2s_mem), &buffer_B);
	m2sSetKernelArg(kernel, &device, 2, sizeof(m2s_mem), &buffer_C);

	m2sEnqueueNDRangeKernel(&queue, kernel, M2S_HINT_1D_X, NULL, &size, NULL, 0, NULL, NULL);

	m2sEnqueueReadBuffer(&queue, &buffer_C, CL_TRUE, 0, size * sizeof(int), C, 0, NULL, NULL);

	//----------------------------------------------------------------------------------------------------------------

	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
	std::cout << "elapsed time : " << sec.count() << " seconds" << std::endl;


	int idx;
	for (idx = 0; idx < size; ++idx) {
		if (A[idx] + B[idx] != C[idx]) {
			printf("\nverification is failed\n");
			break;
		}
	}
	if (idx == size)
		printf("\nverification success\n");
}

void cl_test_double(size_t size) {
	cl_platform_id		platform[2];
	cl_device_id		device[2];
	cl_context			context;
	cl_program			program;
	cl_kernel			kernel;
	cl_command_queue	queue[2];

	char *kernel_source;
	size_t kernel_source_size;

	cl_ulong mem;
	cl_int err;

	clGetPlatformIDs(2, platform, NULL);

	clGetDeviceIDs(platform[0], CL_DEVICE_TYPE_ALL, 2, device, NULL);

	err = clGetDeviceInfo(device[0], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &mem, NULL);
	printf("- Global Mem Size \t%lu\n", mem);
	err = clGetDeviceInfo(device[0], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &mem, NULL);
	printf("- Max Mem Alloc Size \t%lu\n\n", mem);

	err = clGetDeviceInfo(device[1], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &mem, NULL);
	printf("- Global Mem Size \t%lu\n", mem);
	err = clGetDeviceInfo(device[1], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &mem, NULL);
	printf("- Max Mem Alloc Size \t%lu\n\n", mem);

	context = clCreateContext(NULL, 2, device, NULL, NULL, &err);

	queue[0] = clCreateCommandQueue(context, device[0], NULL, &err);
	queue[1] = clCreateCommandQueue(context, device[1], NULL, &err);

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, &kernel_source_size, &err);

	clBuildProgram(program, 2, device, "", NULL, NULL);

	kernel = clCreateKernel(program, "vec_add", &err);

	cl_mem buffer_A, buffer_B, buffer_C;

	int *A = (int *)malloc(sizeof(int) * size);
	int *B = (int *)malloc(sizeof(int) * size);
	int *C = (int *)malloc(sizeof(int) * size);

	printf("total size: %lld bytes\n", size * sizeof(int));

	size_t global_work_size = size / 2;
	size_t global_offset = size / 2;

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	buffer_A = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(int), NULL, &err);
	buffer_B = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(int), NULL, &err);
	buffer_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(int), NULL, &err);

	clEnqueueWriteBuffer(queue[0], buffer_A, CL_FALSE, 0, size * sizeof(int), A, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue[1], buffer_A, CL_FALSE, 0, size * sizeof(int), A, 0, NULL, NULL);

	clEnqueueWriteBuffer(queue[0], buffer_B, CL_FALSE, 0, size * sizeof(int), B, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue[1], buffer_B, CL_FALSE, 0, size * sizeof(int), B, 0, NULL, NULL);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_A);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_B);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_C);

	global_work_size = size * 1 / 8;
	clEnqueueNDRangeKernel(queue[0], kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);

	global_offset = global_work_size;
	global_work_size = size * 7 / 8;
	clEnqueueNDRangeKernel(queue[1], kernel, 1, &global_offset, &global_work_size, NULL, 0, NULL, NULL);


	clEnqueueReadBuffer(queue[0], buffer_C, CL_TRUE, 0, global_offset * sizeof(int), C, 0, NULL, NULL);
	clEnqueueReadBuffer(queue[1], buffer_C, CL_TRUE, global_offset, global_work_size * sizeof(int), C + global_offset, 0, NULL, NULL);

	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
	std::cout << "elapsed time : " << sec.count() << " seconds" << std::endl;


	int idx;
	for (idx = 0; idx < size; ++idx) {
		if (A[idx] + B[idx] != C[idx]) {
			printf("\nverification is failed %d\n", idx);
			break;
		}
	}
	if (idx == size)
		printf("\nverification success\n");
}

void m2s_test_double(size_t size) {
	m2s_platform_id		platform[2];
	m2s_device_id		device;
	m2s_context			context;
	m2s_program			program;
	m2s_kernel			kernel;
	m2s_command_queue	queue;

	char *kernel_source;
	size_t kernel_source_size;

	cl_int err;

	m2sGetPlatformIDs(2, platform, NULL);

	m2sGetDeviceIDs(platform[0], CL_DEVICE_TYPE_ALL, 2, &device, NULL);

	context = m2sCreateContext(NULL, device, NULL, NULL, &err);

	queue = m2sCreateCommandQueue(context, &device, NULL, &err);

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = m2sCreateProgramWithSource(context, 1, (const char **)&kernel_source, &kernel_source_size, &err);

	m2sBuildProgram(program, &device, "", NULL, NULL);

	kernel = m2sCreateKernel(program, "vec_add", &err);

	m2s_mem buffer_A, buffer_B, buffer_C;

	int *A = (int *)malloc(sizeof(int) * size);
	int *B = (int *)malloc(sizeof(int) * size);
	int *C = (int *)malloc(sizeof(int) * size);

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	//--------------------------------------------------------------------------------------------------------------

	buffer_A = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, CL_MEM_READ_ONLY, sizeof(int), size, &err);
	buffer_B = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, CL_MEM_READ_ONLY, sizeof(int), size, &err);
	buffer_C = m2sCreateBuffer(context, &device, M2S_HINT_1D_X, CL_MEM_WRITE_ONLY, sizeof(int), size, &err);

	m2sCreateDeviceHint(kernel, &device, sizeof(int) * size * 3);

	m2sEnqueueWriteBuffer(&queue, &buffer_A, CL_TRUE, 0, size * sizeof(int), A, 0, NULL, NULL);
	m2sEnqueueWriteBuffer(&queue, &buffer_B, CL_TRUE, 0, size * sizeof(int), B, 0, NULL, NULL);

	m2sSetKernelArg(kernel, &device, 0, sizeof(m2s_mem), &buffer_A);
	m2sSetKernelArg(kernel, &device, 1, sizeof(m2s_mem), &buffer_B);
	m2sSetKernelArg(kernel, &device, 2, sizeof(m2s_mem), &buffer_C);

	m2sCreateDeviceHint(kernel, &device, sizeof(int) * size * 3);

	m2sEnqueueNDRangeKernel(&queue, kernel, M2S_HINT_1D_X, NULL, &size, NULL, 0, NULL, NULL);

	m2sEnqueueReadBuffer(&queue, &buffer_C, CL_TRUE, 0, size * sizeof(int), C, 0, NULL, NULL);

	//----------------------------------------------------------------------------------------------------------------

	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
	std::cout << "elapsed time : " << sec.count() << " seconds" << std::endl;


	int idx;
	for (idx = 0; idx < size; ++idx) {
		if (A[idx] + B[idx] != C[idx]) {
			printf("\nverification is failed\n");
			break;
		}
	}
	if (idx == size)
		printf("\nverification success\n");
}

int main() {

	//device_info();

	//test_1d(1);

	//test_2d(1);


	long long int t = 262144;
	const long long int size = t;
	printf(":::%lld:::\n", size);

	//cl_test_single(size);
	//m2s_test_single(size);

	//cl_test_double(size);
	m2s_test_double(size);

	// getchar();

	return 0;
}