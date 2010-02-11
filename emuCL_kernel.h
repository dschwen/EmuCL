#ifndef EMUCL_KERNEL_H
#define EMUCL_KERNEL_H

#define __kernel ''
#define __global ''

#include "emuCL_args.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

void barrier( pthread_barrier_t* fence_barrier )
{
  // Synchronization point
  int rc = 0;//pthread_barrier_wait( fence_barrier );
  if( rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD )
  {
    printf("Could not wait on barrier\n");
    exit(-1);
  }
}

#endif
