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

#ifndef _LOGWRITER_H_
#define _LOGWRITER_H_

#include "Logger.h"
#include "util/CharDefs.h"

class LogWriter
{
public:
  LogWriter(Logger *logger);
  virtual ~LogWriter();

  void interror(const TCHAR *fmt, ...);
  void error(const TCHAR *fmt, ...);
  void warning(const TCHAR *fmt, ...);
  void message(const TCHAR *fmt, ...);
  void info(const TCHAR *fmt, ...);
  void detail(const TCHAR *fmt, ...);
  void debug(const TCHAR *fmt, ...);

protected:
  static const int LOG_INTERR = 0;
  static const int LOG_ERR = 1;
  static const int LOG_WARN = 2;
  static const int LOG_MSG = 3;
  static const int LOG_INFO = 4;
  static const int LOG_DETAIL = 5;
  static const int LOG_DEBUG = 9;

private:
  void vprintLog(int logLevel, const TCHAR *fmt, va_list argList);

  Logger *m_logger;
};

#endif // _LOGWRITER_H_
