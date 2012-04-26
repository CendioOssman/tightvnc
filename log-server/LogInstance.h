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

#ifndef __LOGINSTANCE_H__
#define __LOGINSTANCE_H__

#include "Log.h"

struct DumpMessageContainer
{
  DumpMessageContainer(const TCHAR *mess, int lgLevel);
  StringStorage message;
  int logLevel;
};

typedef std::list<DumpMessageContainer> LogDump;
typedef LogDump::iterator LogDumpIter;

// The LogInstance class provide Instance interface for derived classes.
// This allow immediately create a Log instance at start of an application.
class LogInstance : private Log
{
public:
  LogInstance();
  virtual ~LogInstance();

  // Call this function to create initialize the log instance.
  // This function must always immediately be called after log backend
  // be defined.
  virtual void init() = 0;

protected:

  // At start time the Log class will stores any log messages to
  // an internal buffer until an derived class has been initialized.
  // After initialization a derived class must call this function to
  // flush the buffer and to enable write log directly to a log backend if
  // the success argument is true or ignore log if success == false.
  void enableLog(bool success);

private:
  virtual void writeLine(int logLevel, const TCHAR *line);

  virtual void storeHeaderInstance();
  virtual void getHeaderLinesInstance(std::vector<StringStorage> *headLines);

  // The end implementaion of the LogInstance class must implement
  // this function to provide a methos that will be used to store log
  // messages (e.g saving to a file)
  virtual void flushLine(int logLevel, const TCHAR *line) = 0;

  // Collect log messages to the m_logDump.
  void collect(int logLevel, const TCHAR *line);

  // Returns true if log has not been initialized or initialized successfully.
  virtual bool isLogEnabled();

  static const size_t MAX_LOG_DUMP_ENTRY = 1000;

  // Sets to false if the enableLog() function has been called with
  // success = false.
  bool m_logEnabled;

  LogDump m_logDump;
  LogDump m_logHeader;
  bool m_storeToFlush;
  LocalMutex m_logDumpMutex;
};

#endif // __LOGINSTANCE_H__
