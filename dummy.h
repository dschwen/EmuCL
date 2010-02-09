#ifndef EMUCL_KERNEL_H
#define EMUCL_KERNEL_H

#define __kernel ''
#define __global ''

#include "emucl_args.h"
#include <pthreads.h>
#include <malloc.h>
#include <string.h>

const int __argidmax = 50;

typedef char cl_char;
typedef int cl_int;
typedef void* cl_mem;

typedef bool cl_bool;
const cl_bool CL_TRUE = true;
const cl_bool CL_FALSE = false;

typedef int cl_command_queue;
typedef int cl_device_id;
typedef int cl_context;

struct cl_kernel
{
  __argstruct args;
}

//err = clGetDeviceIDs(NULL,CL_DEVICE_TYPE_GPU, 1, &devices, NULL);


//cl_int clSetKernelArg( kernel[0], 0, sizeof(cl_mem), &c_mem );
cl_int clSetKernelArg( cl_kernel kernel, cl_int id, cl_int size, void* ptr )
{
  if( id >= __argidmax ) return 1;
  kernel.args.arg[id] = ptr;
  return 0;
}

const cl_int CL_MEM_READ_ONLY = 0;
//cl_mem clCreateBuffer(context, CL_MEM_READ_ONLY, c_buffer_size, NULL, NULL);
cl_mem clCreateBuffer( cl_context, cl_int mode, cl_int buffer_size, void* d1, void* d2 )
{
  return (void*)malloc(buffer_size);
}


//clEnqueueWriteBuffer(cmd_queue, lap_mem, CL_TRUE, 0, lap_buffer_size, (void*)lap, 0, NULL, NULL);
cl_int clEnqueueWriteBuffer( cmd_queue queue, cl_mem dst, cl_bool d1, cl_int d2, cl_int buffer_size, 
                             void* src, cl_int d3, void* d4, void* d5 )
{
  memcpy( dst, src, buffer_size );
  return 0;
}

//  err = clEnqueueReadBuffer(cmd_queue, lap_mem, CL_TRUE, 0, lap_buffer_size, lap, 0, NULL, NULL);
cl_int clEnqueueWriteBuffer( cmd_queue queue, cl_mem src, cl_bool d1, cl_int d2, cl_int buffer_size, 
                             void* dst, cl_int d3, void* d4, void* d5 )
{
  memcpy( dst, src, buffer_size );
  return 0;
}


//context = clCreateContext(0, 1, &devices, NULL, NULL, &err);
//  cmd_queue = clCreateCommandQueue(context, devices, 0, NULL);
cl_context clCreateContext(0, 1, &devices, NULL, NULL, cl_int* err )
{
  if( err != 0 ) (*err) = 0;
  //return
}

cl_command_queue clCreateCommandQueue( cl_context context, devices, 0, NULL)
{
}

clFinish( cl_queue queue )
{
}

//err = clGetDeviceInfo(devices, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
const cl_int CL_DEVICE_VENDOR = 0;
const cl_int CL_DEVICE_NAME = 1;
cl_int clGetDeviceInfo(devices, cl_int mode, cl_int buffer_size, cl_char* buffer, cl_int* &returned_size )
{
  const char data[2][] = { "Daniel Schwen", "EmuCL Layer" };
  strncpy( buffer, data[mode], buffer_size );
  returned_size = buffer_size < strlen(data[mode]) ? buffer_size : strlen(data[mode]);

  return 0; 
}

//program[i] = clCreateProgramWithSource(context,1, (const char**)&program_source[i], NULL, &err);
//err = clBuildProgram(program[i], 0, NULL, num_defines, NULL, NULL);
//clGetProgramBuildInfo(program[i], devices, CL_PROGRAM_BUILD_LOG, 2048, build, NULL);
//kernel[i] = clCreateKernel(program[i], label[i], &err);


//clReleaseKernel(kernel[i]);
//clReleaseProgram(program[i]);
//clReleaseCommandQueue(cmd_queue);
//clReleaseContext(context);

clReleaseMemObject( cl_mem mem )
{
  free(mem);
}


//pthread_barrier_init(&barr, NULL, THREADS );


#endif
