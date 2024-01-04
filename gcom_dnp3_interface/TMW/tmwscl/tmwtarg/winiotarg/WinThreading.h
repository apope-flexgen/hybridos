/*****************************************************************************/
/* Triangle MicroWorks, Inc.                        Copyright (c) 1997-2022  */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/* (919) 870 - 6615                                                          */
/*                                                                           */
/* This Source Code and the associated Documentation contain proprietary     */
/* information of Triangle MicroWorks, Inc. and may not be copied or         */
/* distributed in any form without the written permission of Triangle        */
/* MicroWorks, Inc.  Copies of the source code may be made only for backup   */
/* purposes.                                                                 */
/*                                                                           */
/* Your License agreement may limit the installation of this source code to  */
/* specific products.  Before installing this source code on a new           */
/* application, check your license agreement to ensure it allows use on the  */
/* product in question.  Contact Triangle MicroWorks for information about   */
/* extending the number of products that may use this source code library or */
/* obtaining the newest revision.                                            */
/*                                                                           */
/*****************************************************************************/
/* file: WinThreading.h
 * description:  Critical section code
 */
#pragma once

class LockableWinTarg
{
protected:
  LockableWinTarg(const LockableWinTarg&) {}
  LockableWinTarg& operator=(const LockableWinTarg&) { return *this; }

public:
  LockableWinTarg() {}
  virtual ~LockableWinTarg() {}

  virtual void lock() = 0;
  virtual void unlock() = 0;
};

class AutoCriticalSectionWinTarg : public LockableWinTarg
{
protected:
  CRITICAL_SECTION	m_sec;

  AutoCriticalSectionWinTarg(const AutoCriticalSectionWinTarg&);
  AutoCriticalSectionWinTarg& operator=(const AutoCriticalSectionWinTarg&);

public:
  AutoCriticalSectionWinTarg()
  {
    InitializeCriticalSection(&m_sec);
  }
  virtual ~AutoCriticalSectionWinTarg()
  {
    DeleteCriticalSection(&m_sec);
  }

  virtual void lock()
  {
    EnterCriticalSection(&m_sec);
  }

  virtual void unlock()
  {
    LeaveCriticalSection(&m_sec);
  }

  const CRITICAL_SECTION* critsect() { return &m_sec; }
};

class GuardWinTarg
{
private:
  LockableWinTarg	&m_lock;

  GuardWinTarg();
  GuardWinTarg(const GuardWinTarg&);
  GuardWinTarg& operator=(const GuardWinTarg&);

  friend class UnGuardWinTarg;
public:
  GuardWinTarg(LockableWinTarg &lock)
    : m_lock(lock)
  {
    m_lock.lock();
  }
  ~GuardWinTarg()
  {
    m_lock.unlock();
  }
};

class UnGuardWinTarg
{
private:
  LockableWinTarg	&m_lock;

  UnGuardWinTarg();
  UnGuardWinTarg(const UnGuardWinTarg&);
  UnGuardWinTarg& operator=(const UnGuardWinTarg&);

public:
  UnGuardWinTarg(GuardWinTarg &GuardWinTarg)
    : m_lock(GuardWinTarg.m_lock)
  {
    m_lock.unlock();
  }
  ~UnGuardWinTarg()
  {
    m_lock.lock();
  }
};

typedef GuardWinTarg CriticalSectionLockWinTarg;

