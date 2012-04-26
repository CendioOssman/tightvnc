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

#include "ViewerInstance.h"

#include "viewer-core/RemoteViewerCore.h"
#include "viewer-core/FileTransferCapability.h"

ViewerInstance::ViewerInstance(ConnectionData *condata, 
                               ConnectionConfig *conConf) 
: m_conConf(conConf),
  m_condata(condata),
  m_socket(0),
  m_viewerWnd(0)
{
}

ViewerInstance::ViewerInstance(ConnectionData *condata, 
                               ConnectionConfig *conConf, 
                               SocketIPv4 *socket) 
: m_conConf(conConf),
  m_condata(condata),
  m_socket(socket),
  m_viewerWnd(0)
{
}

ViewerInstance::~ViewerInstance()
{
  terminate();
  if (isActive()) {
    wait();
  }
  if (m_viewerWnd != 0)
    delete m_viewerWnd;
}

void ViewerInstance::start(bool newThread)
{
  resume();
  if (!newThread) {
    wait();
  }
}

void ViewerInstance::waitViewer()
{
  wait();
}

void ViewerInstance::onTerminate()
{
  m_viewerWnd->postMessage(ViewerWindow::WM_USER_STOP, 0, 0);
}

void ViewerInstance::execute()
{
  m_viewerWnd = new ViewerWindow(m_condata, m_conConf);
  RemoteViewerCore viewerCore;
  m_viewerWnd->setRemoteViewerCore(&viewerCore);

  FileTransferCapability fileTransfer;
  m_viewerWnd->setFileTransfer(&fileTransfer);
  viewerCore.addExtension(&fileTransfer);

  if (m_socket) {
    viewerCore.start(m_socket,
                     m_viewerWnd, m_conConf->getSharedFlag());
  } else {
    StringStorage strHost;
    m_condata->getReducedHost(&strHost);
    UINT16 portVal = m_condata->getPort();
    viewerCore.start(strHost.getString(), portVal,
                     m_viewerWnd, m_conConf->getSharedFlag());
  }
  m_viewerWnd->run(true);
  viewerCore.stop();
  if (m_socket) {
    m_socket->shutdown(SD_BOTH);
    m_socket->close();
  }
  viewerCore.waitTermination();
}
