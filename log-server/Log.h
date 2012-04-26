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

#ifndef _BASE_LOG_H_
#define _BASE_LOG_H_

#include "CustomLog.h"
#include <vector>

class Log : protected CustomLog
{
public:
  Log();
  virtual ~Log();

  static void interror(const TCHAR *fmt, ...);
  static void error(const TCHAR *fmt, ...);
  static void warning(const TCHAR *fmt, ...);
  static void message(const TCHAR *fmt, ...);
  static void info(const TCHAR *fmt, ...);
  static void detail(const TCHAR *fmt, ...);
  static void debug(const TCHAR *fmt, ...);

  // Stores log lines that was before to a permanent storage.
  // This function calling is actual only before a log instantiation.
  // This will allow repeat the lines on an each log storage
  // instantiation. Also, after calling this function an internal log
  // collection will be cleared.
  static void storeHeader();

  // Returns stored header lines.
  static void getHeaderLines(std::vector<StringStorage> *headLines);

protected:
  static const int LOG_INTERR = 0;
  static const int LOG_ERR = 1;
  static const int LOG_WARN = 2;
  static const int LOG_MSG = 3;
  static const int LOG_INFO = 4;
  static const int LOG_DETAIL = 5;
  static const int LOG_DEBUG = 9;
};

#endif
