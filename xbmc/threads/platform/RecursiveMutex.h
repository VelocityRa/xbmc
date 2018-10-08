/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#if !defined(TARGET_SWITCH)
#include <mutex>
#endif

#if (defined TARGET_POSIX)
#include <pthread.h>

namespace XbmcThreads
{
  // forward declare in preparation for the friend declaration
  class CRecursiveMutex
  {
    pthread_mutex_t m_mutex;

#if !defined TARGET_SWITCH
    // implementation is in threads/platform/pthreads/ThreadImpl.cpp
    static pthread_mutexattr_t* getRecursiveAttr();
#else
#define getRecursiveAttr() 0
#endif

  public:

    CRecursiveMutex(const CRecursiveMutex&) = delete;
    CRecursiveMutex& operator=(const CRecursiveMutex&) = delete;

    inline CRecursiveMutex() { pthread_mutex_init(&m_mutex,getRecursiveAttr()); }

    inline ~CRecursiveMutex() { pthread_mutex_destroy(&m_mutex); }

    inline void lock() { pthread_mutex_lock(&m_mutex); }

    inline void unlock() { pthread_mutex_unlock(&m_mutex); }

    inline bool try_lock() { return (pthread_mutex_trylock(&m_mutex) == 0); }
#if !defined TARGET_SWITCH
    inline std::recursive_mutex::native_handle_type native_handle()
    {
      return &m_mutex;
    }
#else
    inline pthread_mutex_t native_handle()
    {
      return m_mutex;
    }
#endif
  };
}
#elif (defined TARGET_WINDOWS)
namespace XbmcThreads
{
  typedef std::recursive_mutex CRecursiveMutex;
}
#endif

