
#ifndef EMUCL_KERNEL_H
#define EMUCL_KERNEL_H

#define __kernel ''
#define __global ''

#include "emuCL_args.h"

#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>

const int __argidmax = 50;

typedef char cl_char;
typedef void* cl_mem;
typedef int cl_int;
typedef unsigned int cl_uint;
const cl_int CL_SUCCESS = 0;

typedef bool cl_bool;
const cl_bool CL_TRUE = true;
const cl_bool CL_FALSE = false;

typedef int cl_command_queue;
typedef int cl_device_id;

struct cl_context_struct
{
  cl_int sid; 
};
typedef cl_context_struct* cl_context;

typedef void(*cl_kernel_func)(void*); // function pointer
struct cl_kernel_struct
{
  void** a;
  cl_kernel_func func;
};
typedef cl_kernel_struct* cl_kernel;

struct cl_program_struct
{
  char* f; // the name of the temporary source file
  char* b; // compile messages
};
typedef cl_program_struct* cl_program;

typedef cl_int cl_event;

//cl_int clSetKernelArg( kernel[0], 0, sizeof(cl_mem), &c_mem );
cl_int clSetKernelArg( cl_kernel kernel, cl_int id, cl_int size, void* ptr )
{
  if( id >= __argidmax ) return 1;
  kernel->a[id] = ptr;
  return CL_SUCCESS;
}

const cl_int CL_MEM_READ_ONLY = 0;
//cl_mem clCreateBuffer(context, CL_MEM_READ_ONLY, c_buffer_size, NULL, NULL);
cl_mem clCreateBuffer( cl_context, cl_int mode, cl_int buffer_size, void* d1, void* d2 )
{
  return (void*)malloc(buffer_size);
}


//clEnqueueWriteBuffer(cmd_queue, lap_mem, CL_TRUE, 0, lap_buffer_size, (void*)lap, 0, NULL, NULL);
cl_int clEnqueueWriteBuffer( cl_command_queue queue, cl_mem dst, cl_bool d1, cl_int d2, cl_int buffer_size, 
                             void* src, cl_int d3, void* d4, void* d5 )
{
  memcpy( dst, src, buffer_size );
  return CL_SUCCESS;
}

//  err = clEnqueueReadBuffer(cmd_queue, lap_mem, CL_TRUE, 0, lap_buffer_size, lap, 0, NULL, NULL);
cl_int clEnqueueReadBuffer( cl_command_queue queue, cl_mem src, cl_bool d1, cl_int d2, cl_int buffer_size, 
                             void* dst, cl_int d3, void* d4, void* d5 )
{
  memcpy( dst, src, buffer_size );
  return CL_SUCCESS;
}


//context = clCreateContext(0, 1, &devices, NULL, NULL, &err);
//  cmd_queue = clCreateCommandQueue(context, devices, 0, NULL);
cl_context clCreateContext( cl_int d1, cl_int d2, cl_device_id* devices, void* d3, void* d4, cl_int* err )
{
  if( err != 0 ) (*err) = CL_SUCCESS;

  cl_context c = (cl_context)malloc(sizeof(cl_context_struct));
  c->sid = 0;

  return c;
}

cl_command_queue clCreateCommandQueue( cl_context context, cl_device_id device, cl_int d1, void* d2 )
{
  return CL_SUCCESS;
}

cl_int clFinish( cl_command_queue queue )
{
  return CL_SUCCESS;
}

const cl_int CL_DEVICE_TYPE_GPU = 0;
const cl_int CL_DEVICE_TYPE_CPU = 0; //all the same to us
//err = clGetDeviceIDs(NULL,CL_DEVICE_TYPE_GPU, 1, &devices, NULL);
cl_int clGetDeviceIDs( void* d1, cl_int type, cl_int d2, cl_device_id *devices, void* d3 )
{
  return CL_SUCCESS;
}


//err = clGetDeviceInfo(devices, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
const cl_int CL_DEVICE_VENDOR = 0;
const cl_int CL_DEVICE_NAME = 1;
const char CL_DEVICE_DATA_STRINGS[][30] = { "Daniel Schwen", "EmuCL Layer" };
cl_int clGetDeviceInfo( cl_device_id device, cl_int mode, cl_int buffer_size, cl_char* buffer, size_t* returned_size )
{
  strncpy( buffer, CL_DEVICE_DATA_STRINGS[mode], buffer_size );
  (*returned_size) = buffer_size < strlen(CL_DEVICE_DATA_STRINGS[mode]) ? buffer_size : strlen(CL_DEVICE_DATA_STRINGS[mode]);

  return CL_SUCCESS; 
}

//program[i] = clCreateProgramWithSource(context,1, (const char**)&program_source[i], NULL, &err);
//err = clBuildProgram(program[i], 0, NULL, num_defines, NULL, NULL);
//clGetProgramBuildInfo(program[i], devices, CL_PROGRAM_BUILD_LOG, 2048, build, NULL);

cl_kernel clCreateKernel( cl_program p, const char *kernel_name, cl_int *err )
{
  cl_kernel k = (cl_kernel)malloc(sizeof(cl_kernel_struct));
  k->a = (void**)malloc( __argidmax * sizeof(void*) ); // reserve memory for kernel args

  char f[300];
  snprintf( f, 300, "%s.so", p->f );
  void* handle = dlopen( f, RTLD_LAZY);
  k->func = (cl_kernel_func)dlsym( handle, kernel_name );  

  if( err != 0 ) (*err) = CL_SUCCESS;
  return k;
}

typedef int cl_kernel_work_group_info;
const cl_kernel_work_group_info CL_KERNEL_WORK_GROUP_SIZE = 0;
cl_int clGetKernelWorkGroupInfo( cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name, 
                                 size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  if( param_name == CL_KERNEL_WORK_GROUP_SIZE )
  {
    *((size_t*)param_value) = 512;
    return CL_SUCCESS;
  }
  else
  {
    printf( "clGetKernelWorkGroupInfo param_name not implemented!\n" );
    exit(1);
  }
}


cl_program clCreateProgramWithSource( cl_context context, cl_int dummy1, const char** program_source, void* dummy2, cl_int *err)
{
  char cmd[300];
  cl_program p = (cl_program)malloc( sizeof(cl_program_struct) );
  p->f = (char*)malloc(200);
  p->b = (char*)malloc(20000);

  snprintf( p->f, 200, ".%d.%d", getpid(), context->sid++ );
  snprintf( cmd, 300, "./emuCL_process.pl > %s.c", p->f );

  FILE *proc = popen( cmd, "w" );
  fprintf( proc, *program_source ); 
  pclose(proc);

  *err = CL_SUCCESS;
  return p;
}

cl_int clBuildProgram( cl_program p, cl_int dummy1, void* dummy2, char* options, void* dummy3, void* dummy4 )
{
  char cmd[64000];

  // this should ideally be replaced by libtool (but it drives me crazy!!!!)
  snprintf( cmd, 64000, "gcc -xc -std=gnu99 -fPIC -shared %s -Wl,-soname,%s.so -o %s.so %s.c -lpthread -lm", options, p->f, p->f, p->f );
  printf( "%s\n", cmd );

  FILE *proc = popen( cmd, "r" );
  fread( p->b, 1, 20000, proc ); 
  pclose(proc);

  return 0;
}

const cl_int CL_PROGRAM_BUILD_LOG = 0;
//  clGetProgramBuildInfo(program[i], devices, CL_PROGRAM_BUILD_LOG, 2048, build, NULL);
cl_int clGetProgramBuildInfo( cl_program p, cl_device_id device, cl_int mode, size_t buffer_size, cl_char* buffer, void* d1)
{
  strncpy( buffer, p->b, buffer_size );
  return CL_SUCCESS;
}



//err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 3, NULL, global_work_size, local_work_size, 0, NULL, NULL);
cl_int clEnqueueNDRangeKernel( cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, 
                               const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size,
                               cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
  // spawn threads
  /*
    pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned count);
  */

  // wait for threads to finish

  return CL_SUCCESS;
}


cl_int clReleaseKernel( cl_kernel kernel)
{
  free(kernel);
  return CL_SUCCESS;
}

cl_int clReleaseProgram( cl_program p )
{
  char f[300];
  snprintf( f, 300, "%s.c", p->f );
  //unlink( f );
  snprintf( f, 300, "%s.so", p->f );
  //unlink( f );
  free( p->f );
  free( p->b );
  return CL_SUCCESS;
}

cl_int clReleaseCommandQueue( cl_command_queue queue )
{
  return CL_SUCCESS;
}

cl_int clReleaseContext( cl_context c )
{
  free(c);
  return CL_SUCCESS;
}

cl_int clReleaseMemObject( cl_mem mem )
{
  free(mem);
  return CL_SUCCESS;
}


//pthread_barrier_init(&barr, NULL, THREADS );


#endif
