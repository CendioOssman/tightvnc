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

#ifndef _CURSOR_PAINTER_H_
#define _CURSOR_PAINTER_H_

#include "rfb/CursorShape.h"
#include "thread/LocalMutex.h"

class CursorPainter
{
public:
  CursorPainter(FrameBuffer *fb, LocalMutex *fbLock);
  virtual ~CursorPainter();

  // this functions is thread-safe for private data of cursor, but need external lock of frame buffer
  Rect hideCursor();
  Rect showCursor();

  // this functions is thread-safe
  void setIgnoreShapeUpdates(bool ignore);
  void updatePointerPos(const Point *position);
  void setNewCursor(const Point *hotSpot,
                    UINT16 width, UINT16 height,
                    const vector<UINT8> *cursor, 
                    const vector<UINT8> *bitmask);
private:
  Point getUpperLeftPoint() const;
  Rect getCursorRect() const;

  Rect eraseCursor();
  Rect paintCursor();

  FrameBuffer *const m_fb;
  LocalMutex *m_fbLock;

  LocalMutex m_lock;
  CursorShape m_cursor;
  Point m_pointerPos;

  // copy of rect frame buffer under cursor
  FrameBuffer m_cursorOverlay;

  bool m_isExist;
  bool m_cursorIsMoveable;
  bool m_ignoreShapeUpdates;

private:
  // Do not allow copying objects.
  CursorPainter(const CursorPainter &);
  CursorPainter &operator=(const CursorPainter &);
};

#endif
