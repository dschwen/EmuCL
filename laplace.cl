// work group SIZE is 4*4*4
// shared mem is 6*6*6

#ifdef EMUCL
#include <stdio.h>
#endif

__kernel void lap3d(  __global float *c, __global float *lap, int dir, __local float * shared  )
{
  int xg = get_global_id(0);
  int yg = get_global_id(1);
  int zg = get_global_id(2);

  int xl = get_local_id(0);
  int yl = get_local_id(1);
  int zl = get_local_id(2);

  #ifdef EMUCL
  printf(" global: (%d,%d,%d) local: (%d,%d,%d)\n", xg,yg,zg, xl,yl,zl );
  #endif  

  int a = 1+6+36 + xl + yl*6 + zl*36;

  //
  // copy self
  //

  // needs define SIZE, SIZE2, SIZE3
  #ifdef EMUCL
  printf(" before shared(%x)[%d] =c(%x)[%d] \n", (void*)shared, a, (void*)c,  xg + yg*SIZE + zg*SIZE2 + dir*SIZE3  );
  #endif  
  shared[a] = c[ xg + yg*SIZE + zg*SIZE2 + dir*SIZE3 ];
  #ifdef EMUCL
  printf(" after shared[]\n" );
  #endif  

  //
  // copy borders
  //
  if( xl == 0 )
    shared[a-1] = c[ ( (xg-1) & (SIZE-1) ) + yg*SIZE + zg*SIZE2 + dir*SIZE3 ];
  if( xl == 3 )
    shared[a+1] = c[ ( (xg+1) & (SIZE-1) ) + yg*SIZE + zg*SIZE2 + dir*SIZE3 ];

  if( yl == 0 )
    shared[a-6] = c[ xg + ( (yg-1) & (SIZE-1) )*SIZE + zg*SIZE2 + dir*SIZE3 ];
  if( yl == 3 )
    shared[a+6] = c[ xg + ( (yg+1) & (SIZE-1) )*SIZE + zg*SIZE2 + dir*SIZE3 ];
  
  if( zl == 0 )
    shared[a-36] = c[ xg + yg*SIZE + ( (zg-1) & (SIZE-1) )*SIZE2 + dir*SIZE3 ];
  if( zl == 3 )
    shared[a+36] = c[ xg + yg*SIZE + ( (zg+1) & (SIZE-1) )*SIZE2 + dir*SIZE3 ];

  //
  // wait for copying to be finished in every worgroup member
  //
  barrier(CLK_LOCAL_MEM_FENCE);

  //
  // write result to laplacian buffer
  //
  lap[ xg + yg*SIZE + zg*SIZE2 ] = ( shared[a-1] + shared[a+1] +
                                     shared[a-6] + shared[a+6] +  
                                     shared[a-36]+ shared[a+36] - 6.0 * shared[a] );
}
