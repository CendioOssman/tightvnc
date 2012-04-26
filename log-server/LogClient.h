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

#ifndef __LOGCLIENT_H__
#define __LOGCLIENT_H__

#include "util/CommonHeader.h"
#include "io-lib/Channel.h"
#include "io-lib/DataInputStream.h"
#include "io-lib/DataOutputStream.h"
#include "Log.h"
#include "thread/Thread.h"

class LogClient : private Thread
{
public:
  // @param logFileName - is a file name for log without extension.
  LogClient(const TCHAR *publicPipeName, const TCHAR *logFileName);
  virtual ~LogClient();

  // The connect() function try connect to a log server. The function
  // was separated from the constructor to connect the server after
  // it's ready (If objects (server and client) will be automated then
  // no guarantees that configuration for service will be valid at them
  // constructors).
  // @throw Exception on an error.
  void connect();

  // Sends log line to the log server.
  virtual void flushLine(int logLevel, const TCHAR *line);

private:
  void freeResources();

  virtual void execute();

  int getLogBarrier();
  void setLogBarrier(int newLogBar);

  Channel *m_logSendingChan;
  DataInputStream *m_logInput;
  DataOutputStream *m_logOutput;
  LocalMutex m_logWritingMut;

  Channel *m_levListenChan;

  int m_logBarrier;
  LocalMutex m_logBarMut;

  StringStorage m_logFileName;
  StringStorage m_publicPipeName;
};

#endif // __LOGCLIENT_H__
