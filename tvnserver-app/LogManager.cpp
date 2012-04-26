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

#include "LogManager.h"
#include "server-config-lib/Configurator.h"
#include "file-lib/File.h"
#include "tvnserver-app/NamingDefs.h"

LogManager::LogManager(const TCHAR *logFileName)
: m_logClient(0),
  m_fileLog(0),
  m_logFileName(logFileName),
  m_logServer(LogNames::LOG_PIPE_PUBLIC_NAME)
{
}

LogManager::~LogManager()
{
  Configurator::getInstance()->removeListener(this);

  if (m_logClient != 0) delete m_logClient;
  if (m_fileLog != 0) delete m_fileLog;
}

void LogManager::onConfigReload(ServerConfig *serverConfig)
{
  updateLogDirPath();
  int logLevel = serverConfig->getLogLevel();
  StringStorage newLogDir;
  serverConfig->getLogFileDir(&newLogDir);
  AutoLock al(&m_logObjectsCreationMutex);
  if (m_logClient != 0) { // Uses log-server scheme
    m_logServer.changeLogProps(newLogDir.getString(), logLevel);
  } else if (m_fileLog != 0) {
    m_fileLog->changeLogProps(newLogDir.getString(), logLevel);
  }
}

void LogManager::init()
{
  updateLogDirPath();

  ServerConfig *srvConf = Configurator::getInstance()->getServerConfig();
  StringStorage logDir;
  srvConf->getLogFileDir(&logDir);
  unsigned char logLevel = srvConf->getLogLevel();

  try {
    if (Configurator::getInstance()->getServiceFlag()) {
      // If run under service then use the log server scheme
      ServerConfig *srvConf = Configurator::getInstance()->getServerConfig();

      m_logServer.start(logDir.getString(), logLevel);
      AutoLock al(&m_logObjectsCreationMutex);
      m_logClient = new LogClient(LogNames::LOG_PIPE_PUBLIC_NAME,
                                  m_logFileName.getString());
      m_logClient->connect();
    } else {

      AutoLock al(&m_logObjectsCreationMutex);
      m_fileLog = new FileLog(logDir.getString(), m_logFileName.getString(),
                              logLevel);
    }
    Configurator::getInstance()->addListener(this);
  } catch (...) {
    enableLog(false);
    throw;
  }
  enableLog(true);
}

void LogManager::updateLogDirPath()
{
  // Creating log directory if it is still no exists.
  StringStorage logFileDir;
  Configurator::getInstance()->getServerConfig()->getLogFileDir(&logFileDir);
  File logDirectory(logFileDir.getString());
  logDirectory.mkdir();
}

void LogManager::flushLine(int logLevel, const TCHAR *line)
{
  // Marshalling the log line to a log backend.
  if (m_logClient != 0) {
    m_logClient->flushLine(logLevel, line);
  } else if (m_fileLog != 0) {
    m_fileLog->flushLine(logLevel, line);
  } else {
    _ASSERT(false);
  }
}
