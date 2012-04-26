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

#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#include "desktopinterface.h"
#include "ClipboardListener.h"

// External listeners
#include "UpdateSendingListener.h"
#include "AbnormDeskTermListener.h"

// This class is a DesktopInterface implementation that is independed from
// a gui implementation.
class Desktop : public DesktopInterface
{
public:
  Desktop(ClipboardListener *extClipListener,
          UpdateSendingListener *extUpdSendingListener,
          AbnormDeskTermListener *extDeskTermListener);
  virtual ~Desktop();

  // Puts a current desktop name from working session to the
  // desktopName argument and an user name to userMame.
  void getCurrentUserInfo(StringStorage *desktopName,
                          StringStorage *userName);
  // Puts the current frame buffer dimension and pixel format to
  // the dim and pf function arguments.
  void getFrameBufferProperties(Dimension *dim, PixelFormat *pf);

  void getPrimaryDesktopCoords(Rect *rect);
  void getDisplayNumberCoords(Rect *rect,
                              unsigned char dispNumber);

  void getWindowCoords(HWND hwnd, Rect *rect);
  virtual HWND getWindowHandleByName(const StringStorage *windowName);

  void setKeyboardEvent(UINT32 keySym, bool down);
  void setMouseEvent(UINT16 x, UINT16 y, UINT8 buttonMask);
  void setNewClipText(const StringStorage *newClipboard);
private:
  virtual void onUpdateRequest(const Rect *rectRequested,
                               bool incremental);

  // Pointer to a DesktopInterface implementation that is depended from a
  // gui implementation.
  DesktopInterface *m_guiDesktop;
};

#endif // __DESKTOP_H__
