/* Copyright  (C) 2018 - M4xw <m4x@m4xw.net>, RetroArch Team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (switch_pthread.h).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _SWITCH_PTHREAD_
#define _SWITCH_PTHREAD_

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// #include <algorithm>
// #include <vector>

#include <switch.h>
#include <switch/kernel/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIZEOF_PTHREAD_T 8

#define PTHREAD_SCOPE_SYSTEM 0
#define PTHREAD_MUTEX_DEFAULT 1
#define PTHREAD_MUTEX_NORMAL 1
#define PTHREAD_SCOPE_PROCESS 1
#define PTHREAD_MUTEX_RECURSIVE 2
#define PTHREAD_RWLOCK_DEFAULT_NP 2
#define PTHREAD_MUTEX_ERRORCHECK 3
#define pthread_cleanup_pop(execute)                                                               \
  _pthread_cleanup_pop(&_buffer, (execute));                                                       \
  }
#define __LOCK_INITIALIZER                                                                         \
  {                                                                                                \
    0, 0                                                                                           \
  }
#define PTHREAD_RWLOCK_INITIALIZER                                                                 \
  {                                                                                                \
    __LOCK_INITIALIZER, 0, NULL, NULL, NULL, PTHREAD_RWLOCK_DEFAULT_NP, PTHREAD_PROCESS_PRIVATE    \
  }
#define PTHREAD_MUTEX_INITIALIZER                                                                  \
  {                                                                                                \
    0, 0, 0, PTHREAD_MUTEX_NORMAL, __LOCK_INITIALIZER                                              \
  }
#define pthread_cleanup_push(routine, arg)                                                         \
  {                                                                                                \
    struct _pthread_cleanup_buffer _buffer;                                                        \
    _pthread_cleanup_push(&_buffer, (routine), (arg));
#define PTHREAD_COND_INITIALIZER                                                                   \
  {                                                                                                \
    __LOCK_INITIALIZER, 0                                                                          \
  }

#if defined(_WIN32) || defined(__INTEL_COMPILER)
#define INLINE __inline
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define INLINE inline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

#define THREADVARS_MAGIC 0x21545624 /* !TV$ */
int pthread_create(pthread_t* thread,
                   const pthread_attr_t* attr,
                   void* (*start_routine)(void*),
                   void* arg);
void pthread_exit(void* retval);

/* This structure is exactly 0x20 bytes, if more is needed modify getThreadVars() below */
typedef struct
{
  /* Magic value used to check if the struct is initialized */
  u32 magic;

  /* Thread handle, for mutexes */
  Handle handle;

  /* Pointer to the current thread (if exists) */
  Thread* thread_ptr;

  /* Pointer to this thread's newlib state */
  struct _reent* reent;

  /* Pointer to this thread's thread-local segment */
  void* tls_tp; /* !! Offset needs to be TLS+0x1F8 for __aarch64_read_tp !! */
} ThreadVars;

static INLINE ThreadVars* getThreadVars(void)
{
  return (ThreadVars*)((u8*)armGetTls() + 0x1E0);
}

static INLINE Thread* threadGetCurrent(void)
{
  ThreadVars* tv = getThreadVars();
  return tv->thread_ptr;
}

static INLINE pthread_t pthread_self(void)
{
  return threadGetCurrent();
}

// static std::vector<void*> recursive_mutexes;

static bool is_mutex_recursive(void* mutex)
{
  return true;
  // return std::find(recursive_mutexes.begin(), recursive_mutexes.end(), mutex) != recursive_mutexes.end();
}

static INLINE int pthread_mutex_init(pthread_mutex_t* mutex, pthread_mutexattr_t* attr)
{
  // if (*(int*)attr & 1)
    // recursive_mutexes.push_back((void*)mutex);
  
  // TODO: remove
  // if (*(int*)attr & 1) assert(is_mutex_recursive(mutex));

  if (is_mutex_recursive(mutex))
    rmutexInit((RMutex*)mutex);
  else
    mutexInit((Mutex*)mutex);

  return 0;
}

static INLINE int pthread_mutex_destroy(pthread_mutex_t* mutex)
{
  /* Nothing */
  *mutex = 0;

  return 0;
}

static INLINE int pthread_mutex_lock(pthread_mutex_t* mutex)
{
  if (is_mutex_recursive(mutex))
    rmutexLock((RMutex*)mutex);
  else
    mutexLock((Mutex*)mutex);
  return 0;
}

static INLINE int pthread_mutex_trylock(pthread_mutex_t* mutex)
{
  if (is_mutex_recursive(mutex))
    return rmutexTryLock((RMutex*)mutex) ? 0 : 1;
  else
    return mutexTryLock((Mutex*)mutex) ? 0 : 1;     
}

static INLINE int pthread_mutex_unlock(pthread_mutex_t* mutex)
{
  if (is_mutex_recursive(mutex))
    rmutexUnlock((RMutex*)mutex);
  else
    mutexUnlock((Mutex*)mutex);
  return 0;
}

INLINE int pthread_detach(pthread_t thread)
{
  (void)thread;
  /* Nothing for now */
  return 0;
}

static INLINE int pthread_join(pthread_t thread, void** retval)
{
  printf("[Threading]: Waiting for Thread Exit\n");
  threadWaitForExit(thread);
  threadClose(thread);

  return 0;
}

static INLINE int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
  if (is_mutex_recursive(mutex))
    printf("[Threading]: ERROR: Used recursive mutex in condvar!\n");

  condvarWait(cond, (Mutex*)mutex);

  return 0;
}

static INLINE int pthread_cond_timedwait(pthread_cond_t* cond,
                                         pthread_mutex_t* mutex,
                                         const struct timespec* abstime)
{
  if (is_mutex_recursive(mutex))
    printf("[Threading]: ERROR: Used recursive mutex in condvar!\n");

  condvarWaitTimeout(cond, (Mutex*)mutex, abstime->tv_nsec);

  return 0;
}

static INLINE int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr)
{
  condvarInit(cond);

  return 0;
}

static INLINE int pthread_cond_signal(pthread_cond_t* cond)
{
  condvarWakeOne(cond);
  return 0;
}

static INLINE int pthread_cond_broadcast(pthread_cond_t* cond)
{
  condvarWakeAll(cond);
  return 0;
}

INLINE int pthread_cond_destroy(pthread_cond_t* cond)
{
  /* Nothing */
  return 0;
}

INLINE int pthread_equal(pthread_t t1, pthread_t t2)
{
  if (t1->handle == t2->handle)
    return 1;

  return 0;
}

#ifdef __cplusplus
}
#endif

#endif
