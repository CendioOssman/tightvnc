// Copyright (C) 2009,2010,2011,2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include "DesktopServerWatcher.h"
#include "win-system/Environment.h"
#include "util/Exception.h"
#include "log-server/Log.h"
#include "server-config-lib/Configurator.h"
#include "win-system/CurrentConsoleProcess.h"
#include "win-system/AnonymousPipeFactory.h"
#include "win-system/WTS.h"
#include "win-system/WinStaLibrary.h"

#include <time.h>

DesktopServerWatcher::DesktopServerWatcher(ReconnectionListener *recListener)
: m_recListener(recListener),
  m_process(0),
  m_sharedMem(0),
  m_shMemName(_T("Global\\"))
{
  // Desktop server folder.
  StringStorage currentModulePath;
  Environment::getCurrentModulePath(&currentModulePath);

  // Path to desktop server application.
  StringStorage path;
  // FIXME: To think: is quotes needed?
  path.format(_T("\"%s\""), currentModulePath.getString());

  try {
    m_process = new CurrentConsoleProcess(path.getString());

    /* Bytes order:
    (UINT64 *)[0] - memory ready flag
    (UINT64 *)[1] - otherSidePipeChanTo write handle;
    (UINT64 *)[2] - otherSidePipeChanTo read handle;
    (UINT64 *)[3] - otherSidePipeChanFrom write handle;
    (UINT64 *)[4] - otherSidePipeChanFrom read handle;
    */
    srand((unsigned)time(0));
    for (int i = 0; i < 20; i++) {
      m_shMemName.appendChar('a' + rand() % ('z' - 'a'));
    }
    m_sharedMem = new SharedMemory(m_shMemName.getString(), 72);
  } catch (...) {
    if (m_process) delete m_process;
    if (m_sharedMem) delete m_sharedMem;
    throw;
  }

  // Start thread execution.
  resume();
}

DesktopServerWatcher::~DesktopServerWatcher()
{
  terminate();
  wait();
  delete m_process;
  delete m_sharedMem;
}

void DesktopServerWatcher::execute()
{
  AnonymousPipeFactory pipeFactory;
  AnonymousPipe *ownSidePipeChanTo, *otherSidePipeChanTo,
                *ownSidePipeChanFrom, *otherSidePipeChanFrom;

  UINT64 *mem = (UINT64 *)m_sharedMem->getMemPointer();

  while (!isTerminating()) {
    try {
      // Sets memory ready flag to false.
      mem[0] = 0;

      ownSidePipeChanTo = otherSidePipeChanTo =
      ownSidePipeChanFrom = otherSidePipeChanFrom = 0;

      pipeFactory.generatePipes(&ownSidePipeChanTo, false,
                                &otherSidePipeChanTo, false);
      pipeFactory.generatePipes(&ownSidePipeChanFrom, false,
                                &otherSidePipeChanFrom, false);

      // TightVNC server log directory.
      StringStorage logDir;
      Configurator::getInstance()->getServerConfig()->getLogFileDir(&logDir);

      // Arguments that must be passed to desktop server application.
      StringStorage args;
      args.format(_T("-desktopserver -logdir \"%s\" -loglevel %d -shmemname %s"),
                  logDir.getString(),
                  Configurator::getInstance()->getServerConfig()->getLogLevel(),
                  m_shMemName.getString());

      m_process->setArguments(args.getString());
      start();

      // Prepare other side pipe handles for other side
      otherSidePipeChanTo->assignHandlesFor(m_process->getProcessHandle(),
                                            false);
      otherSidePipeChanFrom->assignHandlesFor(m_process->getProcessHandle(),
                                              false);

      // Transfer other side handles by the memory channel
      mem[1] = (UINT64)otherSidePipeChanTo->getWriteHandle();
      mem[2] = (UINT64)otherSidePipeChanTo->getReadHandle();
      mem[3] = (UINT64)otherSidePipeChanFrom->getWriteHandle();
      mem[4] = (UINT64)otherSidePipeChanFrom->getReadHandle();
      // Sets memory ready flag to true.
      mem[0] = 1;

      // Destroying other side objects
      delete otherSidePipeChanTo;
      Log::debug(_T("DesktopServerWatcher::execute(): Destroyed otherSidePipeChanTo"));
      otherSidePipeChanTo = 0;
      delete otherSidePipeChanFrom;
      Log::debug(_T("DesktopServerWatcher::execute(): Destroyed otherSidePipeChanFrom"));
      otherSidePipeChanFrom = 0;

      m_recListener->onReconnect(ownSidePipeChanTo, ownSidePipeChanFrom);

      m_process->waitForExit();

    } catch (Exception &e) {
      if (ownSidePipeChanTo) delete ownSidePipeChanTo;
      if (otherSidePipeChanTo) delete otherSidePipeChanTo;
      if (ownSidePipeChanFrom) delete ownSidePipeChanFrom;
      if (otherSidePipeChanFrom) delete otherSidePipeChanFrom;
      Log::error(_T("DesktopServerWatcher has failed with error: %s"), e.getMessage());
      Sleep(1000);
    }
  }
}

void DesktopServerWatcher::onTerminate()
{
  m_process->stopWait();
}

void DesktopServerWatcher::start()
{
  int pipeNotConnectedErrorCount = 0;

  for (int i = 0; i < 5; i++) {
    try {
      m_process->start();
      return;
    } catch (SystemException &sysEx) {
      // It can be XP specific error.
      if (sysEx.getErrorCode() == 233) {
        pipeNotConnectedErrorCount++;

        DWORD sessionId = WTS::getActiveConsoleSessionId();

        bool isXPFamily = Environment::isWinXP() || Environment::isWin2003Server();
        bool needXPTrick = (isXPFamily) && (sessionId > 0) && (pipeNotConnectedErrorCount >= 3);

        // Try start as current user with xp trick.
        if (needXPTrick) {
          doXPTrick();
          m_process->start();
          return;
        }
      } else {
        throw;
      }
    }
    Sleep(3000);
  } // for 
}

void DesktopServerWatcher::doXPTrick()
{
  Log::info(_T("Trying to do WindowsXP trick to start process on separate session"));

  try {
    WinStaLibrary winSta;

    WCHAR password[1];
    memset(password, 0, sizeof(password));

    if (winSta.WinStationConnectW(NULL, 0, WTS::getActiveConsoleSessionId(),
      password, 0) == FALSE) {
      throw SystemException(_T("Failed to call WinStationConnectW"));
    }

    // Get path to tvnserver binary.
    StringStorage pathToBinary;
    Environment::getCurrentModulePath(&pathToBinary);

     // Start current console process that will lock workstation (not using Xp Trick).
    CurrentConsoleProcess lockWorkstation(pathToBinary.getString(),
      _T("-lockworkstation"));
    lockWorkstation.start();
    lockWorkstation.waitForExit();

     // Check exit code (exit code is GetLastError() value in case of system error,
     // LockWorkstation() in child process failed, or 0 if workstation is locked).
    DWORD exitCode = lockWorkstation.getExitCode();

    if (exitCode != 0) {
      throw SystemException(exitCode);
    }
  } catch (SystemException &ex) {
    Log::error(ex.getMessage());
    throw;
  }
}
