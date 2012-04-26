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

#include "CursorPainter.h"

#include "log-server/Log.h"
#include "thread/AutoLock.h"

CursorPainter::CursorPainter(FrameBuffer *fb, LocalMutex *fbLock)
: m_fb(fb),
  m_fbLock(fbLock),
  m_cursorIsMoveable(false),
  m_ignoreShapeUpdates(false),
  m_isExist(false)
{
}

CursorPainter::~CursorPainter()
{
}

void CursorPainter::updatePointerPos(const Point *position)
{
  AutoLock al(&m_lock);
  m_pointerPos = *position;
  m_cursorIsMoveable = true;
}

void CursorPainter::setNewCursor(const Point *hotSpot,
                                 UINT16 width, UINT16 height,
                                 const vector<UINT8> *cursor,
                                 const vector<UINT8> *bitmask)
{
  AutoLock al(&m_lock);
  Log::debug(_T("Cursor hot-spot is (%d, %d)"), hotSpot->x, hotSpot->y);
  m_cursor.setHotSpot(hotSpot->x, hotSpot->y);

  Log::debug(_T("Cursor size is (%d, %d)"), width, height);
  Dimension cursorDimension(width, height);
  PixelFormat pixelFormat = m_fb->getPixelFormat();

  m_cursor.setProperties(&cursorDimension, &pixelFormat);
  m_cursorOverlay.setProperties(&cursorDimension, &pixelFormat);

  size_t pixelSize = m_fb->getBytesPerPixel();
  size_t cursorSize = width * height * pixelSize;
  if (cursorSize != 0) {
    Log::debug(_T("Set image of cursor..."));
    memcpy(m_cursor.getPixels()->getBuffer(), &cursor->front(), cursorSize);
    Log::debug(_T("Set bitmask of cursor..."));
    m_cursor.assignMaskFromRfb(reinterpret_cast<const char *>(&bitmask->front()));
  }
}

void CursorPainter::setIgnoreShapeUpdates(bool ignore)
{
  Log::debug(_T("Set flag of ignor by cursor update is '%d'"), ignore);

  AutoLock al(&m_lock);
  if (ignore == m_ignoreShapeUpdates)
    return;
  m_ignoreShapeUpdates = ignore;
}

Rect CursorPainter::hideCursor()
{
  AutoLock al(&m_lock);
  return eraseCursor();
}

Rect CursorPainter::showCursor()
{
  AutoLock al(&m_lock);
  return paintCursor();
}

Point CursorPainter::getUpperLeftPoint() const
{
  Point position = m_pointerPos;
  position.move(-m_cursor.getHotSpot().x, -m_cursor.getHotSpot().y);
  return position;
}

Rect CursorPainter::getCursorRect() const
{
  if (!m_isExist)
    return Rect();
  Rect rect(m_cursorOverlay.getDimension().getRect());
  Point corner = getUpperLeftPoint();
  rect.move(corner.x, corner.y);

  if (corner.x < 0)
    rect.move(-corner.x, 0);
  if (corner.y < 0)
    rect.move(0, -corner.y);

  Log::info(_T("Cursor rect: (%d, %d), (%d, %d)"), rect.left, rect.top, rect.right, rect.bottom);
  return rect;
}

Rect CursorPainter::eraseCursor()
{
  if (!m_isExist)
    return Rect();
  Rect erase = getCursorRect();

  m_isExist = false;
  if (erase.area() == 0) {
    return Rect();
  }

  Log::debug(_T("Erasing cursor..."));
  m_fb->copyFrom(&erase, &m_cursorOverlay, 0, 0);

  return erase;
}

Rect CursorPainter::paintCursor()
{
  if (m_isExist) {
    Log::error(_T("Error in CursorPainter: painting double copy of cursor."));
    _ASSERT(true);
  }

  if (!m_ignoreShapeUpdates && m_cursorIsMoveable && m_cursor.getDimension().area() != 0) {

    Log::debug(_T("Painting cursor..."));

    Point corner = getUpperLeftPoint();
    int x = corner.x < 0 ? 0 : corner.x;
    int y = corner.y < 0 ? 0 : corner.y;

    m_cursorOverlay.copyFrom(m_fb, x, y);

    Rect overlayRect(&m_cursor.getDimension().getRect());
    overlayRect.move(corner.x, corner.y);
    int srcX = overlayRect.left < 0 ? -overlayRect.left : 0;
    int srcY = overlayRect.top < 0 ? -overlayRect.top : 0;
    overlayRect.move(srcX, srcY);

    m_fb->overlay(&overlayRect, m_cursor.getPixels(), srcX, srcY, m_cursor.getMask());

    m_isExist = true;
    return overlayRect;
  }
  return Rect();
}
