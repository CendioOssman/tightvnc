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

#include "CustomLog.h"

CustomLog::CustomLog()
{
}

CustomLog::~CustomLog()
{
}

#pragma warning(push)
#pragma warning(disable:4996)

void CustomLog::vprint(int logLevel, const TCHAR *fmt, va_list argList)
{
  CustomLog *instance = getInstance();
  if (instance == 0) {
    // TODO: Place warnings about wrong usage (usage before initialization) here.
    return;
  }

  if (!instance->isLogEnabled()) {
    return;
  }

  // Format the original string.
  int count = _vsctprintf(fmt, argList);
  std::vector<TCHAR> formattedString(count + 1);
  _vstprintf(&formattedString.front(), fmt, argList);
  StringStorage message(&formattedString.front());

  instance->writeLine(logLevel, message.getString());
}

#pragma warning(pop)
