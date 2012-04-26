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

#ifndef __LOGMANAGER_H__
#define __LOGMANAGER_H__

#include "log-server/LogInstance.h"
#include "log-server/LogServer.h"
#include "log-server/LogClient.h"
#include "log-server/FileLog.h"
#include "server-config-lib/ConfigReloadListener.h"

// This class automaticly defines what should be run: the log server
// or simple FileLog. This class designed for the TvnServer application.
class LogManager : private ConfigReloadListener, public LogInstance
{
public:
  // @throw Exception on an error.
  LogManager(const TCHAR *logFileName);
  virtual ~LogManager();

  virtual void init();

private:
  virtual void onConfigReload(ServerConfig *serverConfig);

  // Inherited from the LogInstance
  virtual void flushLine(int logLevel, const TCHAR *line);

  void updateLogDirPath();

  StringStorage m_logFileName;

  LogServer m_logServer;
  LogClient *m_logClient;
  FileLog *m_fileLog;
  LocalMutex m_logObjectsCreationMutex;
};

#endif // __LOGMANAGER_H__
