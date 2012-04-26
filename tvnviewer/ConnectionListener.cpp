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

#include "ConnectionListener.h"

const TCHAR ConnectionListener::DEFAULT_HOST[] = _T("0.0.0.0");

ConnectionListener::ConnectionListener(ConnectionData *conData,
                                       ConnectionConfig *conConf,
                                       UINT16 port)
: m_conData(conData),
  m_conConf(conConf),
  TcpServer(DEFAULT_HOST, port, true)
{
}

ConnectionListener::~ConnectionListener() 
{
  for (size_t i = 0; i < m_viewerInstances.size(); i++) {
    delete m_viewerInstances[i];
  }
}

void ConnectionListener::onAcceptConnection(SocketIPv4 *socket)
{
  ViewerInstance *viewerInst = new ViewerInstance(m_conData,
                                                  m_conConf,
                                                  socket);
  m_viewerInstances.push_back(viewerInst);
  viewerInst->start(true);
}
