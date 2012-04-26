// Copyright (C) 2010,2011,2012 GlavSoft LLC.
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

#include "LogClient.h"
#include "win-system/PipeClient.h"
#include "SecurityPipeClient.h"
#include "util/DateTime.h"

LogClient::LogClient(const TCHAR *publicPipeName, const TCHAR *logFileName)
: m_logSendingChan(0),
  m_levListenChan(0),
  m_logInput(0),
  m_logOutput(0),
  m_logBarrier(0),
  m_logFileName(logFileName),
  m_publicPipeName(publicPipeName)
{
}

LogClient::~LogClient()
{
  freeResources();
}

void LogClient::freeResources()
{
  if (m_levListenChan != 0) m_levListenChan->close();
  terminate();
  resume();
  wait();
  if (m_levListenChan != 0) delete m_levListenChan;

  if (m_logOutput != 0) delete m_logOutput;
  if (m_logInput != 0) delete m_logInput;
  if (m_logSendingChan != 0) delete m_logSendingChan;
}

void LogClient::connect()
{
  NamedPipe *svcChan = 0;
  try {
    // Try connect to log server
    svcChan = PipeClient::connect(m_publicPipeName.getString());
    // Try get security channel from the server.
    SecurityPipeClient secLogPipeClient(svcChan);
    m_logSendingChan = secLogPipeClient.getChannel();
    m_logInput = new DataInputStream(m_logSendingChan);
    m_logOutput = new DataOutputStream(m_logSendingChan);

    SecurityPipeClient secLevelPipeClient(svcChan);
    m_levListenChan = secLevelPipeClient.getChannel();

    m_logOutput->writeUTF8(m_logFileName.getString());

    // Get log level by the m_levListenChan channel.
    DataInputStream m_levInput(m_levListenChan);
    unsigned char logLevel = m_levInput.readUInt8();
    setLogBarrier(logLevel);
  } catch (Exception &e) {
    if (svcChan != 0) delete svcChan;
    freeResources();
    StringStorage formattedException;
    formattedException.format(_T("Can't connect to the log server: %s"),
                              e.getMessage());
    throw Exception(formattedException.getString());
  }
  if (svcChan != 0) delete svcChan;

  resume();
}

void LogClient::flushLine(int logLevel, const TCHAR *line)
{
  if (logLevel <= getLogBarrier()) {
    AutoLock al(&m_logWritingMut);
    if (m_logOutput != 0) {
      try {
        UINT32 processId = GetCurrentProcessId();
        UINT32 threadId = GetCurrentThreadId();
        UINT64 currTime = DateTime::now().getTime();

        m_logOutput->writeUInt32(processId);
        m_logOutput->writeUInt32(threadId);
        m_logOutput->writeUInt64(currTime);
        m_logOutput->writeUInt8(logLevel & 0xf);
        m_logOutput->writeUTF8(line);
      } catch (...) {
      }
    }
  }
}

int LogClient::getLogBarrier()
{
  AutoLock al(&m_logBarMut);
  return m_logBarrier;
}

void LogClient::setLogBarrier(int newLogBar)
{
  AutoLock al(&m_logBarMut);
  m_logBarrier = newLogBar & 0xf;
}

void LogClient::execute()
{
  DataInputStream levInput(m_levListenChan);
  try {
    while (!isTerminating()) {
      setLogBarrier(levInput.readUInt8());
    }
  } catch (...) {
  }
}
