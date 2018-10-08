#ifndef _PTHREAD_DEFINES_H_
#define _PTHREAD_DEFINES_H_

#include <switch/kernel/thread.h>
#include <switch/kernel/condvar.h>

typedef Thread* pthread_t;
typedef void* pthread_mutex_t;
typedef void* pthread_mutexattr_t;
typedef int pthread_attr_t;
typedef CondVar pthread_cond_t;
typedef int pthread_condattr_t;

#endif
