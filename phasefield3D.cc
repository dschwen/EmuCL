#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

//#include <OpenCL/opencl.h>
#include "OpenCL.h"

inline int sqr( int a ) { return a*a; }

char * load_program_source(const char *filename)
{
  struct stat statbuf;
  FILE *fh; 
  char *source; 
	
  fh = fopen(filename, "r");
  if (fh == 0)
    return 0; 
	
  stat(filename, &statbuf);
  source = (char *) malloc(statbuf.st_size + 1);
  fread(source, statbuf.st_size, 1, fh);
  source[statbuf.st_size] = '\0'; 
	
  return source; 
} 

const int ndmax = 1000;
char num_defines[ndmax];
void add_define_int( const char *name, int val )
{
  char buf[ndmax];
  snprintf( buf, 1000, "-D %s=%d ", name, val );
  if( strlen(buf) + strlen(num_defines) >= ndmax-2 )
  {
    printf( "out of constant string mem!\n" );
    exit(1);
  }
  strcat( num_defines, buf );
} 
void add_define_float( const char *name, double val )
{
  char buf[ndmax];
  snprintf( buf, 1000, "-D %s=%f ", name, val );
  if( strlen(buf) + strlen(num_defines) >= ndmax-2 )
  {
    printf( "out of constant string mem!\n" );
    exit(1);
  }
  strcat( num_defines, buf );
} 

int main (int argc, const char * argv[]) 
{
  int size = 128; 
  int size2 = size*size;
  int size3 = size2*size;

  const int itmax = 200;

  float *c, *lap;

  char *filename[2] = { "laplace.cl", "flux.cl" };
  char *label[2] = { "lap3d", "flux3d" };
  num_defines[0] = 0;
	
  //Allocate data storage
  size_t c_buffer_size = sizeof(float) * size3 * 2;
  c = (float*)calloc( size3*2, sizeof(float) );
  size_t lap_buffer_size = sizeof(float) * size3;
  lap = (float*)calloc( size3, sizeof(float) );
	
  // initialize data (only dir=0)
  for( int x = 0; x < size; x++ ) 
    for( int y = 0; y < size; y++ ) 
      for( int z = 0; z < size; z++ ) 
      {
        c[ x + y*size + z*size2 ] = 0.4;

        if( x >= 54 && x <= 64 && y >= 54 && y <= 64 && z >= 54 && z <= 64 )
          c[ x + y*size + z*size2 ] = 0.6;
        if( sqrt( sqr(x-29) + sqr(y-29) + sqr(z-29) ) <= 10.0 )
          c[ x + y*size + z*size2 ] = 0.6;
      }


  // calculate constants needed at kernel compile-time
 
  //     atomic_diameter=1.0 !angtroms
  float atomic_diameter = 1.0; 

  //     ns=1. !number of atoms in each face of the cells
  //     nv=ns**2 !total number of atoms in each cell
  float ns = 1.0;
  float nv = ns*ns*ns;

  //     factor1=ns/nv
  //     factor2=1./nv
  float factor1 = ns/nv;
  float factor2 = 1.0/nv;

  //     dx=ns*atomic_diameter !dimension of the cells (angtroms)
  float dx = ns*atomic_diameter;

  //     dt=5.e18 !time step (sec)
  float dt = 5.e18;

  //     k=8.617385692e-2 !meV/K
  //     T=700. !temperature K
  //     kT=k*T
  float kB = 8.617385692e-2;
  float T = 700.0;
  float kT = kB*T;

  //     nu= 1.e23 !frecuency of jumping 
  float nu = 1.0e23;

  //     gammaB=0.
  float gammaB = 0.0;

  //     z=1 !coordination number in adjacent planes
  //     z0=4 !coordination number in plane "n"
  float z = 1.0;
  float z0 = 4.0;

  //     Eaa=723.3 !binding energy of pair AA (meV)
  //     Ebb=723.3 !binding energy of pair BB (meV)
  float Eaa = 723.3;
  float Ebb = 723.3;

  //     omega=67.78 !meV
  float omega = 67.78;

  //     Eab=omega+(Eaa+Ebb)/2. !binding energy of pair AB (meV)
  float Eab = omega + (Eaa+Ebb) / 2.0;

  //     fac1=z0*(Eab+Eaa)/kT
  //     fac2=z0/kT
  //     fac3=z*factor2/kT
  float fac1 = z0 * (Eab+Eaa) / kT;
  float fac2 = z0 / kT;
  float fac3 = z * factor2/kT;

  // add define: SIZE, SIZE2, SIZE3
  add_define_int( "SIZE", size );
  add_define_int( "SIZE2", size2 );
  add_define_int( "SIZE3", size3 );

  fac1 = fac1 - log(nu) - log(dt); gammaB *= dt; dt = 1.0; // careful when changing dt!!!!!!!!!!!

  // output defines
  printf( "%s\n", num_defines );

  //GPU - Unoptimized calculation
  cl_context context;
  cl_command_queue cmd_queue;
  cl_device_id devices;

  cl_int err;
	
  // Connect to a compute device
  err = clGetDeviceIDs(NULL,CL_DEVICE_TYPE_GPU, 1, &devices, NULL);
	
  size_t returned_size = 0;
  cl_char vendor_name[1024] = {0};
  cl_char device_name[1024] = {0};
  err = clGetDeviceInfo(devices, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
  err|= clGetDeviceInfo(devices, CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
	
  printf("Connecting to %s %s...\n", vendor_name, device_name);
	
  char *program_source[2];

  //Create the context and command queue
  context = clCreateContext(0, 1, &devices, NULL, NULL, &err);
  printf("err=%d\n",err);
  cmd_queue = clCreateCommandQueue(context, devices, 0, NULL);
	
  //Allocate memory for programs and kernels
  cl_program program[2];
  cl_kernel kernel[2];

  // build kernels
  for( int i = 0; i < 2; i++ )
  {
    // Read the program
    printf("Loading program '%s'\n\n", filename[i]);
    program_source[i] = load_program_source(filename[i]);
    
    //Create program from .cl file
    program[i] = clCreateProgramWithSource(context,1, (const char**)&program_source[i], NULL, &err);
    printf("err=%d\n",err);
	
    // build the program (compile it)
    err = clBuildProgram(program[i], 0, NULL, num_defines, NULL, NULL);
    printf("err=%d\n",err);
    char build[2048];
    clGetProgramBuildInfo(program[i], devices, CL_PROGRAM_BUILD_LOG, 2048, build, NULL);
    printf("Build Log:\n%s\n",build);
	
    // create the kernel 
    kernel[i] = clCreateKernel(program[i], label[i], &err);
    printf("err=%d\n",err);
  }

  double cl_alloc, cl_enqueue, cl_read;
	
	
  //Allocate memory and queue it to be written to the device
  cl_mem c_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, c_buffer_size, NULL, NULL);
  err = clEnqueueWriteBuffer(cmd_queue, c_mem, CL_TRUE, 0, c_buffer_size, (void*)c, 0, NULL, NULL);
  cl_mem lap_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, lap_buffer_size, NULL, NULL);
  //err = clEnqueueWriteBuffer(cmd_queue, lap_mem, CL_TRUE, 0, lap_buffer_size, (void*)lap, 0, NULL, NULL);
  printf("err=%d\n",err);
	
  //Push the data out to device
  clFinish(cmd_queue);
	
  // set work-item dimensions 
  size_t global_work_size[3], local_work_size[3], shared_size[2];
  global_work_size[0] = size;
  global_work_size[1] = size;
  global_work_size[2] = size;
  local_work_size[0] = 4;
  local_work_size[1] = 4;
  local_work_size[2] = 4;

  shared_size[0] = ( (local_work_size[0]+2) * (local_work_size[1]+2) * (local_work_size[2]+2) ) * sizeof(float);
  shared_size[1] = 5 * ( (local_work_size[0]+2) * (local_work_size[1]+2) * (local_work_size[2]+2) ) * sizeof(float);
	
  //Set kernel arguments (laplace kernel)
  //  __global float *c, float D, int size, int size2, int size3, int dir, __local float * shared  )
  err  = clSetKernelArg( kernel[0], 0, sizeof(cl_mem), &c_mem );
  err |= clSetKernelArg( kernel[0], 1, sizeof(cl_mem), &lap_mem );
  //                                2 is dir
  err |= clSetKernelArg( kernel[0], 3, shared_size[0], NULL );
  printf("err(lap_init)=%d\n",err);

  //Set kernel arguments (flux kernel)
  //  __global float *c, float D, int dir, float dt, __local float * shared  )
  err  = clSetKernelArg( kernel[1], 0, sizeof(cl_mem), &c_mem );
  err  = clSetKernelArg( kernel[1], 1, sizeof(cl_mem), &lap_mem );
  //                                2 is dir
  err |= clSetKernelArg( kernel[1], 3, sizeof(float), &dt );
  err |= clSetKernelArg( kernel[1], 4, shared_size[1], NULL );
  //float EAA, float EAB, float EBB, float FAC1,  float FAC2, float FAC3, float FACTOR1, float GB, float Z 
  err |= clSetKernelArg( kernel[1], 5, sizeof(float), &Eaa );
  err |= clSetKernelArg( kernel[1], 6, sizeof(float), &Eab );
  err |= clSetKernelArg( kernel[1], 7, sizeof(float), &Ebb );
  err |= clSetKernelArg( kernel[1], 8, sizeof(float), &fac1 );
  err |= clSetKernelArg( kernel[1], 9, sizeof(float), &fac2 );
  err |= clSetKernelArg( kernel[1], 10, sizeof(float), &fac3 );
  err |= clSetKernelArg( kernel[1], 11, sizeof(float), &factor1 );
  err |= clSetKernelArg( kernel[1], 12, sizeof(float), &gammaB );
  err |= clSetKernelArg( kernel[1], 13, sizeof(float), &z );

  printf("err(fxl_init)=%d\n",err);
	
  size_t thread_size;
  clGetKernelWorkGroupInfo(kernel[0],devices,CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),&thread_size,NULL);
  printf("Recommended Size: %lu\n",thread_size);
	
  //Queue up the kernels itmax times
  int dir;
  printf( "executing kernels...\n" );
  for( int i = 0; i < itmax; i++ )
  {
    dir = i % 2;

    // calculate laplacians
    err |= clSetKernelArg( kernel[0], 2, sizeof(int), &dir );
    err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 3, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    //printf("err(lap)=%d\n",err);

    // calculate fluxes
    err |= clSetKernelArg( kernel[1], 2, sizeof(int), &dir );
    err = clEnqueueNDRangeKernel(cmd_queue, kernel[1], 3, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    //printf("err(flx)=%d\n",err);
    
    if( i % 10 == 9 ) printf( "%d steps...\n", i+1 );
  }
  printf( "done.\n" );

  //Finish the calculation
  clFinish(cmd_queue);
	
  // read output image
  printf( "transferring output...\n" );
  err = clEnqueueReadBuffer(cmd_queue, c_mem, CL_TRUE, 0, c_buffer_size, c, 0, NULL, NULL);
  printf("err=%d\n",err);
  clFinish(cmd_queue);
  printf( "done.\n" );

  err = clEnqueueReadBuffer(cmd_queue, lap_mem, CL_TRUE, 0, lap_buffer_size, lap, 0, NULL, NULL);
  printf("err=%d\n",err);
  clFinish(cmd_queue);

  //for( int x = 0; x < size/2; x++ ) 
    //printf( "%f\n", c[ size/2 + size2/2 + size2*x + (itmax%2)*size3 ] );

  FILE *out = fopen("gpu_result.dat", "wt" ); 
  for( int x = 0; x < size; x++ ) 
  {
    for( int y = 0; y < size; y++ ) 
      //fprintf( out, "%d %d %f\n", x, y, lap[ x + size*y + size3/2 ] );
      fprintf( out, "%d %d %f\n", x, y, c[ x + size*y + size3/2 + (itmax%2)*size3 ] );
    fprintf( out, "\n" );
  }
  fclose(out);

  // release kernel, program, and memory objects
  for( int i = 0; i < 2; i++ )
  {
    clReleaseKernel(kernel[i]);
    clReleaseProgram(program[i]);
  }

  clReleaseCommandQueue(cmd_queue);
  clReleaseContext(context);
	
  clReleaseMemObject(c_mem);
  clReleaseMemObject(lap_mem);
	
  //Clean up
  free(c);
  free(lap);
	
  return 0;
}












