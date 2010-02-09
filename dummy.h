#ifndef EMUCL_KERNEL_H
#define EMUCL_KERNEL_H

#define __kernel ''
#define __global ''

#include "emucl_args.h"
#include <pthreads.h>
#include <malloc.h>

const int __argidmax = 50;

typedef char cl_char;
typedef int cl_int;
typedef void* cl_mem;

typedef bool cl_bool;


struct cl_kernel
{
  __argstruct args;
}

//cl_int clSetKernelArg( kernel[0], 0, sizeof(cl_mem), &c_mem );

cl_int clSetKernelArg( cl_kernel kernel, cl_int id, cl_int size, void* ptr )
{
  if( id >= __argidmax ) return 1;

  kernel.args.arg[id] = ptr;
  return 0;
}

//clEnqueueWriteBuffer(cmd_queue, lap_mem, CL_TRUE, 0, lap_buffer_size, (void*)lap, 0, NULL, NULL);
cl_int clEnqueueWriteBuffer( cmd_queue, cl_mem mem, CL_TRUE, 0, lap_buffer_size, (void*)lap, 0, NULL, NULL);



//pthread_barrier_init(&barr, NULL, THREADS );


#endif
