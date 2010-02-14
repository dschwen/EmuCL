
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
typedef void*(*cl_kernel_func)(void*); // function pointer
struct cl_kernel_struct
{
  void** a;
  cl_kernel_func func;
  void* shared;
};
typedef cl_kernel_struct* cl_kernel;

struct cl_program_struct
{
  char* f; // the name of the temporary source file
  char* b; // compile messages
};
typedef cl_program_struct* cl_program;

typedef cl_int cl_event;

// register an argument to be passed to the kernel
// arguments are stored as pointers in the kernel datastructure
cl_int clSetKernelArg( cl_kernel kernel, cl_int id, cl_int size, void* ptr )
{
  if( id >= __argidmax ) return 1;

  // reserve shared memory
  if( ptr == NULL )
  {
    kernel->shared = realloc( kernel->shared, size );
    ptr = kernel->shared;
  }

  kernel->a[id] = ptr;
  return CL_SUCCESS;
}

const cl_int CL_MEM_READ_ONLY = 0;
// create a memory buffer 
cl_mem clCreateBuffer( cl_context, cl_int mode, cl_int buffer_size, void* d1, void* d2 )
{
  return (void*)malloc(buffer_size);
}


// copy data into the memory buffer
cl_int clEnqueueWriteBuffer( cl_command_queue queue, cl_mem dst, cl_bool d1, cl_int d2, cl_int buffer_size, 
                             void* src, cl_int d3, void* d4, void* d5 )
{
  memcpy( dst, src, buffer_size );
  return CL_SUCCESS;
}

// copy data out of memory buffer
cl_int clEnqueueReadBuffer( cl_command_queue queue, cl_mem src, cl_bool d1, cl_int d2, cl_int buffer_size, 
                             void* dst, cl_int d3, void* d4, void* d5 )
{
  memcpy( dst, src, buffer_size );
  return CL_SUCCESS;
}


// 
cl_context clCreateContext( cl_int d1, cl_int d2, cl_device_id* devices, void* d3, void* d4, cl_int* err )
{
  if( err != 0 ) (*err) = CL_SUCCESS;

  cl_context c = (cl_context)malloc(sizeof(cl_context_struct));
  c->sid = 0;

  return c;
}

// this doesn't do anything yet as all commands are immediately executed rather than queued
cl_command_queue clCreateCommandQueue( cl_context context, cl_device_id device, cl_int d1, void* d2 )
{
  return CL_SUCCESS;
}

// should wait for jobs in the queue to finish, but there are never any
cl_int clFinish( cl_command_queue queue )
{
  return CL_SUCCESS;
}

const cl_int CL_DEVICE_TYPE_GPU = 0;
const cl_int CL_DEVICE_TYPE_CPU = 0; //all the same to us
// TODO: return exactly one device here. ID won't matter
cl_int clGetDeviceIDs( void* d1, cl_int type, cl_int d2, cl_device_id *devices, void* d3 )
{
  return CL_SUCCESS;
}


const cl_int CL_DEVICE_VENDOR = 0;
const cl_int CL_DEVICE_NAME = 1;
const char CL_DEVICE_DATA_STRINGS[][30] = { "Daniel Schwen", "EmuCL Layer" };
// whatever ID is queried, it is always the EmuCL device
cl_int clGetDeviceInfo( cl_device_id device, cl_int mode, cl_int buffer_size, cl_char* buffer, size_t* returned_size )
{
  strncpy( buffer, CL_DEVICE_DATA_STRINGS[mode], buffer_size );
  (*returned_size) = buffer_size < strlen(CL_DEVICE_DATA_STRINGS[mode]) ? buffer_size : strlen(CL_DEVICE_DATA_STRINGS[mode]);

  return CL_SUCCESS; 
}

// reserve room for the kernel datastructure and load compiled shared lib
cl_kernel clCreateKernel( cl_program p, const char *kernel_name, cl_int *err )
{
  cl_kernel k = (cl_kernel)malloc(sizeof(cl_kernel_struct));
  k->a = (void**)malloc( __argidmax * sizeof(void*) ); // reserve memory for kernel args
  k->shared = NULL;

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


// choose filenames for the temporary files, run kernel source code through EmuCL-preprocessor
// and write to disk
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

// create temporary shared lib from preprocessed code, save compiler output
cl_int clBuildProgram( cl_program p, cl_int dummy1, void* dummy2, char* options, void* dummy3, void* dummy4 )
{
  char cmd[64000];

  // this should ideally be replaced by libtool (but it drives me crazy!!!!)
  snprintf( cmd, 64000, "gcc -xc -std=gnu99 -fPIC -shared %s -Wl,-soname,%s.so -o %s.so %s.c -lpthread -lm", options, p->f, p->f, p->f ); // 2>&1 ?
  printf( "%s\n", cmd );

  FILE *proc = popen( cmd, "r" );
  fread( p->b, 1, 20000, proc ); 
  pclose(proc);

  return 0;
}

const cl_int CL_PROGRAM_BUILD_LOG = 0;
// make buffered compiler output available
cl_int clGetProgramBuildInfo( cl_program p, cl_device_id device, cl_int mode, size_t buffer_size, cl_char* buffer, void* d1)
{
  strncpy( buffer, p->b, buffer_size );
  return CL_SUCCESS;
}



// TODO: spawn threads for the kernels
cl_int clEnqueueNDRangeKernel( cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, 
                               const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size,
                               cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
  if( work_dim < 1 || work_dim > 3 )
  {
    fprintf( stderr, "Illegal work_dim in clEnqueueNDRangeKernel()\n" );
    exit(1);
  }
fprintf( stderr, "* clEnqueueNDRangeKernel\n" );
  emuCL_argstruct* arg_array;
  

  int i , j[3], k, l[3] = { 1, 1, 1 }, m[3], n = 1, n2, w[3] = { 1, 1, 1 }, g[3] = { 1, 1, 1 };
  for(i = 0; i < work_dim; i++ )
  {
    l[i] = ( global_work_size[i] + local_work_size[i] - 1 ) / local_work_size[i];
    w[i] = local_work_size[i];
    g[i] = global_work_size[i];
    n *= l[i];
  }
fprintf( stderr, " * calculated worksizes\n" );

  pthread_barrier_t fence_barrier;
  pthread_t* thr;

  // initialize argument structures
  arg_array = (emuCL_argstruct*)malloc( n * sizeof(emuCL_argstruct) );
  thr = (pthread_t*)malloc( n * sizeof(pthread_t) );
  k = 0;
fprintf( stderr, " * allocated memory\n" );

  for( m[0] = 1; m[0] < w[0]; m[0]++ )
    for( m[1] = 1; m[1] < w[1]; m[1]++ )
      for( m[2] = 1; m[2] < w[2]; m[2]++ )
      {
        arg_array[k].fence_barrier_p = &fence_barrier;
        arg_array[k].a = kernel->a;
        for(i = 0; i < work_dim; i++ )
        {
          arg_array[k].get_local_id[i] = m[i];
          arg_array[k].get_local_size[i] = w[i];
          arg_array[k].get_global_size[i] = g[i];
        }
        k++;
      }
fprintf( stderr, " * initialized arguments\n" );

  // spawn threads
  for( j[0] = 1; j[0] < l[0]; j[0]++ ) // iterate over work-groups
    for( j[1] = 1; j[1] < l[1]; j[1]++ )
      for( j[2] = 1; j[2] < l[2]; j[2]++ )
      {
        // count threads and initialize global IDs
        n2 = 0;
        for( k = 0; k < n; k++ )
        {
          arg_array[k].spawn = true;
          for(i = 0; i < work_dim; i++ )
          {
            arg_array[k].get_global_id[i] = m[i] + j[i]*w[i];
            if( arg_array[k].get_global_id[i] >= g[i] )
            {
              arg_array[k].spawn = false;
              break;
            }
          }
          if( arg_array[k].spawn ) n2++;
        }

        // initialize the barrier accordingly
        pthread_barrier_init( &fence_barrier, NULL, n2 );
fprintf( stderr, " * initialized barrier\n" );

        // now actually spawn the threads
        for( k = 0; k < n; k++ )
          if( arg_array[k].spawn )
          {
fprintf( stderr, " * spawning thread #%d\n", k );
            
            if( pthread_create(&thr[k], NULL, kernel->func, NULL) )
            {
              fprintf( stderr, "Could not create thread\n" );
              exit(1);
            }
          }

        // now wait for all running threads in the work group to finish
        for( k = 0; k < n; k++ )
          if( arg_array[k].spawn )
            pthread_join( thr[k], NULL );
      }

  return CL_SUCCESS;
}

// clean-up
cl_int clReleaseKernel( cl_kernel kernel)
{
  free(kernel);
  return CL_SUCCESS;
}

// clean-up
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

// clean-up (does nothing yet)
cl_int clReleaseCommandQueue( cl_command_queue queue )
{
  return CL_SUCCESS;
}

// clean-up
cl_int clReleaseContext( cl_context c )
{
  free(c);
  return CL_SUCCESS;
}

// clean-up
cl_int clReleaseMemObject( cl_mem mem )
{
  free(mem);
  return CL_SUCCESS;
}




#endif
