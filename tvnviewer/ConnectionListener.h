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

#ifndef __CONNECTION_LISTENER_H__
#define __CONNECTION_LISTENER_H__

#include "ViewerInstance.h"
#include "network/TcpServer.h"

class ConnectionListener : protected TcpServer
{
public:
  static const TCHAR DEFAULT_HOST[];
  static const UINT16 DEFAULT_PORT = 5500;

  ConnectionListener(ConnectionData *conData,
                     ConnectionConfig *conConf,
                     UINT16 port = DEFAULT_PORT);

  virtual ~ConnectionListener();

protected:
  void onAcceptConnection(SocketIPv4 *socket);

  std::vector<ViewerInstance *> m_viewerInstances;

  ConnectionData *m_conData;
  ConnectionConfig *m_conConf;

};

#endif
