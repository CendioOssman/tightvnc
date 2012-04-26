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

#include "LogInstance.h"

DumpMessageContainer::DumpMessageContainer(const TCHAR *mess, int lgLevel)
: message(mess),
  logLevel(lgLevel)
{

}

LogInstance::LogInstance()
: m_storeToFlush(true),
  m_logEnabled(true)
{
}

LogInstance::~LogInstance()
{
}

bool LogInstance::isLogEnabled()
{
  return m_logEnabled;
}

void LogInstance::writeLine(int logLevel, const TCHAR *line)
{
  if (m_storeToFlush) {
    collect(logLevel, line);
  } else {
    flushLine(logLevel, line);
  }
}

void LogInstance::storeHeaderInstance()
{
  AutoLock al(&m_logDumpMutex);
  m_logHeader = m_logDump;
  m_logDump.clear();
}

void LogInstance::getHeaderLinesInstance(std::vector<StringStorage> *headLines)
{
  headLines->clear();

  AutoLock al(&m_logDumpMutex);
  for (LogDumpIter iter = m_logHeader.begin(); iter != m_logHeader.end(); iter++) {
    headLines->push_back((*iter).message);
  }
}

void LogInstance::collect(int logLevel, const TCHAR *line)
{
  AutoLock al(&m_logDumpMutex);
  DumpMessageContainer cont(line, logLevel);
  m_logDump.push_back(cont);

  // Check for overflow
  if (m_logDump.size() > MAX_LOG_DUMP_ENTRY) {
    // Remove first element
    LogDumpIter firstElement = m_logDump.begin();
    m_logDump.erase(firstElement);
  }
}

void LogInstance::enableLog(bool success)
{
  m_logEnabled = success;

  AutoLock al(&m_logDumpMutex);
  m_storeToFlush = false;

  // Flushing
  for (LogDumpIter iter = m_logDump.begin(); iter != m_logDump.end(); iter++) {
    int loglevel = (*iter).logLevel;
    StringStorage *message = &(*iter).message;
    if (success) {
      flushLine(loglevel, message->getString());
    }
  }
  m_logDump.clear();
}
