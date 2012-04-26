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

#include "LogClientInstance.h"

LogClientInstance::LogClientInstance(const TCHAR *publicPipeName,
                                     const TCHAR *logFileName)
: m_logClient(publicPipeName, logFileName)
{
}

LogClientInstance::~LogClientInstance()
{
}

void LogClientInstance::init()
{
  try {
    m_logClient.connect();
  } catch (...) {
    enableLog(false);
    throw;
  }

  enableLog(true);
}

void LogClientInstance::flushLine(int logLevel, const TCHAR *line)
{
  m_logClient.flushLine(logLevel, line);
}
