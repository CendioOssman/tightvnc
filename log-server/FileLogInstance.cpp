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

#include "FileLogInstance.h"

FileLogInstance::FileLogInstance()
: m_fileLog(0)
{
}

FileLogInstance::~FileLogInstance()
{
  if (m_fileLog != 0) {
    delete m_fileLog;
  }
}

void FileLogInstance::init(const TCHAR *logDir,
                           const TCHAR *logFileName,
                           unsigned char logLevel)
{
  try {
    m_fileLog = new FileLog(logDir, logFileName, logLevel);
  } catch (...) {
    m_fileLog = 0;
  }
  init();
}

void FileLogInstance::changeLogProps(const TCHAR *newLogDir,
                                     unsigned char newLevel)
{
  if (m_fileLog != 0) {
    m_fileLog->changeLogProps(newLogDir, newLevel);
  }
}


void FileLogInstance::init()
{
  if (m_fileLog != 0) {
    enableLog(true);
  } else {
    enableLog(false);
  }
}

void FileLogInstance::flushLine(int logLevel, const TCHAR *line)
{
  _ASSERT(m_fileLog != 0);
  m_fileLog->flushLine(logLevel, line);
}
