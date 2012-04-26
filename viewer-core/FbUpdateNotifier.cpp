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

#include "FbupdateNotifier.h"

#include "log-server/Log.h"
#include "thread/AutoLock.h"

FbUpdateNotifier::FbUpdateNotifier(FrameBuffer *fb, LocalMutex *fbLock)
: m_frameBuffer(fb),
  m_fbLock(fbLock),
  m_cursorPainter(fb, fbLock),
  m_isNewSize(false),
  m_isCursorChange(false)
{
  m_oldPosition = m_cursorPainter.hideCursor();
}

FbUpdateNotifier::~FbUpdateNotifier()
{
  try {
    terminate();
    wait();
  } catch (...) {
  }
}

void FbUpdateNotifier::setAdapter(CoreEventsAdapter *adapter)
{
  m_adapter = adapter;
  resume();
}

void FbUpdateNotifier::execute()
{
  while (!isTerminating()) {
    bool noUpdates = true;

    bool isNewSize;
    bool isCursorChange;
    Region update;
    {
      AutoLock al(&m_updateLock);
      isNewSize = m_isNewSize;
      m_isNewSize = false;

      isCursorChange = m_isCursorChange;
      m_isCursorChange = false;

      update = m_update;
      m_update.clear();
    }

    if (isNewSize) {
      noUpdates = false;
      Log::detail(_T("FbUpdateNotifier (event): new size of frame buffer"));
      try {
        AutoLock al(m_fbLock);
        m_adapter->onFrameBufferPropChange(m_frameBuffer);
        // FIXME: it's bad code. Must work without one next line, but not it.
        m_adapter->onFrameBufferUpdate(m_frameBuffer, &m_frameBuffer->getDimension().getRect());
      } catch (...) {
        Log::info(_T("FbUpdateNotifier (event): error in set new size"));
      }
    }

    if (!update.isEmpty() || isCursorChange) {
      noUpdates = false;

      AutoLock al(m_fbLock);
      Rect cursor = m_cursorPainter.showCursor();
      update.addRect(&cursor);
      update.addRect(&m_oldPosition);
      
      vector<Rect> updateList;
      update.getRectVector(&updateList);
      Log::detail(_T("FbUpdateNotifier (event): %u updates"), updateList.size());

      try {
        for (vector<Rect>::iterator i = updateList.begin(); i != updateList.end(); i++)
          m_adapter->onFrameBufferUpdate(m_frameBuffer, &*i);
      } catch (...) {
        Log::info(_T("FbUpdateNotifier (event): error in update"));
      }
      m_oldPosition = m_cursorPainter.hideCursor();
    }
    if (noUpdates)
      m_eventUpdate.waitForEvent();
  }
}

void FbUpdateNotifier::onTerminate()
{
  m_eventUpdate.notify();
}

void FbUpdateNotifier::onUpdate(Rect *update)
{
  {
    AutoLock al(&m_updateLock);
    m_update.addRect(update);
  }
  m_eventUpdate.notify();
  Log::detail(_T("FbUpdateNotifier: added rectangle"));
}

void FbUpdateNotifier::onPropertiesFb()
{
  {
    AutoLock al(&m_updateLock);
    m_update.clear();
    m_isNewSize = true;
  }
  m_eventUpdate.notify();
  Log::detail(_T("FbUpdateNotifier: new size of frame buffer"));
}

void FbUpdateNotifier::updatePointerPos(const Point *position)
{
  {
    AutoLock al(m_fbLock);
    m_cursorPainter.updatePointerPos(position);
  }
  AutoLock al(&m_updateLock);
  m_isCursorChange = true;
  m_eventUpdate.notify();
}

void FbUpdateNotifier::setNewCursor(const Point *hotSpot,
                                    UINT16 width, UINT16 height,
                                    const vector<UINT8> *cursor, 
                                    const vector<UINT8> *bitmask)
{
  {
    AutoLock al(m_fbLock);
    m_cursorPainter.setNewCursor(hotSpot, width, height, cursor, bitmask);
  }
  AutoLock al(&m_updateLock);
  m_isCursorChange = true;
  m_eventUpdate.notify();
}

void FbUpdateNotifier::setIgnoreShapeUpdates(bool ignore)
{
  {
    AutoLock al(m_fbLock);
    m_cursorPainter.setIgnoreShapeUpdates(ignore);
  }
  AutoLock al(&m_updateLock);
  m_isCursorChange = true;
  m_eventUpdate.notify();
}
