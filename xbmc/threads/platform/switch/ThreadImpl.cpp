/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include <limits.h>
#include <sys/resource.h>
#include <string.h>

#include <signal.h>
#include "utils/log.h"

void CThread::SpawnThread(unsigned stacksize)
{
  // pthread_attr_t attr;
  // pthread_attr_init(&attr);
  // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  // if (pthread_create(&m_ThreadId, &attr, (void*(*)(void*))staticThread, this) != 0)
  if (pthread_create((Thread**)&m_ThreadId, NULL, (void*(*)(void*))staticThread, this) != 0)
  {
    CLog::Log(LOGNOTICE, "%s - fatal error creating thread",__FUNCTION__);
  }
  // pthread_attr_destroy(&attr);
}

void CThread::TermHandler() { }

void CThread::SetThreadInfo()
{
    CLog::Log(LOGNOTICE, "%s called",__FUNCTION__);
}

ThreadIdentifier CThread::GetCurrentThreadId()
{
  return pthread_self()->handle;
}

bool CThread::IsCurrentThread(const ThreadIdentifier tid)
{
  return GetCurrentThreadId() == tid;
}

int CThread::GetMinPriority(void)
{
  // one level lower than application
  return -1;
}

int CThread::GetMaxPriority(void)
{
  // one level higher than application
  return 1;
}

int CThread::GetNormalPriority(void)
{
  // same level as application
  return 0;
}

bool CThread::SetPriority(const int iPriority)
{
  CLog::Log(LOGWARNING, "%s called", __FUNCTION__);
  return true;
}

int CThread::GetPriority()
{
  return 1;
}

bool CThread::WaitForThreadExit(unsigned int milliseconds)
{
  return threadWaitForExit(pthread_self()) != 0;
}

int64_t CThread::GetAbsoluteUsage()
{
  return 0;
}

float CThread::GetRelativeUsage()
{
  return 0.0f;
}

void term_handler (int signum)
{
}

void CThread::SetSignalHandlers()
{
}

