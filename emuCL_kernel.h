#define __kernel ''
#define __global ''

#include <pthreads.h>

struct __argstruct {
  int get_global_id[3];
  int get_local_id[3];
  pthread_barrier_t* fence_barrier;
  pthread_mutex_t* fence_mutex;
  pthread_cond_t* fence_cond;
  void **a;
};

void barrier( pthread_barrier_t* fence_barrier )
{
}
