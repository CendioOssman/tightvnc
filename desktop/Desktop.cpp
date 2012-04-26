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

#include "Desktop.h"
#include "WinDesktop.h"
#include "DesktopClient.h"

Desktop::Desktop(ClipboardListener *extClipListener,
                 UpdateSendingListener *extUpdSendingListener,
                 AbnormDeskTermListener *extDeskTermListener)
: m_guiDesktop(0)
{
  if (Configurator::getInstance()->getServiceFlag()) {
    m_guiDesktop = new DesktopClient(extClipListener,
                                     extUpdSendingListener,
                                     extDeskTermListener);
  } else {
    m_guiDesktop = new WinDesktop(extClipListener,
                                  extUpdSendingListener,
                                  extDeskTermListener);
  }
}

Desktop::~Desktop()
{
  if (m_guiDesktop != 0) {
    delete m_guiDesktop;
  }
}

void Desktop::onUpdateRequest(const Rect *rectRequested,
                              bool incremental)
{
  m_guiDesktop->onUpdateRequest(rectRequested, incremental);
}

void Desktop::getCurrentUserInfo(StringStorage *desktopName,
                                 StringStorage *userName)
{
  m_guiDesktop->getCurrentUserInfo(desktopName, userName);
}

void Desktop::getFrameBufferProperties(Dimension *dim, PixelFormat *pf)
{
  m_guiDesktop->getFrameBufferProperties(dim, pf);
}

void Desktop::getPrimaryDesktopCoords(Rect *rect)
{
  m_guiDesktop->getPrimaryDesktopCoords(rect);
}

void Desktop::getDisplayNumberCoords(Rect *rect,
                                     unsigned char dispNumber)
{
  m_guiDesktop->getDisplayNumberCoords(rect, dispNumber);
}

void Desktop::getWindowCoords(HWND hwnd, Rect *rect)
{
  m_guiDesktop->getWindowCoords(hwnd, rect);
}

HWND Desktop::getWindowHandleByName(const StringStorage *windowName)
{
  return m_guiDesktop->getWindowHandleByName(windowName);
}

void Desktop::setKeyboardEvent(UINT32 keySym, bool down)
{
  m_guiDesktop->setKeyboardEvent(keySym, down);
}

void Desktop::setMouseEvent(UINT16 x, UINT16 y, UINT8 buttonMask)
{
  m_guiDesktop->setMouseEvent(x, y, buttonMask);
}

void Desktop::setNewClipText(const StringStorage *newClipboard)
{
  m_guiDesktop->setNewClipText(newClipboard);
}
