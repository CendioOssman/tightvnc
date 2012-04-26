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

#ifndef __VIEWER_INSTANCE_H__
#define __VIEWER_INSTANCE_H__

#include "ViewerWindow.h"
#include "thread/Thread.h"
#include "network/socket/SocketIPv4.h"

class ViewerInstance : public Thread
{
public:

  // creates the viewer instance by using host:port from condata
  ViewerInstance(ConnectionData *condata, 
                 ConnectionConfig *conConf);

  // creates the viewer instance if we have the socket
  ViewerInstance(ConnectionData *condata, 
                 ConnectionConfig *conConf, 
                 SocketIPv4 *socket);

  virtual ~ViewerInstance();

  // starts the viewer instance if newThread is true in new thread
  // else in the same
  void start(bool newThread);

  // wait while viewer is not terminated
  void waitViewer();

protected:
  ConnectionData *m_condata;
  ConnectionConfig *m_conConf;

  ViewerWindow *m_viewerWnd;
  SocketIPv4 *m_socket;

private:
  // Inherited from Thread
  void onTerminate();
  void execute();
};

#endif
