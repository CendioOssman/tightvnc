// Copyright (C) 2011,2012 GlavSoft LLC.
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

#ifndef _TVN_VIEWER_H_
#define _TVN_VIEWER_H_

#include "ConnectionListener.h"
#include "ControlTrayIcon.h"

#include "thread/ThreadCollector.h"

class TvnViewer : protected ControlTrayIcon
{
public:
  TvnViewer(ConnectionData *condata, ConnectionConfig *conConf);
  virtual ~TvnViewer();

  // sync or async mode of running
  int run(bool isBlocked);

protected:
  void onNewConnection();
  void onDefaultConnection();
  void onConfiguration();
  void onAboutViewer();
  void onCloseViewer();
  void onShowMainWindow();
  void listeningMode();
  void runInstance(bool isBlocked);

  std::auto_ptr<ConnectionListener> m_conListener;
  std::vector<ViewerInstance *> m_viewerInstances;
  std::vector<ConnectionData *> m_conDatas;

  ConnectionData *m_condata;
  ConnectionConfig *m_conConf;
  BaseWindow m_regClass;

  StringStorage m_fullHost;

private:
  int loginDialog();
  bool m_isListening;

  ThreadCollector m_threadCollector;
};

#endif
