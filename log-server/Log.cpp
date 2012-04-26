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

#include "Log.h"

Log::Log()
{
}

Log::~Log()
{
}

void Log::interror(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(LOG_INTERR, fmt, vl);
  va_end(vl);
}

void Log::error(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(LOG_ERR, fmt, vl);
  va_end(vl);
}

void Log::warning(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(LOG_WARN, fmt, vl);
  va_end(vl);
}

void Log::message(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(LOG_MSG, fmt, vl);
  va_end(vl);
}

void Log::info(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(LOG_INFO, fmt, vl);
  va_end(vl);
}

void Log::detail(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(LOG_DETAIL, fmt, vl);
  va_end(vl);
}

void Log::debug(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(LOG_DEBUG, fmt, vl);
  va_end(vl);
}

void Log::storeHeader()
{
  CustomLog *instance = getInstance();
  if (instance == 0) {
    // TODO: Place warnings about wrong usage (usage before initialization) here.
    return;
  }
  instance->storeHeaderInstance();
}

void Log::getHeaderLines(std::vector<StringStorage> *headLines)
{
  CustomLog *instance = getInstance();
  if (instance == 0) {
    // TODO: Place warnings about wrong usage (usage before initialization) here.
    return;
  }
  instance->getHeaderLinesInstance(headLines);
}
