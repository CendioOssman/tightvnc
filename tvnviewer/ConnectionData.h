// Copyright (C) 2012 GlavSoft LLC.
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

#ifndef _CONNECTION_DATA_
#define _CONNECTION_DATA_

#include "util/StringStorage.h"
#include "rfb/HostPath.h"

class ConnectionData : private HostPath
{
public:
  ConnectionData();

  void setHost(const StringStorage *str);
  bool isUsed() const;
  void setListening();
  bool isListening() const;
  const StringStorage *getDefaultPassword() const;
  void setPlainPassword(const StringStorage *password);
  void setCryptedPassword(const StringStorage *password);
  void getCryptedPassword(StringStorage *hidePassword) const;
  void getHost(StringStorage *strHost) const;
  const StringStorage getHost() const;
  void getReducedHost(StringStorage *strHost) const;
  int getPort() const;
  
protected:
  bool m_Used;
  bool m_isListening;
  StringStorage m_defaultPassword;
};


#endif
