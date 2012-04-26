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

#ifndef __LOGCLIENTINSTANCE_H__
#define __LOGCLIENTINSTANCE_H__

#include "LogInstance.h"
#include "LogClient.h"

// This class provides the log instance for the LogClient backend.
class LogClientInstance : public LogInstance
{
public:
  LogClientInstance(const TCHAR *publicPipeName, const TCHAR *logFileName);
  virtual ~LogClientInstance();

  // Inherited from the LogInstance class
  virtual void init();

private:
  // Inherited from the LogInstance class
  virtual void flushLine(int logLevel, const TCHAR *line);

  LogClient m_logClient;
};

#endif // __LOGCLIENTINSTANCE_H__
