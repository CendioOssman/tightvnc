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

#ifndef __CUSTOMLOG_H__
#define __CUSTOMLOG_H__

#include "util/CommonHeader.h"
#include "util/Singleton.h"
#include <list>

class CustomLog : protected Singleton<CustomLog>
{
public:
  CustomLog();
  virtual ~CustomLog();

  static void vprint(int logLevel, const TCHAR *fmt, va_list argList);

  // This function defines what it must do with message - store to internal
  // buffer or flush it.
  virtual void writeLine(int logLevel, const TCHAR *line) = 0;

  // Returns true if log has not been initialized yet or
  // has been initialized successfully.
  virtual bool isLogEnabled() = 0;

  virtual void storeHeaderInstance() = 0;
  virtual void getHeaderLinesInstance(std::vector<StringStorage> *headLines) = 0;
};

#endif // __CUSTOMLOG_H__
