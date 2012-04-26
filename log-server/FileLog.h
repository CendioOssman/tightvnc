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

#ifndef __FILELOG_H__
#define __FILELOG_H__

#include "FileAccount.h"

// This class is alternative to log server scheme. Use this class to create log
// file directly.
class FileLog
{
public:
  // @param logDir - a valid path to directory for log file.
  // @param fileName - a valid log file name without extension.
  // @param logLevel - is an initialization value for log verbosity level.
  FileLog(const TCHAR *logDir, const TCHAR *fileName, unsigned char logLevel);
  virtual ~FileLog();

  // This function allows change a log directory and/or log level after
  // object creation.
  void changeLogProps(const TCHAR *newLogDir, unsigned char newLevel);

  // Stores a log line to the file.
  virtual void flushLine(int logLevel, const TCHAR *line);

private:
  FileAccount m_fileAccount;
};

#endif // __FILELOG_H__
