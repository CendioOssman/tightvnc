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

#ifndef _VIEWER_COLLECTOR_H_
#define _VIEWER_COLLECTOR_H_

#include "thread/Thread.h"
#include "thread/LocalMutex.h"
#include "win-system/WindowsEvent.h"
#include <list>
#include "ViewerInstance.h"
#include "TvnViewer.h"

using namespace std;

class TvnViewer;

typedef list<ViewerInstance *> InstanceList;

// Collector instances.
// ViewerCollector has it's own instance which deletes in infinity loop not
// active intances.
class ViewerCollector : private Thread
{
public:
  ViewerCollector(TvnViewer *viewer);
  virtual ~ViewerCollector();

  // Adds instance to a self list.
  void addInstance(ViewerInstance *instance);

  // Forces terminates all instances, waits until they dies and than
  // delete them from memory and thread list.
  void destroyAllInstances();

  // Return true, if and only if m_instances is empty()
  bool empty() const;

protected:
  virtual void execute();

  // Deletes all stopped instances from memory and removes them from self list.
  void deleteDeadInstances();

protected:
  InstanceList m_instances;
  mutable LocalMutex m_lockObj;
  WindowsEvent m_timer;

  TvnViewer *m_viewer;
};

#endif
