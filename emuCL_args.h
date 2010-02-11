#ifndef EMUCL_ARGS_H
#define EMUCL_ARGS_H

#include <pthread.h>

struct emuCL_sargstruct {
  int get_global_id[3];
  int get_local_id[3];
  pthread_barrier_t* fence_barrier;
  pthread_mutex_t* fence_mutex;
  pthread_cond_t* fence_cond;
  void **a;
};
typedef struct emuCL_sargstruct emuCL_argstruct;

#endif
